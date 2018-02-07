//
#include "reverse_ec_scheduler.h"
#include "../actor/coordinator.h"
#include "../actor/base/local_send.h"
#include "../reliable/process_reliable_msg.h"

#include <glog/logging.h>

void reverse_ec_scheduler::ProcessBatch(bess::PacketBatch *batch){
  dp_pkt_batch.clear();
  cp_pkt_batch.clear();
  coordinator_actor_->ec_scheduler_batch_.clear();
  char keys[bess::PacketBatch::kMaxBurst][flow_key_size] __ymm_aligned;

  for(int i=0; i<batch->cnt(); i++){
    char* data_start = batch->pkts()[i]->head_data<char*>();

    if(unlikely( ((*((uint16_t*)(data_start+14)) & 0x00f0) != 0x0040) ||
                 ( ((*((uint16_t*)(data_start+23)) & 0x00ff) != 0x0006) &&
                   ((*((uint16_t*)(data_start+23)) & 0x00ff) != 0x0011) &&
                   ((*((uint16_t*)(data_start+23)) & 0x00ff) != 0x00FF)) ) ){
      coordinator_actor_->gp_collector_.collect(batch->pkts()[i]);
      continue;
    }

    if(((*((uint16_t*)(data_start+23)) & 0x00ff) == 0x00FF)){
      cp_pkt_batch.add(batch->pkts()[i]);

    }
    else{
      dp_pkt_batch.add(batch->pkts()[i]);
    }
  }

  for(int i=0; i<dp_pkt_batch.cnt(); i++){
    char* data_start = dp_pkt_batch.pkts()[i]->head_data<char*>();

    memset(&keys[i][flow_key_size-8], 0, sizeof(uint64_t));
    for(int j=0; j<3; j++){
      char* key = keys[i]+coordinator_actor_->fields_[j].pos;
      *(uint64_t *)key = *(uint64_t *)(data_start + coordinator_actor_->fields_[j].offset) &
                         coordinator_actor_->fields_[j].mask;
    }

    flow_actor** actor_ptr = coordinator_actor_->htable_.Get(reinterpret_cast<flow_key_t*>(keys[i]));
    flow_actor* actor = 0;

    if(unlikely(actor_ptr==nullptr)){
      actor = coordinator_actor_->allocator_.allocate();

      if(unlikely(actor==nullptr)){
        LOG(WARNING)<<"No available flow actors to allocate";
        actor = coordinator_actor_->deadend_flow_actor_;
      }
      else{
        generic_list_item* replica_item = coordinator_actor_->replicas_rrlist_.rotate();

        coordinator_actor_->active_flows_rrlist_.add_to_tail(actor);

        send(actor, flow_actor_init_with_pkt_t::value,
             coordinator_actor_,
             reinterpret_cast<flow_key_t*>(keys[i]),
             coordinator_actor_->service_chain_,
             dp_pkt_batch.pkts()[i],
             replica_item);
      }

      coordinator_actor_->htable_.Set(reinterpret_cast<flow_key_t*>(keys[i]), &actor);

      uint64_t actor_id_64 = actor->get_id_64();
      coordinator_actor_->actorid_htable_.Set(&actor_id_64, &actor);

      actor_ptr = &actor;
    }

    send(*actor_ptr, pkt_msg_t::value, dp_pkt_batch.pkts()[i]);
  }

  for(int i=0; i<cp_pkt_batch.cnt(); i++){
    char* data_start = cp_pkt_batch.pkts()[i]->head_data<char*>();
    uint64_t mac_addr = ((*(reinterpret_cast<uint64_t *>(data_start+6))) & 0x0000FFffFFffFFfflu);

    reliable_p2p** r_ptr = coordinator_actor_->mac_to_reliables_.Get(&mac_addr);
    if(unlikely(r_ptr == nullptr)){
      coordinator_actor_->gp_collector_.collect(cp_pkt_batch.pkts()[i]);
      continue;
    }

    reliable_single_msg* msg_ptr = (*r_ptr)->recv(cp_pkt_batch.pkts()[i]);
    if(unlikely(msg_ptr == nullptr)){
      continue;
    }

    process_reliable_msg::match(msg_ptr, coordinator_actor_);
    msg_ptr->clean(&(coordinator_actor_->gp_collector_));
  }

  RunNextModule(&(coordinator_actor_->ec_scheduler_batch_));
}

void reverse_ec_scheduler::customized_init(coordinator* coordinator_actor){
  coordinator_actor_ = coordinator_actor;
}

ADD_MODULE(reverse_ec_scheduler, "reverse_ec_scheduler",
    "process packets received from output port to input port and schedule actors in reverse direction")

//
#include "recv_reliable_msgack.h"
#include "../actor/coordinator.h"
#include "../actor/base/local_send.h"
#include "../reliable/process_reliable_msg.h"

void recv_reliable_msgack::customized_init(coordinator* coordinator_actor){
  coordinator_actor_ = coordinator_actor;
}

void recv_reliable_msgack::ProcessBatch(bess::PacketBatch *batch){
  coordinator_actor_->gb_.clear();
  for(int i=0; i<batch->cnt(); i++){
    char* data_start = batch->pkts()[i]->head_data<char*>();

    if(unlikely( ((*((uint16_t*)(data_start+14)) & 0x00f0) != 0x0040) ||
                 ((*((uint16_t*)(data_start+23)) & 0x00ff) != 0x00FF))  ){
      coordinator_actor_->gp_collector_.collect(batch->pkts()[i]);
      continue;
    }

    uint64_t mac_addr = ((*(reinterpret_cast<uint64_t *>(data_start+6))) & 0x0000FFffFFffFFfflu);

    reliable_p2p** r_ptr = coordinator_actor_->mac_to_reliables_.Get(&mac_addr);
    if(unlikely(r_ptr == nullptr)){
      coordinator_actor_->gp_collector_.collect(batch->pkts()[i]);
      continue;
    }

    reliable_single_msg* msg_ptr = (*r_ptr)->recv(batch->pkts()[i]);
    if(unlikely(msg_ptr == nullptr)){
      continue;
    }

    process_reliable_msg::match(msg_ptr, coordinator_actor_);
    msg_ptr->clean(&(coordinator_actor_->gp_collector_));
  }

  int last_batch_pos = coordinator_actor_->gb_.get_batch_pos();

  for(int i=0; i<last_batch_pos; i++){
    RunSplit(coordinator_actor_->gb_.get_gate(i), coordinator_actor_->gb_.get_batch(i));
  }

  bess::PacketBatch* last_batch = coordinator_actor_->gb_.get_batch(last_batch_pos);
  if(last_batch->cnt()>0){
    RunSplit(coordinator_actor_->gb_.get_gate(last_batch_pos), last_batch);
  }
}

ADD_MODULE(recv_reliable_msgack, "recv_reliable_msgack",
    "this module handles packets received from the control port")

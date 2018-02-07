#ifndef PROCESS_RELIABLE_MSG_H
#define PROCESS_RELIABLE_MSG_H

#include "../actor/coordinator.h"
#include "../actor/flow_actor.h"
#include "./base/reliable_message_misc.h"

class process_reliable_msg{
public:
  static inline void match(reliable_single_msg* msg_ptr, coordinator* coordinator_actor_){
    if(msg_ptr->rmh.recv_actor_id == coordinator_actor_id){
      switch(static_cast<coordinator_messages>(msg_ptr->rmh.msg_type)){
        case coordinator_messages::ping : {
          send(coordinator_actor_,
               ping_t::value,
               msg_ptr->send_runtime_id,
               msg_ptr->rmh.send_actor_id,
               msg_ptr->rmh.msg_id,
               msg_ptr->cstruct_pkt->head_data<ping_cstruct*>());
          break;
        }
        case coordinator_messages::pong : {
          send(coordinator_actor_,
               pong_t::value,
               msg_ptr->send_runtime_id,
               msg_ptr->rmh.send_actor_id,
               msg_ptr->rmh.msg_id,
               msg_ptr->cstruct_pkt->head_data<pong_cstruct*>());
          break;
        }
        case coordinator_messages::create_migration_target_actor : {
          send(coordinator_actor_,
               create_migration_target_actor_t::value,
               msg_ptr->send_runtime_id,
               msg_ptr->rmh.send_actor_id,
               msg_ptr->rmh.msg_id,
               msg_ptr->cstruct_pkt->head_data<create_migration_target_actor_cstruct*>());
          break;
        }
        case coordinator_messages::change_vswitch_route : {
          send(coordinator_actor_,
               change_vswitch_route_t::value,
               msg_ptr->send_runtime_id,
               msg_ptr->rmh.send_actor_id,
               msg_ptr->rmh.msg_id,
               msg_ptr->cstruct_pkt->head_data<change_vswitch_route_request_cstruct*>());

          break;
        }
        case coordinator_messages::replication_msg : {
          // msg_ptr->send_runtime_id,
          // msg_ptr->fs_msg_batch,
          // msg_ptr->raw_pkt,
          flow_key_t* single_key =
              msg_ptr->fs_msg_batch.pkts()[0]->head_data<flow_key_t*>(2*sizeof(uint32_t)+2*sizeof(struct ether_addr));

          flow_actor** actor_ptr = coordinator_actor_->htable_.Get(single_key);
          flow_actor* actor = 0;

          if(unlikely(actor_ptr==nullptr)){
            actor = coordinator_actor_->allocator_.allocate();

            if(unlikely(actor==nullptr)){
              LOG(WARNING)<<"No available flow actors to allocate";
              actor = coordinator_actor_->deadend_flow_actor_;
            }
            else{

              struct cdlist_head* replica_flow_list =
                  coordinator_actor_->replica_flow_lists_.find(msg_ptr->send_runtime_id);
              cdlist_add_tail(replica_flow_list, actor->get_cdlist_item());

              send(actor, flow_actor_init_with_first_rep_pkt_t::value,
                   coordinator_actor_,
                   (single_key),
                   coordinator_actor_->service_chain_,
                   &(msg_ptr->fs_msg_batch));
            }

            coordinator_actor_->htable_.Set((single_key), &actor);

            uint64_t actor_id_64 = actor->get_id_64();
            coordinator_actor_->actorid_htable_.Set(&actor_id_64, &actor);

            actor_ptr = &actor;
          }

          send(*actor_ptr, rep_fs_pkt_msg_t::value, &(msg_ptr->fs_msg_batch));

          break;
        }
        case coordinator_messages::replica_recover : {
           send(coordinator_actor_,
                replica_recover_t::value,
                msg_ptr->send_runtime_id,
                msg_ptr->rmh.send_actor_id,
                msg_ptr->rmh.msg_id,
                msg_ptr->cstruct_pkt->head_data<replica_recover_cstruct*>());

           break;
         }
        default : {
          assert(1==0);
          break;
        }
      }
    }
    else{
      uint64_t actor_id_64 = 0x00000000FfFfFfFf & msg_ptr->rmh.recv_actor_id;
      flow_actor** actor_ptr = coordinator_actor_->actorid_htable_.Get(&actor_id_64);
      if(unlikely(actor_ptr == 0)){
        LOG(INFO)<<"The actor with id "<<msg_ptr->rmh.recv_actor_id<<" does not exist";
        return;
      }

      switch(static_cast<flow_actor_messages>(msg_ptr->rmh.msg_type)){
        case flow_actor_messages::start_migration_response : {
          send(*actor_ptr,
               start_migration_response_t::value,
               msg_ptr->cstruct_pkt->head_data<start_migration_response_cstruct*>());
          break;
        }
        case flow_actor_messages::change_vswitch_route_response : {
          send(*actor_ptr,
               change_vswitch_route_response_t::value,
               msg_ptr->cstruct_pkt->head_data<change_vswitch_route_response_cstruct*>());
          break;
        }
        case flow_actor_messages::migrate_flow_state : {
          send(*actor_ptr,
               migrate_flow_state_t::value,
               msg_ptr->send_runtime_id,
               msg_ptr->rmh.send_actor_id,
               msg_ptr->rmh.msg_id,
               &(msg_ptr->fs_msg_batch));
          break;
        }
        case flow_actor_messages::migrate_flow_state_response : {
          send(*actor_ptr,
               migrate_flow_state_response_t::value,
               msg_ptr->cstruct_pkt->head_data<migrate_flow_state_response_cstruct*>());
          break;
        }
        case flow_actor_messages::replica_recover_response : {
          send(*actor_ptr,
               replica_recover_response_t::value,
               msg_ptr->cstruct_pkt->head_data<replica_recover_response_cstruct*>());
          break;
        }
        default : {
          assert(1==0);
          break;
        }
      }
    }
  }
};

#endif

#ifndef COORDINATOR_H
#define COORDINATOR_H

#include <list>
#include <unordered_map>

#include "../bessport/utils/htable.h"
#include "../bessport/pktbatch.h"
#include "../bessport/worker.h"
#include "./base/nfa_ipv4_field.h"
#include "./base/flow_hash.h"
#include "coordinator_messages.h"
#include "../nf/base/network_function_base.h"
#include "../rpc/ring_msg.h"
#include "../nfaflags.h"
#include "../rpc/llring_holder.h"
#include "../utils/generic_ring_allocator.h"
#include "coordinator_public_field.h"

class flow_actor;
class flow_actor_allocator;

class coordinator : public core, public garbage, public local_batch, public timer_list,
                    public rpcworker_llring, public local_runtime_info, public rr_lists,
                    public migration_target_source_holder, public reliables_holder, public migration_stats,
                    public giant_batch_holder, public replica_flow_holder, public replication_stats{
public:
  coordinator(llring_holder& holder);

  void handle_message(remove_flow_t, flow_actor* flow_actor, flow_key_t* flow_key);

  void handle_message(ping_t, int32_t sender_rtid, uint32_t sender_actor_id, uint32_t msg_id,
                      ping_cstruct* cstruct_ptr);

  void handle_message(pong_t, int32_t sender_rtid, uint32_t sender_actor_id, uint32_t msg_id,
                      pong_cstruct* cstruct_ptr);

  void handle_message(create_migration_target_actor_t,
                      int32_t sender_rtid,
                      uint32_t sender_actor_id,
                      uint32_t msg_id,
                      create_migration_target_actor_cstruct* cstruct_ptr);

  void handle_message(change_vswitch_route_t,
                      int32_t sender_rtid,
                      uint32_t sender_actor_id,
                      uint32_t msg_id,
                      change_vswitch_route_request_cstruct* cstruct_ptr);

  void handle_message(replica_recover_t,
                      int32_t sender_rtid,
                      uint32_t sender_actor_id,
                      uint32_t msg_id,
                      replica_recover_cstruct* cstruct_ptr);



  inline generic_ring_allocator<generic_list_item>* get_list_item_allocator(){
    return &mac_list_item_allocator_;
  }

  inline uint32_t allocate_msg_id(){
    uint32_t return_id = next_msg_id_;
    next_msg_id_+=1;
    return return_id;
  }

private:
  uint32_t next_msg_id_;

  int counter = 0;
  uint64_t start_time = 0;

  uint64_t parse_service_chain(string service_chain_str);
};

#endif

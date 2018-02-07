#ifndef FLOW_ACTOR_H
#define FLOW_ACTOR_H

#include <iostream>
#include <list>
#include <vector>

#include "../bessport/packet.h"
#include "../bessport/pktbatch.h"
#include "./base/flow_key.h"
#include "./base/flow_ether_header.h"
#include "flow_actor_messages.h"
#include "../nf/base/network_function_base.h"
#include "../nf/base/nf_item.h"
#include "../utils/cdlist.h"
#include "actor_timer.h"
#include "./base/actor_misc.h"
#include "../utils/generic_list_item.h"
#include "../reliable/reliable_p2p.h"

using namespace std;

class coordinator;

class flow_actor{
public:
  using flow_actor_id_t = uint32_t;

  void handle_message(flow_actor_init_with_pkt_t,
                      coordinator* coordinator_actor,
                      flow_key_t* flow_key,
                      vector<network_function_base*>& service_chain,
                      bess::Packet* first_packet,
                      generic_list_item* replica_item);

  void handle_message(flow_actor_init_with_cstruct_t,
                      coordinator* coordinator_actor,
                      flow_key_t* flow_key,
                      vector<network_function_base*>& service_chain,
                      create_migration_target_actor_cstruct* cstruct);

  void handle_message(flow_actor_init_with_first_rep_pkt_t,
                      coordinator* coordinator_actor,
                      flow_key_t* flow_key,
                      vector<network_function_base*>& service_chain,
                      bess::PacketBatch* first_fs_msg_batch);


  void handle_message(check_idle_t);


  void handle_message(pkt_msg_t, bess::Packet* pkt);


  void handle_message(rep_fs_pkt_msg_t, bess::PacketBatch* fs_msg_batch);


  void handle_message(start_migration_t, int32_t migration_target_rtid);

  void handle_message(start_migration_timeout_t);

  void handle_message(start_migration_response_t, start_migration_response_cstruct* cstruct_ptr);


  void handle_message(change_vswitch_route_timeout_t);

  void handle_message(change_vswitch_route_response_t, change_vswitch_route_response_cstruct* cstruct_ptr);


  void handle_message(migrate_flow_state_t,
                      int32_t sender_rtid,
                      uint32_t sender_actor_id,
                      uint32_t request_msg_id,
                      bess::PacketBatch* fs_pkt_batch);

  void handle_message(migrate_flow_state_timeout_t);

  void handle_message(migrate_flow_state_response_t, migrate_flow_state_response_cstruct* cstruct_ptr);


  void start_recover();

  void handle_message(replica_recover_timeout_t);

  void handle_message(replica_recover_response_t, replica_recover_response_cstruct* cstruct_ptr);

  inline flow_actor_id_t get_id(){
    return actor_id_;
  }

  inline uint64_t get_id_64(){
    uint64_t actor_id_64 = 0x00000000FfFfFfFf & actor_id_;
    return actor_id_64;
  }

  inline void set_id(flow_actor_id_t actor_id){
    actor_id_ = actor_id;
  }

  inline actor_timer<actor_timer_type::flow_actor_req_timer>* get_idle_timer(){
    return &idle_timer_;
  }

  inline actor_timer<actor_timer_type::flow_actor_req_timer>* get_migration_timer(){
    return &migration_timer_;
  }

  inline actor_timer<actor_timer_type::flow_actor_req_timer>* get_replication_timer(){
    return &replication_timer_;
  }

  inline void update_output_header(int32_t new_output_rtid,
                                   uint64_t new_output_rt_input_mac){
    output_header_.dest_rtid = new_output_rtid;
    output_header_.ethh.d_addr = *(reinterpret_cast<struct ether_addr*>(&new_output_rt_input_mac));
  }

  inline void set_up_pkt_processing_funcs(){
    funcs_[1] = &flow_actor::pkt_normal_nf_processing;
    funcs_[3] = &flow_actor::pkt_migration_target_processing;

    funcs_[2] = &flow_actor::pkt_normal_nf_processing;
    funcs_[6] = &flow_actor::pkt_process_after_route_change;
    funcs_[4] = &flow_actor::pkt_normal_nf_processing;


    funcs_[0] = &flow_actor::pkt_normal_nf_processing;
    funcs_[5] = &flow_actor::pkt_normal_nf_processing;
  }

  inline void set_up_replication_processing_funcs(){
    replication_funcs_[0] = &flow_actor::no_replication_output;
    replication_funcs_[1] = &flow_actor::replication_output;
    replication_funcs_[2] = &flow_actor::no_replication_output;
  }

  inline void init_buffer_head(){
    cdlist_head_init(&buffer_head_);
  }

  inline void init_cdlist_item(){
    cdlist_item_init(&list_item_);
  }

  inline struct cdlist_item* get_cdlist_item(){
    return &list_item_;
  }

private:
  struct cdlist_item list_item_;

  flow_actor_id_t actor_id_;

  uint64_t pkt_counter_;

  uint64_t sample_counter_;

  flow_key_t flow_key_;

  coordinator* coordinator_actor_;

  flow_ether_header input_header_;

  flow_ether_header output_header_;

  size_t service_chain_length_;

  flow_actor_nfs nfs_;

  flow_actor_fs fs_;

  flow_actor_fs_size fs_size_;

  actor_timer<actor_timer_type::flow_actor_req_timer> idle_timer_;

  actor_timer<actor_timer_type::flow_actor_req_timer> migration_timer_;

  actor_timer<actor_timer_type::flow_actor_req_timer> replication_timer_;

  // for migration
  uint32_t migration_target_actor_id_;

  uint32_t current_state_;

  void failure_handling();

  void pkt_normal_nf_processing(bess::Packet* pkt);

  void pkt_migration_target_processing(bess::Packet* pkt);

  void pkt_process_after_route_change(bess::Packet* pkt);

  typedef  void (flow_actor::*pkt_processing_func)(bess::Packet*);

  pkt_processing_func funcs_[7];

  struct cdlist_head buffer_head_;

  // for replication
  uint32_t replication_state_;

  reliable_p2p* r_;

  void no_replication_output(bess::Packet* pkt, bool result);

  void replication_output(bess::Packet* pkt, bool result);

  typedef void(flow_actor::*replication_processing_func)(bess::Packet*, bool result);

  replication_processing_func replication_funcs_[3];

};

static_assert(std::is_pod<flow_actor>::value, "flow_actor is not pod");

#endif

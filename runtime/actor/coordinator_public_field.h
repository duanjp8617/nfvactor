#ifndef COORDINATOR_PUBLIC_FIELD
#define COORDINATOR_PUBLIC_FIELD

#include <list>
#include <unordered_map>

#include "../utils/round_rubin_list.h"
#include "../utils/fast_hash_map.h"
#include "../bessport/module.h"
#include "../bessport/kmod/llring.h"
#include "../rpc/ring_msg.h"
#include "../actor/flow_actor.h"
#include "../reliable/reliable_p2p.h"
#include "./base/garbage_pkt_collector.h"
#include "../utils/generic_list_item.h"
#include "actor_timer_list.h"
#include "../utils/fixed_array.h"
#include "flow_actor_allocator.h"
#include "./base/giant_batch.h"
#include "../utils/generic_ring_allocator.h"
#include "../utils/buffered_packet.h"

struct core{
  flow_actor_allocator allocator_;

  HTable<flow_key_t, flow_actor*, flow_keycmp, flow_hash> htable_;

  HTable<uint64_t, flow_actor*, actorid_keycmp, actorid_hash> actorid_htable_;

  flow_actor* deadend_flow_actor_;

  nfa_ipv4_field fields_[3];

  std::vector<network_function_base*> service_chain_;

  size_t total_fs_size_;

  generic_ring_allocator<generic_list_item> mac_list_item_allocator_;
};

struct garbage{
  garbage_pkt_collector gp_collector_;
};

struct local_batch{
  bess::PacketBatch ec_scheduler_batch_;
  uint16_t ec_scheduler_gates_[bess::PacketBatch::kMaxBurst];
};

struct timer_list{
  actor_timer_list<actor_timer_type::flow_actor_req_timer> idle_flow_list_;
  actor_timer_list<actor_timer_type::flow_actor_req_timer> req_timer_list_;
};

struct rpcworker_llring{
  struct llring* rpc2worker_ring_;
  struct llring* worker2rpc_ring_;
};

struct local_runtime_info{
  runtime_config local_runtime_;
  uint64_t default_input_mac_;
  uint64_t default_output_mac_;
};

struct rr_lists{

  round_rubin_list<generic_list_item> output_runtime_mac_rrlist_;
  round_rubin_list<generic_list_item> input_runtime_mac_rrlist_;

  round_rubin_list<flow_actor> active_flows_rrlist_;

  round_rubin_list<generic_list_item> replicas_rrlist_;

  round_rubin_list<generic_list_item> reliable_send_list_;
};

struct migration_target_source_holder{
  uint64_t migration_qouta_;
  uint32_t migration_target_rt_id_;
  uint64_t outgoing_migrations_;
  fixed_array<int32_t> migration_targets_;
};

struct reliables_holder{
  fast_hash_map<uint32_t, reliable_p2p, uint32_keycmp, uint32_hash> reliables_;
  HTable<uint64_t, reliable_p2p*, uint64_keycmp, uint64_hash> mac_to_reliables_;
};

struct replica_flow_holder{
  fast_hash_map<uint32_t, struct cdlist_head, uint32_keycmp, uint32_hash> replica_flow_lists_;
};

struct migration_stats{

  uint64_t passive_migration_iteration_;
  uint64_t total_passive_migration_;
  uint64_t successful_passive_migration_;
  uint64_t failed_passive_migration_;
  uint64_t null_passive_migration_;
  uint64_t migration_source_loss_counter_;
  uint64_t current_iteration_start_time_;
  uint64_t current_iteration_end_time_;

  uint64_t migration_target_loss_counter_;
  uint64_t migration_target_buffer_size_counter_;
  uint64_t migrated_in_flow_num_;
};

struct replication_stats{
  uint32_t storage_rtid_;
  uint64_t out_going_recovery_;

  uint64_t recovery_iteration_;
  uint64_t successful_recovery_;
  uint64_t unsuccessful_recovery_;
  uint64_t current_recovery_iteration_start_time_;
  uint64_t current_recovery_iteration_end_time_;
};

struct giant_batch_holder{
  generic_ring_allocator<buffered_packet> collective_buffer_;
  giant_batch gb_;
};

#endif

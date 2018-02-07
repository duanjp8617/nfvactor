#ifndef FLOW_ACTOR_MESSAGES_H
#define FLOW_ACTOR_MESSAGES_H

#include "./base/local_message.h"

enum class flow_actor_messages : uint16_t{
  flow_actor_init_with_pkt,
  flow_actor_init_with_cstruct,
  flow_actor_init_with_first_rep_pkt,

  check_idle,

  pkt_msg,

  rep_fs_pkt_msg,

  start_migration,
  start_migration_timeout,
  start_migration_response,

  change_vswitch_route_timeout,
  change_vswitch_route_response,

  migrate_flow_state,
  migrate_flow_state_timeout,
  migrate_flow_state_response,

  replica_recover_timeout,
  replica_recover_response
};

using flow_actor_init_with_pkt_t = local_message(flow_actor_messages, flow_actor_init_with_pkt);
using flow_actor_init_with_cstruct_t = local_message(flow_actor_messages, flow_actor_init_with_cstruct);
using flow_actor_init_with_first_rep_pkt_t = local_message(flow_actor_messages, flow_actor_init_with_first_rep_pkt);

using check_idle_t = local_message(flow_actor_messages, check_idle);

using pkt_msg_t = local_message(flow_actor_messages, pkt_msg);

using rep_fs_pkt_msg_t = local_message(flow_actor_messages, rep_fs_pkt_msg);

using start_migration_t = local_message(flow_actor_messages, start_migration);
using start_migration_timeout_t = local_message(flow_actor_messages, start_migration_timeout);
using start_migration_response_t = local_message(flow_actor_messages, start_migration_response);
struct start_migration_response_cstruct{
  uint32_t request_msg_id;
  uint32_t migration_target_actor_id;
};

using change_vswitch_route_timeout_t = local_message(flow_actor_messages, change_vswitch_route_timeout);
using change_vswitch_route_response_t = local_message(flow_actor_messages, change_vswitch_route_response);
struct change_vswitch_route_response_cstruct{
  uint32_t request_msg_id;
  uint32_t change_route_succeed;
};


using migrate_flow_state_t = local_message(flow_actor_messages, migrate_flow_state);
using migrate_flow_state_timeout_t = local_message(flow_actor_messages, migrate_flow_state_timeout);
using migrate_flow_state_response_t = local_message(flow_actor_messages, migrate_flow_state_response);
struct migrate_flow_state_response_cstruct{
  uint32_t request_msg_id;
};

using replica_recover_timeout_t = local_message(flow_actor_messages, replica_recover_timeout);
using replica_recover_response_t = local_message(flow_actor_messages, replica_recover_response);
struct replica_recover_response_cstruct{
  uint32_t request_msg_id;
  uint32_t recover_succeed;
};


#endif

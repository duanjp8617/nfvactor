#ifndef ACTOR_MISC_H
#define ACTOR_MISC_H

#include "../../bessport/packet.h"

// size of the flow actor allocator
static constexpr int num_flow_actors = 1024*1024;

// actor ids
static constexpr int32_t invalid_flow_actor_id = 0;

static constexpr int32_t coordinator_actor_id = 1;

static constexpr int32_t flow_actor_id_start = 2;

// message ids
static constexpr uint32_t invalid_message_id = 0;

static constexpr uint32_t idle_message_id = 1;

static constexpr uint32_t message_id_start = 2;

// time out value
static constexpr uint64_t flow_actor_idle_timeout = 1000000000; //3s

static constexpr uint64_t request_timeout = 1000000000; //1s ms

// type of actor timer
// TODO: remove this
enum class actor_timer_type{
  flow_actor_req_timer,
  flow_actor_idle_timer
};

// type of the actors
enum class actor_type : uint16_t{
  flow_actor,
  coordinator_actor
};

// flow actor state
static constexpr uint32_t flow_actor_normal_processing = 0x1; // 0001

static constexpr uint32_t flow_actor_migration_target = 0x3;  // 0011

static constexpr uint32_t flow_actor_migration_source = 0x2;  // 0010

static constexpr uint32_t flow_actor_migration_source_after_route_change = 0x6; // 0110

static constexpr uint32_t flow_actor_migration_failure_processing = 0x4; //0100

// The number of packet batches in giant batch
static constexpr int buffer_batch_size = 50;

// replication state
static constexpr uint32_t no_replica = 0x0;

static constexpr uint32_t have_replica = 0x1;

static constexpr uint32_t is_replica = 0x2;

#endif

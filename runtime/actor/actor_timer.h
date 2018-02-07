#ifndef ACTOR_TIMER_H
#define ACTOR_TIMER_H

#include <cassert>

#include "../utils/round_rubin_list.h"
#include "./base/actor_misc.h"
#include "./base/local_send.h"
#include "coordinator_messages.h"
#include "flow_actor_messages.h"

class coordinator;
class flow_actor;

template<actor_timer_type T>
struct actor_timer{
  cdlist_item list_item_;

  // ptr to the actor, may be a flow_actor pointer
  // or coordinator actor pointer
  void* actor_ptr_;

  // type of the actor_ptr, according t
  uint16_t type_;

  uint64_t to_time_;

  uint32_t request_msg_id_;

  uint16_t msg_type_;

  // called at initialization time.
  inline void init(uint16_t type, void* actor_ptr){
    type_ = type;
    actor_ptr_ = actor_ptr;
    request_msg_id_ = invalid_message_id;
    cdlist_item_init(&list_item_);
  }

  // called when the actor is deallocated,
  // or after the timer has finished triggering,
  // or after the response associated with the request has been received.
  inline void invalidate(){
    request_msg_id_ = invalid_message_id;
    cdlist_del(&list_item_);
  }

  inline bool request_timeout(uint32_t request_msg_id){
    return (request_msg_id_==request_msg_id);
  }

  inline void send_flow_actor_messgae(){
    switch(static_cast<flow_actor_messages>(msg_type_)){
      case flow_actor_messages::check_idle :
        send(static_cast<flow_actor*>(actor_ptr_), check_idle_t::value);
        break;
      case flow_actor_messages::start_migration_timeout :
        send(static_cast<flow_actor*>(actor_ptr_), start_migration_timeout_t::value);
        break;
      case flow_actor_messages::change_vswitch_route_timeout :
        send(static_cast<flow_actor*>(actor_ptr_), change_vswitch_route_timeout_t::value);
        break;
      case flow_actor_messages::migrate_flow_state_timeout :
        send(static_cast<flow_actor*>(actor_ptr_), migrate_flow_state_timeout_t::value);
        break;
      case flow_actor_messages::replica_recover_timeout :
        send(static_cast<flow_actor*>(actor_ptr_), replica_recover_timeout_t::value);
        break;
      default:
        assert(1==0);
        break;
    }
  }

  inline void send_coordinator_actor_message(){
    switch(static_cast<coordinator_messages>(msg_type_)){
      default:
        assert(1==0);
        break;
    }
  }

  inline void trigger(){
    assert(request_msg_id_ != invalid_message_id);
    switch(static_cast<actor_type>(type_)){
      case actor_type::coordinator_actor :
        send_coordinator_actor_message();
        break;
      case actor_type::flow_actor :
        send_flow_actor_messgae();
        break;
      default:
        break;
    }
  }

};

static_assert(std::is_pod<actor_timer<actor_timer_type::flow_actor_idle_timer>>::value,
    "actor_timer is not a pod");

static_assert(std::is_pod<actor_timer<actor_timer_type::flow_actor_req_timer>>::value,
    "actor_timer is not a pod");
#endif

#ifndef ACTOR_TIMER_LIST_H
#define ACTOR_TIMER_LIST_H

#include "actor_timer.h"
#include "./base/actor_misc.h"

template<actor_timer_type T>
class actor_timer_list{
public:

  inline void init_list(uint64_t to_ns){
    cdlist_head_init(&timer_list_head_);
    to_ns_ = to_ns;
  }

  inline void add_timer(actor_timer<T>* timer, uint64_t now_ns,
                        uint32_t request_msg_id, uint16_t msg_type){
    assert(timer->request_msg_id_ == invalid_message_id);

    timer->to_time_ = now_ns+to_ns_;
    timer->request_msg_id_ = request_msg_id;
    timer->msg_type_ = msg_type;

    cdlist_add_tail(&timer_list_head_, &(timer->list_item_));
  }

  inline bool timeout_occur(uint64_t now){
    cdlist_item* first_item = cdlist_peek_first_item(&timer_list_head_);

    if( (first_item==nullptr) || (reinterpret_cast<actor_timer<T>*>(first_item)->to_time_>=now)){
      return false;
    }

    return true;
  }

  inline void trigger_timer(){
    cdlist_item* first_item = cdlist_peek_first_item(&timer_list_head_);
    actor_timer<T>* actor_timer_ptr = reinterpret_cast<actor_timer<T>*>(first_item);
    actor_timer_ptr->trigger();

    // The invalidate should be called within the message handler
    // actor_timer_ptr->invalidate();
  }

private:
  struct cdlist_head timer_list_head_;
  uint64_t to_ns_;
};

#endif

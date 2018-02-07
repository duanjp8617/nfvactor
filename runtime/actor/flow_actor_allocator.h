#ifndef FLOW_ACTOR_ALLOCATOR_H
#define FLOW_ACTOR_ALLOCATOR_H

#include <memory>
#include <cassert>

#include "flow_actor.h"
#include "../bessport/utils/simple_ring_buffer.h"
#include "./base/actor_misc.h"

class flow_actor_allocator{

public:
  flow_actor_allocator(){
    max_actors_ = 0;
    next_flow_actor_id_ = 0;
    flow_actor_array_ = nullptr;
  }

  void init(size_t max_actors){
    assert(flow_actor_array_ == nullptr);

    max_actors_ = max_actors;
    next_flow_actor_id_ = flow_actor_id_start;
    ring_buf_.init(max_actors);

    flow_actor_array_ = static_cast<flow_actor*>(mem_alloc(sizeof(flow_actor)*max_actors));
    for(size_t i=0; i<max_actors; i++){
      flow_actor_array_[i].set_id(invalid_flow_actor_id);
      flow_actor_array_[i].get_idle_timer()->init(static_cast<uint16_t>(actor_type::flow_actor),
                                                  &flow_actor_array_[i]);
      flow_actor_array_[i].get_migration_timer()->init(static_cast<uint16_t>(actor_type::flow_actor),
                                                  &flow_actor_array_[i]);
      flow_actor_array_[i].get_replication_timer()->init(static_cast<uint16_t>(actor_type::flow_actor),
                                                  &flow_actor_array_[i]);
      flow_actor_array_[i].set_up_pkt_processing_funcs();
      flow_actor_array_[i].set_up_replication_processing_funcs();
      flow_actor_array_[i].init_cdlist_item();
      flow_actor_array_[i].init_buffer_head();
      ring_buf_.push(&flow_actor_array_[i]);
    }
  }

  ~flow_actor_allocator(){
    if(flow_actor_array_ != nullptr){
      delete[] flow_actor_array_;
    }
  }

  inline flow_actor* allocate(){
    flow_actor* new_actor = ring_buf_.pop();

    if(unlikely(new_actor==nullptr)){
      return nullptr;
    }

    new_actor->set_id(next_flow_actor_id_);
    next_flow_actor_id_+=1;

    return new_actor;
  }

  inline bool deallocate(flow_actor* flow_actor_ptr){
    flow_actor_ptr->set_id(invalid_flow_actor_id);
    return ring_buf_.push(flow_actor_ptr);
  }

  inline size_t get_max_actor(){
    return max_actors_;
  }

private:

  size_t max_actors_;

  uint32_t next_flow_actor_id_;

  simple_ring_buffer<flow_actor> ring_buf_;

  flow_actor* flow_actor_array_;
};

#endif

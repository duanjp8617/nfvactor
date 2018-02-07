#ifndef NETWORK_FUNCTION_H
#define NETWORK_FUNCTION_H

#include "../../bessport/packet.h"
#include "../../bessport/utils/simple_ring_buffer.h"
#include "../../bessport/mem_alloc.h"
#include "./flow_al_status.h"

class network_function_base{
public:

  explicit network_function_base(size_t nf_state_size, uint8_t nf_id)
    : nf_state_size_(nf_state_size), array_(0), ring_buf_(), nf_id_(nf_id){
  }

  inline void init_ring(size_t nf_state_num){
    array_ = reinterpret_cast<char*>(mem_alloc(nf_state_size_*nf_state_num));
    ring_buf_.init(nf_state_num);
    for(size_t i=0; i<nf_state_num; i++){
      ring_buf_.push(array_+i*nf_state_size_);
    }
  }

  virtual ~network_function_base(){}

  inline char* allocate(){
    return ring_buf_.pop();
  }

  inline bool deallocate(char* state_ptr){
    return ring_buf_.push(state_ptr);
  }

  virtual bool nf_logic(bess::Packet* pkt, char* state_ptr) = 0;

  // used when the flow is migrated to the target, or
  // when the flow is bring back alive on the replica
  virtual void nf_flow_arrive(char* state_ptr, flow_arrive_status status) = 0;

  // used when the flow becomes idle, or when the flow
  // is migrated out from the source.
  virtual void nf_flow_leave(char* state_ptr, flow_leave_status status) = 0;

  // initialize the flow state
  virtual void nf_init_fs(char* state_ptr) = 0;

  inline size_t get_nf_state_size(){
    return nf_state_size_;
  }

  inline uint8_t get_nf_id(){
    return nf_id_;
  }

private:
  size_t nf_state_size_;
  char* array_;
  simple_ring_buffer<char> ring_buf_;
  uint8_t nf_id_;
};

#endif

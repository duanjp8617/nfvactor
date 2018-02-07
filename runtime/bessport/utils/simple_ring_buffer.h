#ifndef SIMPLE_RING_BUFFER_H
#define SIMPLE_RING_BUFFER_H

#include <cassert>

#include "../mem_alloc.h"

// This is a simple ring buffer implementation,
// with no concurrent support. The ring buffer
// only stores pointer to object. It doesn't
// take care of the allocation/deallocation of the
// objects.
template<class T>
class simple_ring_buffer{

public:
  simple_ring_buffer(size_t max_size) :
    head_pos_(0), tail_pos_(0), cur_size_(0), max_size_(max_size){
    ring_buf_ = static_cast<T**>(mem_alloc(max_size*sizeof(T*)));
  }

  simple_ring_buffer() :
    head_pos_(0), tail_pos_(0), cur_size_(0), max_size_(0), ring_buf_(nullptr){
  }

  inline void init(size_t max_size){
    assert(ring_buf_ == nullptr);
    head_pos_ = 0;
    tail_pos_ = 0;
    cur_size_ = 0;
    max_size_ = max_size;
    ring_buf_ = static_cast<T**>(mem_alloc(max_size*sizeof(T*)));
  }

  ~simple_ring_buffer(){
    if(ring_buf_!=nullptr){
      mem_free(ring_buf_);
    }
  }

  inline size_t get_cur_size(){
    return cur_size_;
  }

  inline T* pop(){
    if(cur_size_ == 0){
      return nullptr;
    }
    else{
      T* return_val = ring_buf_[head_pos_];
      head_pos_ = ((head_pos_+1)%max_size_);
      cur_size_-=1;
      return return_val;
    }
  }

  inline bool push(T* obj_ptr){
    if(cur_size_==max_size_){
      return false;
    }
    else{
      ring_buf_[tail_pos_] = obj_ptr;
      tail_pos_ = ((tail_pos_+1)%max_size_);
      cur_size_+=1;
      return true;
    }
  }

private:
  size_t head_pos_;
  size_t tail_pos_;
  size_t cur_size_;
  size_t max_size_;

  T** ring_buf_;
};

#endif

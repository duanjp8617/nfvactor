#ifndef GENERIC_RING_ALLOCATOR
#define GENERIC_RING_ALLOCATOR

#include <cassert>
#include "../bessport/utils/simple_ring_buffer.h"

template<class T>
class generic_ring_allocator{
public:

  generic_ring_allocator(){
    max_size_ = 0;
    obj_array_ = nullptr;
  }

  void init(size_t size){
    assert(obj_array_ == nullptr);
    max_size_ = size;
    obj_array_ = static_cast<T*>(mem_alloc(size*sizeof(T)));
    assert(obj_array_!=nullptr);
    ring_buf_.init(size);
    for(size_t i=0; i<size; i++){
      ring_buf_.push(&obj_array_[i]);
    }
  }

  ~generic_ring_allocator(){
    if(obj_array_ != nullptr){
      delete[] obj_array_;
    }
  }

  inline T* allocate(){
    return ring_buf_.pop();
  }

  inline bool deallocate(T* obj_ptr){
    return ring_buf_.push(obj_ptr);
  }

private:

  static_assert(std::is_pod<T>::value, "The type argument passed to generic_ring_allocator is not POD");

  size_t max_size_;

  simple_ring_buffer<T> ring_buf_;

  T* obj_array_;
};

#endif

#ifndef FIXED_ARRAY_H
#define FIXED_ARRAY_H

#include <rte_memcpy.h>

#include "../bessport/mem_alloc.h"

static constexpr size_t relloc_size = 32;

template<class T>
class fixed_array{
public:
  static_assert(std::is_pod<T>::value, "T is not a POD");

  fixed_array() : array_(0), array_size_(0), cur_size_(0){};

  void init(size_t initial_array_size){
    array_ = reinterpret_cast<T*>(mem_alloc(sizeof(T)*initial_array_size));
    array_size_ = initial_array_size;
    cur_size_ = 0;
  }

  void add(T* new_val){
    if(cur_size_==array_size_){
      array_ = reinterpret_cast<T*>(mem_realloc(array_, array_size_+relloc_size));
      array_size_ += relloc_size;
    }

    array_[cur_size_] = *new_val;
    cur_size_ += 1;
  }

  void remove(size_t pos){
    if(pos>=cur_size_){
      return;
    }

    if(pos==cur_size_-1){
      cur_size_-=1;
      return;
    }

    rte_memcpy(&array_[pos], &array_[pos+1], sizeof(T)*(cur_size_-1-pos));
    cur_size_-=1;
  }

  size_t size(){
    return cur_size_;
  }

  T* get(size_t pos){
    if(pos>=cur_size_){
      return nullptr;
    }

    return &array_[pos];
  }

private:
  T* array_;
  size_t array_size_;
  size_t cur_size_;
};

#endif

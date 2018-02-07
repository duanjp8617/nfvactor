#ifndef ROUND_RUBIN_LIST_H
#define ROUND_RUBIN_LIST_H

#include "cdlist.h"

template<class T>
class round_rubin_list{
public:
  round_rubin_list(){
    cdlist_head_init(&rr_list_head_);
  }

  inline void add_to_tail(T* obj_ptr){
    cdlist_add_tail(&rr_list_head_, reinterpret_cast<struct cdlist_item*>(obj_ptr));
  }

  inline void add_to_head(T* obj_ptr){
    cdlist_add_head(&rr_list_head_, reinterpret_cast<struct cdlist_item*>(obj_ptr));
  }

  inline T* peek_head(){
    struct cdlist_item* item = cdlist_peek_first_item(&rr_list_head_);
    if(item == nullptr){
      return nullptr;
    }
    else{
      return reinterpret_cast<T*>(item);
    }
  }

  inline T* peek_tail(){
    struct cdlist_item* item = cdlist_peek_last_item(&rr_list_head_);
    if(item == nullptr){
      return nullptr;
    }
    else{
      return reinterpret_cast<T*>(item);
    }
  }

  inline T* pop_head(){
    struct cdlist_item* item = cdlist_pop_head(&rr_list_head_);
    if(item == nullptr){
      return nullptr;
    }
    else{
      return reinterpret_cast<T*>(item);
    }
  }

  inline T* rotate(){
    struct cdlist_item* item = cdlist_rotate_left(&rr_list_head_);
    if(item == nullptr){
      return nullptr;
    }
    else{
      return reinterpret_cast<T*>(item);
    }
  }

  inline struct cdlist_head* get_list_head(){
    return &rr_list_head_;
  }

private:
  struct cdlist_head rr_list_head_;

  static_assert(std::is_pod<T>::value, "The type argument passed to round_rubin_list is not POD");
};

#endif

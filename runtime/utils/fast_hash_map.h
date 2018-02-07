#ifndef FAST_HASH_MAP_H
#define FAST_HASH_MAP_H

#include "cdlist.h"
#include "hash_functions.h"
#include "../bessport/utils/htable.h"

template<class ValType>
struct hash_map_list_item{
  cdlist_item list_item;
  ValType* val_ptr;
};

template<class KeyType,
         class ValType,
         HTableBase::KeyCmpFunc C,
         HTableBase::HashFunc H>
class fast_hash_map{
public:
  static_assert(std::is_pod<KeyType>::value, "Key type is not a POD");
  static_assert(std::is_pod<hash_map_list_item<ValType>>::value, "Value type is not a POD");

  fast_hash_map(){
    cdlist_head_init(&list_head_);
    htable_.Init(sizeof(KeyType), sizeof(hash_map_list_item<ValType>*));
  }

  ~fast_hash_map(){
    cdlist_item* next_list_item = list_head_.next;
    while(next_list_item != reinterpret_cast<cdlist_item*>(&list_head_)){
      cdlist_item* current_list_item = next_list_item;
      next_list_item = next_list_item->next;

      hash_map_list_item<ValType>* hmap_list_item =
          reinterpret_cast<hash_map_list_item<ValType>*>(current_list_item);

      free(hmap_list_item->val_ptr);
      free(hmap_list_item);
    }
    htable_.Close();
  }

  template<class... Args>
  ValType* emplace(KeyType key, Args&&... args){
    hash_map_list_item<ValType>** list_item_ptr_ptr = htable_.Get(&key);
    if(unlikely(list_item_ptr_ptr != nullptr)){
      return nullptr;
    }

    hash_map_list_item<ValType>* hmap_list_item = new hash_map_list_item<ValType>;
    hmap_list_item->val_ptr = new ValType(std::forward<Args>(args)...);

    cdlist_add_tail(&list_head_, &(hmap_list_item->list_item));
    htable_.Set(&key, &hmap_list_item);

    return hmap_list_item->val_ptr;
  }

  void erase(KeyType key){
    hash_map_list_item<ValType>** list_item_ptr_ptr = htable_.Get(&key);
    if(unlikely(list_item_ptr_ptr==nullptr)){
      return;
    }

    hash_map_list_item<ValType>* list_item_ptr = *list_item_ptr_ptr;
    cdlist_del(&(list_item_ptr->list_item));
    htable_.Del(&key);

    free(list_item_ptr->val_ptr);
    free(list_item_ptr);
  }

  ValType* find(KeyType key){
    hash_map_list_item<ValType>** list_item_ptr_ptr = htable_.Get(&key);
    if(unlikely(list_item_ptr_ptr == nullptr)){
      return nullptr;
    }

    return (*list_item_ptr_ptr)->val_ptr;
  }

  int cnt(){
    return htable_.Count();
  }

  ValType* next(){
    cdlist_item* list_item = cdlist_rotate_left(&list_head_);
    if(unlikely(list_item == nullptr)){
      return nullptr;
    }

    return reinterpret_cast<hash_map_list_item<ValType>*>(list_item)->val_ptr;
  }

private:
  cdlist_head list_head_;
  HTable<KeyType, hash_map_list_item<ValType>*, C, H> htable_;
};

#endif

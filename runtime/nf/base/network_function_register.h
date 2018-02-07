#ifndef NETWORK_FUNCTION_REGISTER_H
#define NETWORK_FUNCTION_REGISTER_H

#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include <iostream>

#include "network_function_derived.h"

class network_function_register{
public:

  template<class NF, class SS, class NFS, class... NFArgs>
  bool register_nf(std::string nf_name, uint8_t nf_id, NFArgs&&... args){
    if( (name_id_map_.find(nf_name) != name_id_map_.end()) ||
        (id_nf_map_.find(nf_id) != id_nf_map_.end()) ){
      std::cout<<"[FATAL ERROR] nf_name : "<<nf_name<<" or nf_id : "<<nf_id<<" has been used."<<std::endl;
      assert(false);
      return false;
    }
    name_id_map_.emplace(nf_name, nf_id);

    std::unique_ptr<network_function_base> ptr(new network_function_derived<NF, SS, NFS>(nf_id,
                                                                                     std::forward<NFArgs>(args)...));

    id_nf_map_.emplace(nf_id, std::move(ptr));
    return true;
  }

  std::vector<network_function_base*> get_service_chain(uint64_t service_chain_type_sig);

  void init(size_t nf_state_num);

  uint8_t compute_network_function(uint64_t s, int pos);

  int compute_service_chain_length(uint64_t s);

  uint8_t look_up_id(std::string str);

private:
  std::unordered_map<std::string, uint8_t> name_id_map_;
  std::unordered_map<uint8_t, std::unique_ptr<network_function_base>> id_nf_map_;
};

class static_nf_register{
public:
  static network_function_register& get_register();
};

#endif

#include <cassert>

#include <glog/logging.h>

#include "network_function_register.h"

std::vector<network_function_base*>
network_function_register::get_service_chain(uint64_t service_chain_type_sig){
  std::vector<network_function_base*> v;
  for(int i=0; i<compute_service_chain_length(service_chain_type_sig); i++){
    uint8_t nf_id = compute_network_function(service_chain_type_sig, i);
    v.push_back(id_nf_map_.find(nf_id)->second.get());
  }
  return v;
}

void network_function_register::init(size_t nf_state_num){
  for(auto it = id_nf_map_.begin(); it!=id_nf_map_.end(); it++){
    it->second->init_ring(nf_state_num);
  }
}

uint8_t network_function_register::compute_network_function(uint64_t s, int pos){
  return static_cast<uint8_t>((s>>(8*pos))&0x00000000000000FF);
}

int network_function_register::compute_service_chain_length(uint64_t s){
  int length = 0;
  bool encounter_zero = false;
  for(int i=0; i<8; i++){
    uint8_t nf =
        static_cast<uint8_t>((s>>(8*i))&0x00000000000000FF);
    if(nf>0){
      length+=1;
      if(encounter_zero){
        return -1;
      }
    }
    else{
      encounter_zero = true;
    }
  }
  return length;
}



uint8_t network_function_register::look_up_id(std::string str){

	auto it=name_id_map_.find(str);
	if(it==name_id_map_.end()){
		return 0;
	}else{
		return it->second;
	}
}

network_function_register& static_nf_register::get_register(){
  static network_function_register reg;

  return reg;
}

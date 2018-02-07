#ifndef NAT_STATE_H
#define NAT_STATE_H

#include <forward_list>
#include <string>

#include "../../nfaflags.h"

// a simple source nat

struct ip_port_tuple{
  uint32_t src_ip;
  uint16_t src_port;
};

static constexpr uint8_t no_nat_allocation = 1;
static constexpr uint8_t has_nat_allocation = 2;
static constexpr uint8_t nat_allocation_exception = 3;

struct nat_fs{
  ip_port_tuple ip_port;
  uint8_t nat_status;
};

class nat_shared_state{
public:
  nat_shared_state(){
    // Here, the FLAGS_runtime_id is different for each runtime
    // we set the source address pool to be
    // 101.122.FLAGS_runtime_id.(1~10) with port being 5001~65000

    for(int i=0; i<10; i++){
      std::string ip = "101.122."+std::to_string(FLAGS_runtime_id)+"."+std::to_string(i+1);
      uint32_t src_ip_network_representation = convert_string_ip(ip);
      for(uint16_t j=5001; j<65001; j++){
        uint16_t src_port_network_representation = lton(j);
        ip_port_tuple ip_port;
        ip_port.src_ip = src_ip_network_representation;
        ip_port.src_port = src_port_network_representation;
        address_pool_.push_front(ip_port);
      }
    }
  }

  ip_port_tuple nat_allocate(){
    if(address_pool_.empty()){
      ip_port_tuple ip_port;
      ip_port.src_ip = 0;
      ip_port.src_port = 0;
      return ip_port;
    }
    else{
      ip_port_tuple ip_port = address_pool_.front();
      address_pool_.pop_front();
      return ip_port;
    }
  }

  void nat_deallocate(ip_port_tuple& ip_port){
    address_pool_.push_front(ip_port);
  }

private:
  uint16_t lton(uint16_t local_representation){
    char buf[2];
    char* ptr = (char*)(&local_representation);

    buf[1] = ptr[0];
    buf[0] = ptr[1];

    uint16_t network_representation = *((uint16_t*)buf);
    return network_representation;
  }

  uint32_t convert_string_ip(std::string ip){
    std::stringstream ss(ip);
    uint32_t a,b,c,d;
    char ch;
    ss >> a >> ch >> b >> ch >> c >> ch >> d;
    return  (((d<<24)&0xFF000000) |
             ((c<<16)&0x00FF0000) |
             ((b<< 8)&0x0000FF00) |
             ( a     &0x000000FF));
  }

  std::forward_list<ip_port_tuple> address_pool_;
};

#endif

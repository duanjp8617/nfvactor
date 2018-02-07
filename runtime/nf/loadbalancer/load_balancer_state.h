#ifndef LOAD_BALANCER_STATE_H
#define LOAD_BALANCER_STATE_H

#include <vector>
#include <string>
#include <sstream>
#include <algorithm> // std::min_element
#include <iterator>  // std::begin, std::end

// an IP based load balancer
struct load_balancer_fs{
  int server_pos;
  uint32_t server_ip_addr;
};

class load_balancer_shared_state{
public:
  load_balancer_shared_state(){
    add_server("192.168.1.1");
    add_server("192.168.1.2");
    add_server("192.168.1.3");
    add_server("192.168.1.4");
    add_server("192.168.1.5");
    add_server("192.168.1.6");
  }

  uint32_t get_server_ip(int server_pos){
    return servers_[server_pos];
  }

  void add_server_workload(int server_pos){
    server_workload_[server_pos]+=1;
  }

  void decrease_server_workload(int server_pos){
    if(server_workload_[server_pos]>0){
      server_workload_[server_pos]-=1;
    }
  }

  int find_smallest_workload_server(){
    auto result = std::min_element(std::begin(server_workload_), std::end(server_workload_));
    return *result;
  }

private:
  void add_server(std::string server_ip){
    servers_.push_back(convert_string_ip(server_ip));
    server_workload_.push_back(0);
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

  std::vector<uint32_t> servers_;
  std::vector<uint64_t> server_workload_;
};

#endif

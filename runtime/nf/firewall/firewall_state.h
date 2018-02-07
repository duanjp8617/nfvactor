#ifndef FIREWALL_STATE_H
#define FIREWALL_STATE_H

#include <cstdint>
#include <string>
#include <sstream>

static constexpr uint8_t no_conneciton = 0;
static constexpr uint8_t connection_up = 1;
static constexpr uint8_t connection_down = 2;
static constexpr uint8_t bad_connection = 3;

static constexpr uint8_t protocol_tcp = 0x06;
static constexpr uint8_t protocol_udp = 0x11;

struct firewall_fs{
  // basic flow information
  uint32_t src_ip;
  uint32_t dst_ip;
  uint16_t src_port;
  uint16_t dst_port;
  uint8_t protocol;

  // counters
  uint32_t passed_pkts;
  uint32_t dropped_packets;

  // connection status
  uint8_t connection_status;
};

// a simple definition for a firewall rule
struct firewall_rule_type {
  uint32_t src_ip_;
  uint32_t src_mask_;
  uint32_t dest_ip_;
  uint32_t dest_mask_;

  // need change!!!!!
  firewall_rule_type(std::string src_ip, int src_mask_length,
      std::string dest_ip, int dest_mask_length){
    src_ip_ = convert_string_ip(src_ip);
    src_mask_ = 0xFFFFFFFF - uint32_t((1 << (32-src_mask_length))-1);
    dest_ip_ = convert_string_ip(dest_ip);
    dest_mask_ = 0xFFFFFFFF - uint32_t((1 << (32-dest_mask_length))-1);
  }

  bool match(uint32_t src_ip, uint32_t dest_ip){
    if( ((src_ip&src_mask_) == (src_ip_&src_mask_)) ||
        ((dest_ip&dest_mask_) == (dest_ip_&dest_mask_)) ){
      return true;
    }
    else{
      return false;
    }
  }

  uint32_t convert_string_ip(std::string ip){
    std::stringstream ss(ip);
    uint32_t a,b,c,d;
    char ch;
    ss >> a >> ch >> b >> ch >> c >> ch >> d;
    return  (((a<<24)&0xFF000000) |
             ((b<<16)&0x00FF0000) |
             ((c<< 8)&0x0000FF00) |
             ( d     &0x000000FF));
  }
};

// the shared state holds all the firewall rule, currently we use a static
// way to initialize all the firewall rules.
class firewall_shared_state{
public:
  firewall_shared_state() :
    rule1("117.7.8.1", 32, "0.0.0.0", 32),
    rule2("157.8.8.1", 32, "0.0.0.0", 32),
    rule3("127.9.8.1", 32, "0.0.0.0", 32),
    rule4("137.10.8.1", 32, "0.0.0.0", 32),
    rule5("147.11.8.1", 32, "0.0.0.0", 32){

  }

  bool match(uint32_t src_ip, uint32_t dest_ip){
    if( rule1.match(src_ip, dest_ip) ||
        rule2.match(src_ip, dest_ip) ||
        rule3.match(src_ip, dest_ip) ||
        rule4.match(src_ip, dest_ip) ||
        rule5.match(src_ip, dest_ip)   ){
      return true;
    }
    else{
      return false;
    }
  }

private:
  firewall_rule_type rule1;
  firewall_rule_type rule2;
  firewall_rule_type rule3;
  firewall_rule_type rule4;
  firewall_rule_type rule5;
};

#endif

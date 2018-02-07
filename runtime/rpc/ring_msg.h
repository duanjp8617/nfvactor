#ifndef RING_MSG_H
#define RING_MSG_H

#include <string>
#include <sstream>
#include <iostream>

#include <glog/logging.h>

#include "../bessport/nfa_msg.grpc.pb.h"
#include "../bessport/mem_alloc.h"

using std::string;
using std::stringstream;
using std::istreambuf_iterator;

using nfa_msg::RuntimeConfig;

struct runtime_config{
  int32_t runtime_id;
  uint64_t input_port_mac;
  uint64_t output_port_mac;
  uint64_t control_port_mac;
  uint32_t rpc_ip;
  int32_t rpc_port;

  inline bool operator==(const runtime_config& rhs){
    if( (this->runtime_id == rhs.runtime_id) &&
        (this->input_port_mac == rhs.input_port_mac) &&
        (this->output_port_mac == rhs.output_port_mac) &&
        (this->control_port_mac == rhs.control_port_mac) &&
        (this->rpc_ip == rhs.rpc_ip) &&
        (this->rpc_port == rhs.rpc_port) ){
      return true;
    }
    else{
      return false;
    }
  }

  inline bool operator!=(const runtime_config& rhs){
    return !((*this) == rhs);
  }
};

inline int ring_msg_parse_mac_addr(const char *str, char *addr){
  if (str != NULL && addr != NULL) {
    int r = sscanf(str,
             "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx",
             addr,
             addr+1,
             addr+2,
             addr+3,
             addr+4,
             addr+5);

    if (r != 6)
      return -EINVAL;
  }

  return 0;
}

inline uint64_t l2_addr_to_u64(char *addr) {
  uint64_t *addrp = reinterpret_cast<uint64_t *>(addr);
  return (*addrp & 0x0000FFffFFffFFfflu);
}

inline uint64_t convert_string_mac(const string& mac){
  char char_mac[6];
  ring_msg_parse_mac_addr(mac.c_str(), char_mac);
  return l2_addr_to_u64(char_mac);
}

inline uint32_t convert_string_ip(const string& ip){
  stringstream ss(ip);
  uint32_t a,b,c,d;
  char ch;
  ss >> a >> ch >> b >> ch >> c >> ch >> d;
  return  (((a<<24)&0xFF000000) |
           ((b<<16)&0x00FF0000) |
           ((c<< 8)&0x0000FF00) |
           ( d     &0x000000FF));
}

inline string convert_uint64t_mac(uint64_t mac){
  char str[19];
  char* array = reinterpret_cast<char*>(&mac);
  sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",array[0],
          array[1], array[2], array[3], array[4],array[5]);

  return string(str);
}

inline string convert_uint32t_ip(uint32_t ip){
  uint32_t a,b,c,d;
  a = (ip>>24)&0x000000FF;
  b = (ip>>16)&0x000000FF;
  c = (ip>>8)&0x000000FF;
  d = ip&0x000000FF;
  stringstream ss("");
  ss<<a<<"."<<b<<"."<<c<<"."<<d;
  string ret(istreambuf_iterator<char>(ss), {});
  return ret;
}

inline runtime_config protobuf2local(const RuntimeConfig &protobuf_msg){
  runtime_config rc;
  rc.runtime_id = protobuf_msg.runtime_id();
  rc.input_port_mac = convert_string_mac(protobuf_msg.input_port_mac());
  rc.output_port_mac = convert_string_mac(protobuf_msg.output_port_mac());
  rc.control_port_mac = convert_string_mac(protobuf_msg.control_port_mac());
  rc.rpc_ip = convert_string_ip(protobuf_msg.rpc_ip());
  rc.rpc_port = protobuf_msg.rpc_port();
  return rc;
}

inline RuntimeConfig local2protobuf(runtime_config local_msg){
  RuntimeConfig rc;
  rc.set_runtime_id(local_msg.runtime_id);
  rc.set_input_port_mac(convert_uint64t_mac(local_msg.input_port_mac));
  rc.set_output_port_mac(convert_uint64t_mac(local_msg.output_port_mac));
  rc.set_control_port_mac(convert_uint64t_mac(local_msg.control_port_mac));
  rc.set_rpc_ip(convert_uint32t_ip(local_msg.rpc_ip));
  rc.set_rpc_port(local_msg.rpc_port);
  return rc;
}

struct storage_stat{
  uint64_t replication_source_runtime_id;
  uint64_t num_of_flow_replicas;
  uint64_t total_replay_time;
};

struct runtime_stat{
  uint64_t input_port_incoming_pkts;
  uint64_t input_port_outgoing_pkts;
  uint64_t input_port_dropped_pkts;

  uint64_t output_port_incoming_pkts;
  uint64_t output_port_outgoing_pkts;
  uint64_t output_port_dropped_pkts;


  uint64_t control_port_incoming_pkts;
  uint64_t control_port_outgoing_pkts;
  uint64_t control_port_dropped_pkts;

  uint64_t active_flows;
  uint64_t inactive_flows;

  uint64_t migration_index;
  uint64_t migration_target_runtime_id;
  uint64_t migration_qouta;
  uint64_t average_flow_migration_completion_time;
  uint64_t toal_flow_migration_completion_time;
  uint64_t successful_migration;

  storage_stat* array;
  uint64_t array_size;

  runtime_stat(uint64_t size) :
    input_port_incoming_pkts(0),
    input_port_outgoing_pkts(0),
    input_port_dropped_pkts(0),
    output_port_incoming_pkts(0),
    output_port_outgoing_pkts(0),
    output_port_dropped_pkts(0),
    control_port_incoming_pkts(0),
    control_port_outgoing_pkts(0),
    control_port_dropped_pkts(0),
    active_flows(0),
    inactive_flows(0),
    migration_index(0),
    migration_target_runtime_id(0),
    migration_qouta(0),
    average_flow_migration_completion_time(0),
    toal_flow_migration_completion_time(0),
    successful_migration(0){
    if(size>0){
      array = static_cast<storage_stat*>(mem_alloc(sizeof(storage_stat)*size));
    }
    else{
      array=nullptr;
    }
    array_size = size;
  }

  ~runtime_stat(){
    if(array!=nullptr){
      mem_free(array);
    }
  }
};

enum class rpc_operation{
  add_input_runtime,
  add_output_runtime,
  delete_input_runtime,
  delete_output_runtime,
  add_input_mac,
  add_output_mac,
  delete_input_mac,
  delete_output_mac,
  migrate_to,
  set_migration_target,
  migration_negotiate,
  delete_migration_target,
  delete_migration_source,
  add_replica,
  add_storage,
  remove_replica,
  remove_storage,
  recover,
  get_stats
};

struct llring_item{
  rpc_operation op_code;
  runtime_config rt_config;
  uint64_t migration_qouta;
  runtime_stat stat;

  llring_item(rpc_operation code,
              runtime_config config,
              uint64_t qouta,
              uint64_t replica_num) :
                op_code(code),
                rt_config(config),
                migration_qouta(qouta),
                stat(replica_num){}
};

inline string opcode2string(rpc_operation code){
  string return_val = "";
  switch (code){
    case rpc_operation::add_input_runtime:
      return_val = "add_input_runtime";
      break;
    case rpc_operation::add_output_runtime:
      return_val = "add_output_runtime";
      break;
    case rpc_operation::delete_input_runtime:
      return_val = "delete_input_runtime";
      break;
    case rpc_operation::delete_output_runtime:
      return_val = "delete_output_runtime";
      break;
    case rpc_operation::add_input_mac:
      return_val = "add_input_mac";
      break;
    case rpc_operation::add_output_mac:
      return_val = "add_output_mac";
      break;
    case rpc_operation::delete_input_mac:
      return_val = "delete_input_mac";
      break;
    case rpc_operation::delete_output_mac:
      return_val = "delete_output_mac";
      break;
    case rpc_operation::migrate_to:
      return_val = "migrate_to";
      break;
    case rpc_operation::set_migration_target:
      return_val = "set_migration_target";
      break;
    case rpc_operation::migration_negotiate:
      return_val = "migration_negotiate";
      break;
    case rpc_operation::delete_migration_source:
      return_val = "delete_migration_source";
      break;
    case rpc_operation::delete_migration_target:
      return_val = "delete_migration_target";
      break;
    case rpc_operation::add_replica:
      return_val = "add_replica";
      break;
    case rpc_operation::add_storage:
      return_val = "add_storage";
      break;
    case rpc_operation::remove_replica:
      return_val = "remove_replica";
      break;
    case rpc_operation::remove_storage:
      return_val = "remove_storage";
      break;
    case rpc_operation::recover:
      return_val = "recover";
      break;
    case rpc_operation::get_stats:
      return_val = "get_stats";
      break;
    default:
      return_val = "wtf??";
      break;
  }
  return return_val;
}

inline void print_config(runtime_config config){
  LOG(INFO) << "runtime_config-> "
            << "id:"<<config.runtime_id<<" "
            << "input_port_mac:"<<convert_uint64t_mac(config.input_port_mac)<<" "
            << "output_port_mac:"<<convert_uint64t_mac(config.output_port_mac)<<" "
            << "control_port_mac:"<<convert_uint64t_mac(config.control_port_mac)<<" "
            << "rpc_ip:"<<convert_uint32t_ip(config.rpc_ip)<<" "
            << "rpc_port:"<<std::to_string(config.rpc_port);
}

inline void print_stat(rpc_operation op_code, runtime_stat& stat){
  if(op_code == rpc_operation::get_stats){
    LOG(INFO) << "runtime_stat-> "
              << "input_port_incoming_pkts:"<<stat.input_port_incoming_pkts<<" ";
    for(uint64_t i=0; i<stat.array_size; i++){
      LOG(INFO) << "storage_stat-> "
                << "replication_source_runtime_id:"<<stat.array[i].replication_source_runtime_id<<" ";
    }
  }
}



#endif

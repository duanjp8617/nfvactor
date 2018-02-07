#ifndef LOAD_BALANCER_H
#define LOAD_BALANCER_H

#include "../../bessport/packet.h"
#include "load_balancer_state.h"
#include "../base/flow_al_status.h"

#include <glog/logging.h>

class load_balancer{
public:
  inline bool nf_logic_impl(bess::Packet* pkt, load_balancer_fs* fs, load_balancer_shared_state* ss){
    // comment out the following code for multi-threaded actor
    if(fs->server_pos == -1){
      //LOG(INFO)<<"New flow, no server, allocate one";
      fs->server_pos = ss->find_smallest_workload_server();
      fs->server_ip_addr = ss->get_server_ip(fs->server_pos);

      ss->add_server_workload(fs->server_pos);
    }

    //LOG(INFO)<<"change the ip to the server";
    //perform a nat
    *(pkt->head_data<uint32_t*>(14+16)) = fs->server_ip_addr;

    return true;
  }

  inline void nf_flow_arrive_impl(load_balancer_fs* fs, load_balancer_shared_state* ss, flow_arrive_status status){
    if(fs->server_pos != -1){
      //LOG(INFO)<<"Flow arrive, the flow is directed to a server, increase the workload.";
      ss->add_server_workload(fs->server_pos);
    }
  }

  inline void nf_flow_leave_impl(load_balancer_fs* fs, load_balancer_shared_state* ss, flow_leave_status status){
    if(fs->server_pos != -1){
      //LOG(INFO)<<"Flow leave, the flow is directed to a server, decrease the workload";
      ss->decrease_server_workload(fs->server_pos);
    }
  }

  inline void nf_init_fs_impl(load_balancer_fs* fs){
    //LOG(INFO)<<"New flow arrive, initiate loadbalancer flow state";
    fs->server_pos = -1;
  }

};

#endif

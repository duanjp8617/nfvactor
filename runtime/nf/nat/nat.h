#ifndef NAT_H
#define NAT_H

#include "../../bessport/packet.h"
#include "nat_state.h"
#include "../base/flow_al_status.h"

#include <glog/logging.h>

class nat{
public:
  inline bool nf_logic_impl(bess::Packet* pkt, nat_fs* fs, nat_shared_state* ss){
    bool result = true;

    switch(fs->nat_status){
      case no_nat_allocation:{
        //LOG(INFO)<<"Receive the first packet";

        // comment out the following lines until
        // "fs->ip_port = ip_port"
        // for multi-threaded nat
        ip_port_tuple ip_port = ss->nat_allocate();

        if(ip_port.src_ip == 0 && ip_port.src_port == 0){
          //LOG(INFO)<<"nat allocation fails, change status";
          fs->nat_status = nat_allocation_exception;
          result = false;
          break;
        }

        //LOG(INFO)<<"nat allocation succeeed";
        fs->ip_port = ip_port;
        fs->nat_status = has_nat_allocation;

        //LOG(INFO)<<"perform nat";
        *(pkt->head_data<uint32_t*>(14+12)) = fs->ip_port.src_ip;
        *(pkt->head_data<uint16_t*>(14+20)) = fs->ip_port.src_port;
        result = true;
        break;
      }
      case has_nat_allocation: {
        //LOG(INFO)<<"perform nat";
        *(pkt->head_data<uint32_t*>(14+12)) = fs->ip_port.src_ip;
        *(pkt->head_data<uint16_t*>(14+20)) = fs->ip_port.src_port;
        result = true;
        break;
      }
      case nat_allocation_exception: {
        //LOG(INFO)<<"nat exception, drop packet";
        result = false;
        break;
      }
    }

    return result;
  }

  inline void nf_flow_arrive_impl(nat_fs* fs, nat_shared_state* ss, flow_arrive_status status){
    //LOG(INFO)<<"flow arrive, nothing happen";
  }

  inline void nf_flow_leave_impl(nat_fs* fs, nat_shared_state* ss, flow_leave_status status){
    switch(status){
      case flow_leave_status::idle : {
        //LOG(INFO)<<"flow leaves in idle state";
        if(fs->nat_status == has_nat_allocation){
          //LOG(INFO)<<"the flow has a nat, deallocate it";
          ss->nat_deallocate(fs->ip_port);
        }
        break;
      }
      default:
        break;
    }
  }

  inline void nf_init_fs_impl(nat_fs* fs){
    //LOG(INFO)<<"new flow arrives, initiate nat flow state";
    fs->nat_status = no_nat_allocation;
  }
};

#endif

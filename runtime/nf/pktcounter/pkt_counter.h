#ifndef PKT_COUNTER_H
#define PKT_COUNTER_H

#include "../../bessport/packet.h"
#include "pkt_counter_fs.h"
#include "../base/flow_al_status.h"

class pkt_counter{
public:
  inline bool nf_logic_impl(bess::Packet* pkt, pkt_counter_fs* fs, pkt_counter_shared_state* ss){
    fs->counter += 1;
    return true;
  }

  inline void nf_flow_arrive_impl(pkt_counter_fs* fs, pkt_counter_shared_state* ss, flow_arrive_status status){

  }

  inline void nf_flow_leave_impl(pkt_counter_fs* fs, pkt_counter_shared_state* ss, flow_leave_status status){

  }

  inline void nf_init_fs_impl(pkt_counter_fs* fs){
    fs->counter = 0;
  }

};

#endif

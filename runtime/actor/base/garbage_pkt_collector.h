#ifndef GABAGE_PKT_COLLECTOR_H
#define GABAGE_PKT_COLLECTOR_H

#include "../../bessport/packet.h"

class garbage_pkt_collector{
public:
  inline void collect(bess::Packet* pkt){
    if(garbage_pkt_batch.cnt() == bess::PacketBatch::kMaxBurst){
      bess::Packet::Free(&garbage_pkt_batch);
      garbage_pkt_batch.clear();
    }
    garbage_pkt_batch.add(pkt);
  }

  inline void collect(bess::PacketBatch* batch){
    if((garbage_pkt_batch.cnt()+batch->cnt())>32){
      bess::Packet::Free(&garbage_pkt_batch);
      garbage_pkt_batch.clear();
    }
    for(int i=0; i<batch->cnt(); i++){
      garbage_pkt_batch.add(batch->pkts()[i]);
    }
  }

  inline void init(){
    garbage_pkt_batch.clear();
  }


private:
  bess::PacketBatch garbage_pkt_batch;
};

#endif

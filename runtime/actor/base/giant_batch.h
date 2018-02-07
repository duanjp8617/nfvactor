#ifndef GIANT_BATCH_H
#define GIANT_BATCH_H

#include "../../bessport/packet.h"
#include "actor_misc.h"

class giant_batch{
public:
  void clear(){
    batch_pos_ = 0;
    gate_pos_ = 0;
    batches_[batch_pos_].clear();
  }

  void add_pkt_set_gate(bess::Packet* pkt, uint16_t gate){
    gates_[gate_pos_] = gate;
    gate_pos_+=1;
    batches_[batch_pos_].add(pkt);

    if(batches_[batch_pos_].full()&&(batch_pos_<buffer_batch_size-1)){
      batch_pos_+=1;
      batches_[batch_pos_].clear();
    }
  }

  bess::PacketBatch* get_batch(int pos){
    return &batches_[pos];
  }

  uint16_t* get_gate(int pos){
    return &gates_[pos*bess::PacketBatch::kMaxBurst];
  }

  int get_batch_pos(){
    return batch_pos_;
  }

private:
  bess::PacketBatch batches_[buffer_batch_size];
  uint16_t gates_[bess::PacketBatch::kMaxBurst*buffer_batch_size];
  int batch_pos_;
  int gate_pos_;

};

#endif

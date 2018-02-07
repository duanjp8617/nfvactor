//
#include "send_reliable_ack.h"
#include "../bessport/utils/time.h"

#include <glog/logging.h>

void send_reliable_ack::customized_init(coordinator* coordinator_actor){
  RegisterTask(nullptr);
  coordinator_actor_ = coordinator_actor;
  ns_per_cycle_ = 1e9 / tsc_hz;
}

struct task_result send_reliable_ack::RunTask(void *arg){
  struct task_result ret;
  ret = (struct task_result){
      .packets = 0, .bits = 0,
  };

  bess::PacketBatch batch;
  batch.clear();
  uint16_t out_gates[bess::PacketBatch::kMaxBurst];

  uint64_t current_ns = rdtsc()*ns_per_cycle_;

  for(int i=0; i<coordinator_actor_->reliables_.cnt(); i++){
    reliable_p2p* r = coordinator_actor_->reliables_.next();
    r->check(current_ns);

    bess::Packet* ack_pkt = r->get_ack_pkt();
    if(unlikely(ack_pkt == nullptr)){
      continue;
    }

    out_gates[batch.cnt()] = r->get_output_gate();
    batch.add(ack_pkt);
  }

  if(batch.cnt()>0){
    RunSplit(out_gates, &batch);
  }

  return ret;
}

ADD_MODULE(send_reliable_ack, "send_reliable_ack", "send out all the ack packets")

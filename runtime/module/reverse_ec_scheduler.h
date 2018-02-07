// This module is where we poll input/output port and schedule
// execution_context actors.

#ifndef REVERSEE_EC_SCHEDULER_H
#define REVERSE_EC_SCHEDULER_H

#include "../bessport/module.h"
#include "../reliable/base/reliable_message_misc.h"

class coordinator;

class reverse_ec_scheduler final : public Module {
public:

  static const gate_idx_t kNumOGates = 1;
  static const gate_idx_t kNumIGates = 1;

  reverse_ec_scheduler() : Module(), coordinator_actor_(0){}

  virtual void ProcessBatch(bess::PacketBatch *batch);

  void customized_init(coordinator* coordinator_actor);

private:

  coordinator* coordinator_actor_;
  bess::PacketBatch dp_pkt_batch;
  bess::PacketBatch cp_pkt_batch;
};

#endif

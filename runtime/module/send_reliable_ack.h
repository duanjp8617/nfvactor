// This module handles reliable message passing for remote communication.
#ifndef SENDD_RELIABLE_ACK_H
#define SENDD_RELIABLE_ACK_H

#include "../bessport/module.h"
#include "../actor/coordinator.h"
#include "../rpc/ring_msg.h"

class send_reliable_ack final : public Module{

public:
  static const gate_idx_t kNumOGates = 3;
  static const gate_idx_t kNumIGates = 0;

  send_reliable_ack() : Module(), coordinator_actor_(0){}

  virtual struct task_result RunTask(void *arg);

  void customized_init(coordinator* coordinator_actor);

private:

  coordinator* coordinator_actor_;

  double ns_per_cycle_;
};

#endif

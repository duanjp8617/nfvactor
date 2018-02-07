// This module handles reliable message passing for remote communication.
#ifndef SENDD_RELIABLE_MSG_H
#define SENDD_RELIABLE_MSG_H

#include "../bessport/module.h"
#include "../actor/coordinator.h"
#include "../rpc/ring_msg.h"

class send_reliable_msg final : public Module{

public:
  static const gate_idx_t kNumOGates = 3;
  static const gate_idx_t kNumIGates = 0;

  send_reliable_msg() : Module(), coordinator_actor_(0){}

  virtual struct task_result RunTask(void *arg);

  void customized_init(coordinator* coordinator_actor);

private:

  coordinator* coordinator_actor_;
};

#endif

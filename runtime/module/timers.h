// This module handles the flow timers and actor event timers.
#ifndef TIMERS_H
#define TIMERS_H

#include "../bessport/module.h"
#include "../actor/coordinator.h"

class timers final : public Module {
 public:
  static const gate_idx_t kNumIGates = 0;

  virtual struct task_result RunTask(void *arg);

  void customized_init(coordinator* coordinator_actor);

 private:
  coordinator* coordinator_actor_;
};

#endif  // BESS_MODULES_SINK_H_

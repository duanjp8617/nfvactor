// This module handles reliable message passing for remote communication.
#ifndef COORDINATOR_MP_H
#define COORDINATOR_MP_H

#include "../bessport/module.h"
#include "../actor/coordinator.h"

class coordinator_mp final : public Module{

public:
  static const gate_idx_t kNumOGates = 1;
  static const gate_idx_t kNumIGates = 0;

  coordinator_mp() : Module(), coordinator_actor_(0), num_to_send(0){}

  virtual struct task_result RunTask(void *arg);

  void customized_init(coordinator* coordinator_actor);

private:

  coordinator* coordinator_actor_;

  int num_to_send;

  int successful_send = 0;

  int unsuccessful_send= 0;

  bool send_end_flag = false;

  uint64_t start_time = 0;
  uint64_t end_time = 0;

  uint64_t current_iteration = 0;

  uint64_t local_replication_iteration = 0;
};

#endif

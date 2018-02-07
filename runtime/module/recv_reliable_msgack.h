// This module handles reliable message passing for remote communication.
#ifndef RECV_RELIABLE_MSGACK_H
#define RECV_RELIABLE_MSGACK_H

#include "../bessport/module.h"
#include "../reliable/base/reliable_message_misc.h"

class coordinator;

class recv_reliable_msgack final : public Module{

public:
  static const gate_idx_t kNumOGates = 2;
  static const gate_idx_t kNumIGates = 1;

  recv_reliable_msgack() : Module(), coordinator_actor_(0){}

  virtual void ProcessBatch(bess::PacketBatch *batch);

  void customized_init(coordinator* coordinator_actor);

private:
  coordinator* coordinator_actor_;
};

#endif

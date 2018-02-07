#ifndef SINK_H_
#define SINK_H_

#include "../bessport/module.h"

class Sink final : public Module {
 public:
  static const gate_idx_t kNumOGates = 0;

  void ProcessBatch(bess::PacketBatch *batch) override;

  void customized_init();
};

#endif  // BESS_MODULES_SINK_H_

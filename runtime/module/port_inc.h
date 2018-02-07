// Port the BESS port_inc module here.
#ifndef PORT_INC_H
#define PORT_INC_H

#include <string>

#include "../port/sn_port.h"
#include "../bessport/module.h"
#include "../bessport/module_msg.pb.h"
#include "../bessport/message.h"

using std::string;

class PortInc final : public Module {
public:
  static const gate_idx_t kNumIGates = 0;

  PortInc() : Module(), port_(), prefetch_(), burst_() {}

  // Fake init function.
  pb_error_t Init(const bess::pb::PortIncArg &arg);

  virtual struct task_result RunTask(void *arg);

  void customized_init(sn_port* port, int prefetch, int burst);

private:
  sn_port* port_;

  int prefetch_;

  int burst_;
};

#endif

// Port the BESS port_out module here.
#ifndef PORT_OUT_H
#define PORT_OUT_H

#include <string>

#include "../port/sn_port.h"
#include "../bessport/module.h"
#include "../bessport/module_msg.pb.h"
#include "../bessport/message.h"

using std::string;

class PortOut final : public Module {
public:
  static const gate_idx_t kNumOGates = 0;

  PortOut() : Module(), port_() {}

  // Fake init function.
  pb_error_t Init(const bess::pb::PortIncArg &arg);

  virtual void ProcessBatch(bess::PacketBatch *batch);

  void customized_init(sn_port* port);

private:
  sn_port* port_;
};

#endif

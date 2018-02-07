#include "port_out.h"
#include "../bessport/message.h"

#include <glog/logging.h>

void PortOut::ProcessBatch(bess::PacketBatch *batch) {
  /* TODO: choose appropriate out queue */
  const uint8_t qid = 0;

  uint64_t sent_bytes = 0;
  int sent_pkts;

  sent_pkts = port_->SendPackets(qid, batch->pkts(), batch->cnt());

  const packet_dir_t dir = PACKET_DIR_OUT;

  for (int i = 0; i < sent_pkts; i++){
    sent_bytes += batch->pkts()[i]->total_len();
  }

    port_->queue_stats[dir][qid].packets += sent_pkts;
    port_->queue_stats[dir][qid].dropped += (batch->cnt() - sent_pkts);
    port_->queue_stats[dir][qid].bytes += sent_bytes;


  if (sent_pkts < batch->cnt()) {
    bess::Packet::Free(batch->pkts() + sent_pkts, batch->cnt() - sent_pkts);
  }
}

pb_error_t PortOut::Init(const bess::pb::PortIncArg &arg){
  return pb_errno(0);
}

void PortOut::customized_init(sn_port* port){
  port_ = port;
}

ADD_MODULE(PortOut, "port_out", "sends pakets to a port")

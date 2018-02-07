#include "sink.h"

void Sink::ProcessBatch(bess::PacketBatch *batch) {
  bess::Packet::Free(batch);
}

void Sink::customized_init(){
}

ADD_MODULE(Sink, "sink", "discards all packets")

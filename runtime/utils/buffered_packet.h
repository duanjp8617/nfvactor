#ifndef BUFFERED_PACKET_H
#define BUFFERED_PACKET_H

#include "../bessport/packet.h"
#include "cdlist.h"

struct buffered_packet{
  cdlist_item list_item;
  bess::Packet* packet;
};

#endif

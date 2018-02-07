#ifndef SN_PORT_H
#define SN_PORT_H

#include <stdint.h>

#include "../bessport/kmod/sn_common.h"
#include "../bessport/kmod/llring.h"
#include "../bessport/packet.h"

/* The term RX/TX could be very confusing for a virtual switch.
 * Instead, we use the "incoming/outgoing" convention:
 * - incoming: outside -> BESS
 * - outgoing: BESS -> outside */
typedef enum {
  PACKET_DIR_INC = 0,
  PACKET_DIR_OUT = 1,
  PACKET_DIRS
} packet_dir_t;

struct packet_stats {
  packet_stats() : packets(), dropped(), bytes() {}

  uint64_t packets;
  uint64_t dropped; /* Not all drivers support this for inc dir */
  uint64_t bytes;   /* doesn't include Ethernet overhead */
};

typedef struct packet_stats port_stats_t[PACKET_DIRS];

/* Ideally share this with vport driver */

#define PORT_NAME_LEN 128
#define PORT_FNAME_LEN 128 + 256

#define MAX_QUEUES_PER_PORT_DIR 32

#define VPORT_DIR_PREFIX "sn_vports"

#ifndef __cacheline_aligned
#define __cacheline_aligned __attribute__((aligned(64)))
#endif

struct vport_inc_regs {
  uint64_t dropped;
} __cacheline_aligned;

struct vport_out_regs {
  volatile uint32_t irq_enabled;
} __cacheline_aligned;

/* This is equivalent to the old bar */
struct vport_bar {
  char name[PORT_NAME_LEN];

  /* The term RX/TX could be very confusing for a virtual switch.
   * Instead, we use the "incoming/outgoing" convention:
   * - incoming: outside -> BESS
   * - outgoing: BESS -> outside */
  int num_inc_q;
  int num_out_q;

  struct vport_inc_regs *inc_regs[MAX_QUEUES_PER_PORT_DIR];
  struct llring *inc_qs[MAX_QUEUES_PER_PORT_DIR];

  struct vport_out_regs *out_regs[MAX_QUEUES_PER_PORT_DIR];
  struct llring *out_qs[MAX_QUEUES_PER_PORT_DIR];
};

class sn_port {

public:
  sn_port() :
    bar(0),
    num_txq(0),
    num_rxq(0){}

  ~sn_port();

  bool init_port(const char* ifname);

  inline int RecvPackets(uint8_t qid, bess::Packet **pkts, int cnt){
    return llring_dequeue_burst(this->rx_qs[qid], (void **)pkts, cnt);
  }

  inline int SendPackets(uint8_t qid, bess::Packet **pkts, int cnt){
    int sent;

    sent = llring_enqueue_burst(this->tx_qs[qid], (void **)pkts, cnt) &
           (~RING_QUOT_EXCEED);

    this->tx_regs[qid]->dropped += (cnt - sent);

    return sent;
  }

  struct packet_stats queue_stats[PACKET_DIRS][MAX_QUEUES_PER_PORT_DIR];

private:
  struct vport_bar *bar;

  int num_txq;

  int num_rxq;

  struct vport_inc_regs *tx_regs[MAX_QUEUES_PER_PORT_DIR];

  struct llring *tx_qs[MAX_QUEUES_PER_PORT_DIR];

  struct vport_out_regs *rx_regs[MAX_QUEUES_PER_PORT_DIR];

  struct llring *rx_qs[MAX_QUEUES_PER_PORT_DIR];

  int fd[MAX_QUEUES_PER_PORT_DIR];
};

#endif

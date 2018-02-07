#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <fcntl.h>

#include "sn_port.h"

sn_port::~sn_port(){
  for (int i = 0; i < this->num_rxq; i++) {
    if(this->fd[i]>0){
      close(this->fd[i]);
    }
  }
}

bool sn_port::init_port(const char *ifname)
{
  struct vport_bar *bar;
  int i;

  FILE* fd;
  char port_file[PORT_FNAME_LEN];

  snprintf(port_file, PORT_FNAME_LEN, "%s/%s/%s",
      P_tmpdir, VPORT_DIR_PREFIX, ifname);
  fd = fopen(port_file, "r");
  if (!fd) {
    return false;
  }
  i = fread(&bar, 8, 1, fd);
  fclose(fd);
  if (i != 1) {
    return false;
  }

  if (!bar)
    return false;

  this->bar = bar;

  this->num_txq = bar->num_inc_q;
  this->num_rxq = bar->num_out_q;

  for (i = 0; i < this->num_rxq; i++) {
    this->rx_regs[i] = bar->out_regs[i];
    this->rx_qs[i] = bar->out_qs[i];
  }

  for (i = 0; i < this->num_txq; i++) {
    this->tx_regs[i] = bar->inc_regs[i];
    this->tx_qs[i] = bar->inc_qs[i];
  }

  for (i = 0; i < this->num_rxq; i++) {
    char fifoname[256];

    sprintf(fifoname, "%s/%s/%s.rx%d",
        P_tmpdir, VPORT_DIR_PREFIX, ifname, i);

    this->fd[i] = open(fifoname, O_RDONLY);
    assert(this->fd[i] > 0);
  }

  return true;
}

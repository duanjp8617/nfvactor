#ifndef LLRING_HOLDER_H
#define LLRING_HOLDER_H

#include <rte_malloc.h>

#include "../bessport/kmod/llring.h"

class llring_holder{

public:
  static const int llring_size = 1024;

  llring_holder(){
    int bytes_per_llring = llring_bytes_with_slots(llring_size);
    rpc2worker_ring_ = static_cast<struct llring*>(rte_zmalloc(nullptr, bytes_per_llring, 0));
    worker2rpc_ring_ = static_cast<struct llring*>(rte_zmalloc(nullptr, bytes_per_llring, 0));
    llring_init(rpc2worker_ring_, llring_size, 0, 0);
    llring_set_water_mark(rpc2worker_ring_, ((llring_size >> 3) * 7));
    llring_init(worker2rpc_ring_, llring_size, 0, 0);
    llring_set_water_mark(worker2rpc_ring_, ((llring_size >> 3) * 7));
  }

  ~llring_holder(){
    rte_free(rpc2worker_ring_);
    rte_free(worker2rpc_ring_);
  }

  struct llring* rpc2worker_ring(){
    return rpc2worker_ring_;
  }

  struct llring* worker2rpc_ring(){
    return worker2rpc_ring_;
  }

private:
  struct llring* rpc2worker_ring_;
  struct llring* worker2rpc_ring_;
};

#endif

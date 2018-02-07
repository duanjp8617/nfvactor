#ifndef SERVER_IMPL_H
#define SERVER_IMPL_H

#include <memory>
#include <iostream>
#include <string>
#include <thread>
#include <set>
#include <atomic>
#include <unordered_map>

#include <grpc++/grpc++.h>
#include <grpc/support/log.h>

#include <rte_config.h>
#include <rte_lcore.h>
#include <rte_malloc.h>

#include "../bessport/kmod/llring.h"
#include "../bessport/nfa_msg.grpc.pb.h"
#include "../nfaflags.h"
#include "../bessport/utils/common.h"
#include "ring_msg.h"
#include "../nfaflags.h"

using std::string;
using std::set;
using std::unordered_map;

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerCompletionQueue;
using grpc::Status;
using nfa_msg::Runtime_RPC;

class ServerImpl final {
 public:
  ServerImpl(struct llring* rpc2worker_ring,
             struct llring* worker2rpc_ring) :
               rpc2worker_ring_(rpc2worker_ring),
               worker2rpc_ring_(worker2rpc_ring){
    local_runtime_.runtime_id = FLAGS_runtime_id;
    local_runtime_.input_port_mac = convert_string_mac(FLAGS_input_port_mac);
    local_runtime_.output_port_mac = convert_string_mac(FLAGS_output_port_mac);
    local_runtime_.control_port_mac = convert_string_mac(FLAGS_control_port_mac);
    local_runtime_.rpc_ip = convert_string_ip(FLAGS_rpc_ip);
    local_runtime_.rpc_port = FLAGS_rpc_port;
  }

  ~ServerImpl() {
    if(server_!=nullptr){
      server_->Shutdown();
    }
    if(cq_!=nullptr){
      cq_->Shutdown();
    }
  }

  bool Run(string rpc_ip, int32_t rpc_port);

  void HandleRpcs(set<int> cpu_set, int lcore_id, std::atomic<bool>& rpc_server_thread_ready);

  void HandleRpcs();

 private:

  template<class... T>
  void create_call_data(T&&... arg);

  std::unique_ptr<ServerCompletionQueue> cq_;

  Runtime_RPC::AsyncService service_;

  std::unique_ptr<Server> server_;

  struct llring* rpc2worker_ring_;

  struct llring* worker2rpc_ring_;

  unordered_map<string, runtime_config> input_runtimes_;

  unordered_map<string, runtime_config> output_runtimes_;

  unordered_map<string, runtime_config> replicas_;

  unordered_map<string, runtime_config> storages_;

  unordered_map<string, runtime_config> migration_targets_;

  unordered_map<string, runtime_config> migration_sources_;

  runtime_config local_runtime_;
};

#endif

#ifndef CALL_DATA_BASE
#define CALL_DATA_BASE

#include <grpc++/grpc++.h>
#include <grpc/support/log.h>

#include "../bessport/nfa_msg.grpc.pb.h"

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerCompletionQueue;
using grpc::Status;
using grpc::Channel;
using grpc::ClientContext;
using nfa_msg::Runtime_RPC;

class call_data_base {
public:
  explicit call_data_base(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq)
    : service_(service), cq_(cq), status_(CREATE) {
  }

  virtual void Proceed() = 0;

  virtual ~call_data_base() {}

protected:

  Runtime_RPC::AsyncService* service_;

  ServerCompletionQueue* cq_;

  ServerContext ctx_;

  enum CallStatus { CREATE, PROCESS, FINISH };
  CallStatus status_;
};

#endif

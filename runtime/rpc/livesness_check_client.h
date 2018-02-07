#include <iostream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>
#include <glog/logging.h>

#include "../bessport/nfa_msg.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using nfa_msg::LivenessRequest;
using nfa_msg::LivenessReply;
using nfa_msg::Runtime_RPC;

using namespace nfa_msg;

class LivenessCheckClient {
 public:
  LivenessCheckClient(std::shared_ptr<Channel> channel)
      : stub_(Runtime_RPC::NewStub(channel)) {}

  std::string Check() {

    LivenessRequest request;

    LivenessReply reply;

    ClientContext context;

    // The actual RPC.
    Status status = stub_->LivenessCheck(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return "LivenessCheck succeed";
    } else {
      return "LivenessCheck fail";
    }
  }

  std::string AddOutputRt(){
    AddOutputRtsReq request;
    AddOutputRtsRes reply;
    ClientContext context;

    auto new_addr_ptr = request.add_addrs();
    new_addr_ptr->set_rpc_ip("202.45.128.155");
    new_addr_ptr->set_rpc_port(10241);

    new_addr_ptr = request.add_addrs();
    new_addr_ptr->set_rpc_ip("202.45.128.156");
    new_addr_ptr->set_rpc_port(10242);

    Status status = stub_->AddOutputRts(&context, request, &reply);

    if(status.ok()){
      return "AddOutputRt finishes.";
    }
    else{
      return "AddOutputRt fails.";
    }
  }

  std::string SingleAddOutputRt(std::string ip,int32_t port_num){
    AddOutputRtsReq request;
    AddOutputRtsRes reply;
    ClientContext context;

    auto new_addr_ptr = request.add_addrs();
    new_addr_ptr->set_rpc_ip(ip);
    new_addr_ptr->set_rpc_port(port_num);

    Status status = stub_->AddOutputRts(&context, request, &reply);

    if(status.ok()){
      return "AddOutputRt finishes.";
    }
    else{
      return "AddOutputRt fails.";
    }
  }

  std::string DeleteOutputRt(std::string ip,int32_t port_num){
    DeleteOutputRtReq request;
    DeleteOutputRtRep reply;
    ClientContext context;

    request.mutable_addrs()->set_rpc_ip(ip);
    request.mutable_addrs()->set_rpc_port(port_num);

    Status status = stub_->DeleteOutputRt(&context, request, &reply);

    if(status.ok()){
      return "DeleteOutputRt finishes.";
    }
    else{
      return "DeleteOutputRt fails.";
    }
  }

  std::string DeleteInputRt(std::string ip,int32_t port_num){
    DeleteInputRtReq request;
    DeleteInputRtRep reply;
    ClientContext context;

    request.mutable_addrs()->set_rpc_ip(ip);
    request.mutable_addrs()->set_rpc_port(port_num);

    Status status = stub_->DeleteInputRt(&context, request, &reply);

    if(status.ok()){
      return "DeleteInputRt finishes.";
    }
    else{
      return "DeleteInputRt fails.";
    }
  }

  std::string AddOutputMac(std::string ip,int32_t port_num){
    AddOutputMacReq request;
    AddOutputMacRep reply;
    ClientContext context;

    request.mutable_addrs()->set_rpc_ip(ip);
    request.mutable_addrs()->set_rpc_port(port_num);

    Status status = stub_->AddOutputMac(&context, request, &reply);

    if(status.ok()){
      return "AddOutputMac finishes.";
    }
    else{
      return "AddOutputMac fails.";
    }
  }

  std::string AddInputMac(std::string ip,int32_t port_num){
    AddInputMacReq request;
    AddInputMacRep reply;
    ClientContext context;

    request.mutable_addrs()->set_rpc_ip(ip);
    request.mutable_addrs()->set_rpc_port(port_num);

    Status status = stub_->AddInputMac(&context, request, &reply);

    if(status.ok()){
      return "AddInputMac finishes.";
    }
    else{
      return "AddInputMac fails.";
    }
  }

  std::string DeleteOutputMac(std::string ip,int32_t port_num){
    DeleteOutputMacReq request;
    DeleteOutputMacRep reply;
    ClientContext context;

    request.mutable_addrs()->set_rpc_ip(ip);
    request.mutable_addrs()->set_rpc_port(port_num);

    Status status = stub_->DeleteOutputMac(&context, request, &reply);

    if(status.ok()){
      return "DeleteOutputMac finishes.";
    }
    else{
      return "DeleteOutputMac fails.";
    }
  }

  std::string DeleteInputMac(std::string ip,int32_t port_num){
    DeleteInputMacReq request;
    DeleteInputMacRep reply;
    ClientContext context;

    request.mutable_addrs()->set_rpc_ip(ip);
    request.mutable_addrs()->set_rpc_port(port_num);

    Status status = stub_->DeleteInputMac(&context, request, &reply);

    if(status.ok()){
      return "DeleteInputMac finishes.";
    }
    else{
      return "DeleteInputMac fails.";
    }
  }

  std::string MigrateTo(std::string ip,int32_t port_num, int32_t quota){
    MigrateToReq request;
    MigrateToRep reply;
    ClientContext context;

    request.mutable_addr()->set_rpc_ip(ip);
    request.mutable_addr()->set_rpc_port(port_num);
    request.set_quota(quota);

    Status status = stub_->MigrateTo(&context, request, &reply);

    if(status.ok()){
      return "MigrateTo finishes.";
    }
    else{
      return "MigrateTo fails.";
    }
  }

  std::string SetMigrationTarget(std::string ip,int32_t port_num, int32_t qouta){
    SetMigrationTargetReq request;
    SetMigrationTargetRep reply;
    ClientContext context;

    auto new_addr_ptr = request.add_addrs();
    new_addr_ptr->set_rpc_ip(ip);
    new_addr_ptr->set_rpc_port(port_num);

    Status status = stub_->SetMigrationTarget(&context, request, &reply);

    if(status.ok()){
      return "SetMigrationTarget finishes.";
    }
    else{
      return "SetMigrationTarget fails.";
    }
  }

  std::string AddReplicas(std::string ip,int32_t port_num){
    AddReplicasReq request;
    AddReplicasRep reply;
    ClientContext context;

    auto new_addr_ptr = request.add_addrs();
    new_addr_ptr->set_rpc_ip(ip);
    new_addr_ptr->set_rpc_port(port_num);

    Status status = stub_->AddReplicas(&context, request, &reply);

    if(status.ok()){
      return "AddReplicas finishes.";
    }
    else{
      return "AddReplicas fails.";
    }
  }

  std::string DeleteReplica(std::string ip,int32_t port_num){
    DeleteReplicaReq request;
    DeleteReplicaRep reply;
    ClientContext context;

    request.mutable_addrs()->set_rpc_ip(ip);
    request.mutable_addrs()->set_rpc_port(port_num);

    Status status = stub_->DeleteReplica(&context, request, &reply);

    if(status.ok()){
      return "DeleteReplica finishes.";
    }
    else{
      return "DeleteReplica fails.";
    }
  }

  std::string DeleteStorage(std::string ip,int32_t port_num){
    DeleteStorageReq request;
    DeleteStorageRep reply;
    ClientContext context;

    request.mutable_addrs()->set_rpc_ip(ip);
    request.mutable_addrs()->set_rpc_port(port_num);

    Status status = stub_->DeleteStorage(&context, request, &reply);

    if(status.ok()){
      return "DeleteStorage finishes.";
    }
    else{
      return "DeleteStorage fails.";
    }
  }

  std::string Recover(std::string ip,int32_t port_num){
    RecoverReq request;
    RecoverRep reply;
    ClientContext context;

    request.mutable_addr()->set_rpc_ip(ip);
    request.mutable_addr()->set_rpc_port(port_num);

    Status status = stub_->Recover(&context, request, &reply);

    if(status.ok()){
      return "Recover finishes.";
    }
    else{
      return "Recover fails.";
    }
  }

  std::string GetRuntimeState(){
    GetRuntimeStateReq request;
    GetRuntimeStateRep reply;
    ClientContext context;

    Status status = stub_->GetRuntimeState(&context, request, &reply);

    LOG(INFO)<<"The rpc ip of the runtime is "<<reply.local_runtime().rpc_ip();
    LOG(INFO)<<"The rpc port of the runtime is "<<reply.local_runtime().rpc_port();

    if(status.ok()){
      return "GetRuntimeStateReq finishes.";
    }
    else{
      return "GetRuntimeStateReq fails.";
    }
  }

 private:
  std::unique_ptr<Runtime_RPC::Stub> stub_;
};

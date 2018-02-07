#ifndef CALL_DATA_IMPL
#define CALL_DATA_IMPL

#include <unordered_map>
#include <memory>
#include <thread>
#include <chrono>
#include <string>

#include <glog/logging.h>

#include "call_data_base.h"
#include "../bessport/kmod/llring.h"
#include "../nfaflags.h"
#include "ring_msg.h"

using std::string;
using std::unordered_map;

using namespace nfa_msg;

template<class TReq, class TRep>
class derived_call_data : public call_data_base{
public:
  derived_call_data(Runtime_RPC::AsyncService* service,
                    ServerCompletionQueue* cq,
                    struct llring* rpc2worker_ring,
                    struct llring* worker2rpc_ring,
                    unordered_map<string, runtime_config>& input_runtimes,
                    unordered_map<string, runtime_config>& output_runtimes,
                    unordered_map<string, runtime_config>& replicas,
                    unordered_map<string, runtime_config>& storages,
                    unordered_map<string, runtime_config>& migration_targets,
                    unordered_map<string, runtime_config>& migration_sources,
                    runtime_config& local_runtime)
    : call_data_base(service, cq),
      responder_(&ctx_),
      rpc2worker_ring_(rpc2worker_ring),
      worker2rpc_ring_(worker2rpc_ring),
      input_runtimes_(input_runtimes),
      output_runtimes_(output_runtimes),
      replicas_(replicas),
      storages_(storages),
      migration_targets_(migration_targets),
      migration_sources_(migration_sources),
      local_runtime_(local_runtime){
    Proceed();
  }

  void Proceed() override{}

  ~derived_call_data() = default;

private:
  void create_itself(){
    new derived_call_data<TReq, TRep>(service_,
                                      cq_,
                                      rpc2worker_ring_,
                                      worker2rpc_ring_,
                                      input_runtimes_,
                                      output_runtimes_,
                                      replicas_,
                                      storages_,
                                      migration_targets_,
                                      migration_sources_,
                                      local_runtime_);
  }

  void* poll_worker2rpc_ring(){
    int aggressive_poll_attemps = 50;
    int flag = 0;
    void* dequeue_output[1];

    for(int i=0; i<aggressive_poll_attemps; i++){
      flag = llring_sc_dequeue(worker2rpc_ring_, dequeue_output);

      if(flag != 0){
        continue;
      }
      else{
        return dequeue_output[0];
      }
    }

    for(;;){
      flag = llring_sc_dequeue(worker2rpc_ring_, dequeue_output);

      if(flag != 0){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
      else{
        return dequeue_output[0];
      }
    }
  }

  TReq request_;

  TRep reply_;

  ServerAsyncResponseWriter<TRep> responder_;

  struct llring* rpc2worker_ring_;

  struct llring* worker2rpc_ring_;

  unordered_map<string, runtime_config>& input_runtimes_;

  unordered_map<string, runtime_config>& output_runtimes_;

  unordered_map<string, runtime_config>& replicas_;

  unordered_map<string, runtime_config>& storages_;

  unordered_map<string, runtime_config>& migration_targets_;

  unordered_map<string, runtime_config>& migration_sources_;

  runtime_config& local_runtime_;

  inline string concat_with_colon(const string& s1, const string&s2){
    return s1+string(":")+s2;
  }
};

// The following code is the template for implementing the RPC call.
// template<>
// void derived_call_data<RPCRequestType, RPCResponseType>::Proceed(){
//   if (status_ == CREATE) {
//     status_ = PROCESS;
//     service_->RequestRPCCallName(&ctx_, &request_, &responder_, cq_, cq_, this);
//   } else if (status_ == PROCESS) {
//     create_itself();
//
    // Where the actual handling is done.

//     status_ = FINISH;
//     responder_.Finish(reply_, Status::OK, this);
//   } else {
//     GPR_ASSERT(status_ == FINISH);
//     delete this;
//   }
// }


// RPC implementation for LivenessCheck

template<>
void derived_call_data<LivenessRequest, LivenessReply>::Proceed(){
  if (status_ == CREATE) {
    status_ = PROCESS;
    service_->RequestLivenessCheck(&ctx_, &request_, &responder_, cq_, cq_, this);
  } else if (status_ == PROCESS) {
    create_itself();
    LOG(INFO) <<"Get a LivenessCheck call.";

    status_ = FINISH;
    responder_.Finish(reply_, Status::OK, this);
  } else {
    GPR_ASSERT(status_ == FINISH);
    delete this;
  }
}

// RPC implementation for AddOutputRts

template<>
void derived_call_data<AddOutputRtsReq, AddOutputRtsRes>::Proceed(){
  if (status_ == CREATE) {
    status_ = PROCESS;
    service_->RequestAddOutputRts(&ctx_, &request_, &responder_, cq_, cq_, this);
  } else if (status_ == PROCESS) {
    create_itself();

    LOG(INFO)<<"Receive AddOutputRts RPC";

    RuntimeConfig protobuf_local_runtime =  local2protobuf(local_runtime_);
    string local_addr = concat_with_colon(protobuf_local_runtime.rpc_ip(),
                                          std::to_string(protobuf_local_runtime.rpc_port()));
    for(int i=0; i<request_.addrs_size(); i++){
      string dest_addr = concat_with_colon(request_.addrs(i).rpc_ip(),
                                           std::to_string(request_.addrs(i).rpc_port()));
      if((output_runtimes_.find(dest_addr)!=output_runtimes_.end()) ||
         (input_runtimes_.find(dest_addr)!=input_runtimes_.end()) ||
         (dest_addr == local_addr) ||
         (replicas_.find(dest_addr)!=replicas_.end()) ||
         (storages_.find(dest_addr)!=storages_.end()) ||
         (migration_targets_.find(dest_addr)!=migration_targets_.end()) ||
         (migration_sources_.find(dest_addr)!=migration_sources_.end())
         ){
        continue;
      }

      std::unique_ptr<Runtime_RPC::Stub> stub(Runtime_RPC::NewStub(
          grpc::CreateChannel(dest_addr, grpc::InsecureChannelCredentials())));

      AddInputRtReq req;
      req.mutable_input_runtime()->CopyFrom(protobuf_local_runtime);
      AddInputRtRep rep;

      ClientContext context;
      std::chrono::system_clock::time_point deadline =
              std::chrono::system_clock::now() + std::chrono::seconds(FLAGS_rpc_timeout);
      context.set_deadline(deadline);

      Status status = stub->AddInputRt(&context, req, &rep);

      if(status.ok() && rep.has_local_runtime()){
        LOG(INFO)<<"AddInputRt recursive call finishes.";

        runtime_config output_runtime = protobuf2local(rep.local_runtime());

        output_runtimes_.emplace(dest_addr, output_runtime);

        llring_item item(rpc_operation::add_output_runtime, output_runtime, 0, 0);

        LOG(INFO)<<"In AddOutputRts, enqueue the item to the ring";
        llring_sp_enqueue(rpc2worker_ring_, static_cast<void*>(&item));

        LOG(INFO)<<"In AddOutputRts, dequeue the item from the ring";
        poll_worker2rpc_ring();
      }
    }

    status_ = FINISH;
    responder_.Finish(reply_, Status::OK, this);
  } else {
    GPR_ASSERT(status_ == FINISH);
    delete this;
  }
}

// RPC implementation for AddInputRt

template<>
void derived_call_data<AddInputRtReq, AddInputRtRep>::Proceed(){
  if (status_ == CREATE) {
    status_ = PROCESS;
    service_->RequestAddInputRt(&ctx_, &request_, &responder_, cq_, cq_, this);
  } else if (status_ == PROCESS) {
    create_itself();
    LOG(INFO)<<"Receive AddInputRt RPC";

    runtime_config input_runtime = protobuf2local(request_.input_runtime());
    string input_runtime_addr = concat_with_colon(request_.input_runtime().rpc_ip(),
                                                  std::to_string(request_.input_runtime().rpc_port()));
    if((input_runtime != local_runtime_) &&
       (input_runtimes_.find(input_runtime_addr)==input_runtimes_.end()) &&
       (output_runtimes_.find(input_runtime_addr)==output_runtimes_.end())){
      input_runtimes_.emplace(input_runtime_addr, input_runtime);

      llring_item item(rpc_operation::add_input_runtime, input_runtime, 0, 0);

      LOG(INFO)<<"In AddInputRt, enqueue the item to the ring";
      llring_sp_enqueue(rpc2worker_ring_, static_cast<void*>(&item));

      LOG(INFO)<<"In AddInputRt, dequeue the item from the ring";
      poll_worker2rpc_ring();

      RuntimeConfig protobuf_local_runtime =  local2protobuf(local_runtime_);

      reply_.mutable_local_runtime()->CopyFrom(protobuf_local_runtime);
    }

    status_ = FINISH;
    responder_.Finish(reply_, Status::OK, this);
  } else {
    GPR_ASSERT(status_ == FINISH);
    delete this;
  }
}

// RPC implementation for DeleteOutputRt

template<>
void derived_call_data<DeleteOutputRtReq, DeleteOutputRtRep>::Proceed(){
  if (status_ == CREATE) {
    status_ = PROCESS;
    service_->RequestDeleteOutputRt(&ctx_, &request_, &responder_, cq_, cq_, this);
  } else if (status_ == PROCESS) {
    create_itself();
    string output_runtime_addr = concat_with_colon(request_.addrs().rpc_ip(),
                                                   std::to_string(request_.addrs().rpc_port()));
    if((output_runtimes_.find(output_runtime_addr)!=output_runtimes_.end())){
      llring_item item(rpc_operation::delete_output_runtime, output_runtimes_[output_runtime_addr], 0, 0);
      output_runtimes_.erase(output_runtime_addr);

      llring_sp_enqueue(rpc2worker_ring_, static_cast<void*>(&item));

      poll_worker2rpc_ring();
    }

    status_ = FINISH;
    responder_.Finish(reply_, Status::OK, this);
  } else {
    GPR_ASSERT(status_ == FINISH);
    delete this;
  }
}

// RPC implementation for DeleteInputRt

template<>
void derived_call_data<DeleteInputRtReq, DeleteInputRtRep>::Proceed(){
  if (status_ == CREATE) {
    status_ = PROCESS;
    service_->RequestDeleteInputRt(&ctx_, &request_, &responder_, cq_, cq_, this);
  } else if (status_ == PROCESS) {
    create_itself();

    string input_runtime_addr = concat_with_colon(request_.addrs().rpc_ip(),
                                                   std::to_string(request_.addrs().rpc_port()));
    if((input_runtimes_.find(input_runtime_addr)!=input_runtimes_.end())){
      llring_item item(rpc_operation::delete_input_runtime, input_runtimes_[input_runtime_addr], 0, 0);
      input_runtimes_.erase(input_runtime_addr);

      llring_sp_enqueue(rpc2worker_ring_, static_cast<void*>(&item));

      poll_worker2rpc_ring();
    }

    status_ = FINISH;
    responder_.Finish(reply_, Status::OK, this);
  } else {
    GPR_ASSERT(status_ == FINISH);
    delete this;
  }
}

// RPC implementation for AddInputMac

template<>
void derived_call_data<AddInputMacReq, AddInputMacRep>::Proceed(){
  if (status_ == CREATE) {
    status_ = PROCESS;
    service_->RequestAddInputMac(&ctx_, &request_, &responder_, cq_, cq_, this);
  } else if (status_ == PROCESS) {
    create_itself();

    string input_runtime_addr = concat_with_colon(request_.addrs().rpc_ip(),
                                                  std::to_string(request_.addrs().rpc_port()));
    auto runtime_to_find=input_runtimes_.find(input_runtime_addr);

    if(runtime_to_find!=input_runtimes_.end()){

      llring_item item(rpc_operation::add_input_mac, runtime_to_find->second, 0, 0);

      llring_sp_enqueue(rpc2worker_ring_, static_cast<void*>(&item));

      poll_worker2rpc_ring();
    }

    status_ = FINISH;
    responder_.Finish(reply_, Status::OK, this);
  } else {
    GPR_ASSERT(status_ == FINISH);
    delete this;
  }
}

// RPC implementation for AddOutputMac

template<>
void derived_call_data<AddOutputMacReq, AddOutputMacRep>::Proceed(){
  if (status_ == CREATE) {
    status_ = PROCESS;
    service_->RequestAddOutputMac(&ctx_, &request_, &responder_, cq_, cq_, this);
  } else if (status_ == PROCESS) {
    create_itself();

    string output_runtime_addr = concat_with_colon(request_.addrs().rpc_ip(),
                                                  std::to_string(request_.addrs().rpc_port()));
    auto runtime_to_find=output_runtimes_.find(output_runtime_addr);
    if(runtime_to_find!=output_runtimes_.end()){

      llring_item item(rpc_operation::add_output_mac, runtime_to_find->second, 0, 0);

      llring_sp_enqueue(rpc2worker_ring_, static_cast<void*>(&item));

      poll_worker2rpc_ring();
    }

    status_ = FINISH;
    responder_.Finish(reply_, Status::OK, this);
  } else {
    GPR_ASSERT(status_ == FINISH);
    delete this;
  }
}

// RPC implementation for DeleteInputMac

template<>
void derived_call_data<DeleteInputMacReq, DeleteInputMacRep>::Proceed(){
  if (status_ == CREATE) {
    status_ = PROCESS;
    service_->RequestDeleteInputMac(&ctx_, &request_, &responder_, cq_, cq_, this);
  } else if (status_ == PROCESS) {
    create_itself();

    string input_runtime_addr = concat_with_colon(request_.addrs().rpc_ip(),
                                                  std::to_string(request_.addrs().rpc_port()));
    auto runtime_to_find=input_runtimes_.find(input_runtime_addr);
    if(runtime_to_find!=input_runtimes_.end()){

      llring_item item(rpc_operation::delete_input_mac, runtime_to_find->second, 0, 0);

      llring_sp_enqueue(rpc2worker_ring_, static_cast<void*>(&item));

      poll_worker2rpc_ring();
    }

    status_ = FINISH;
    responder_.Finish(reply_, Status::OK, this);
  } else {
    GPR_ASSERT(status_ == FINISH);
    delete this;
  }
}

// RPC implementation for DeleteOutputMac

template<>
void derived_call_data<DeleteOutputMacReq, DeleteOutputMacRep>::Proceed(){
  if (status_ == CREATE) {
    status_ = PROCESS;
    service_->RequestDeleteOutputMac(&ctx_, &request_, &responder_, cq_, cq_, this);
  } else if (status_ == PROCESS) {
    create_itself();

    string output_runtime_addr = concat_with_colon(request_.addrs().rpc_ip(),
                                                  std::to_string(request_.addrs().rpc_port()));
    auto runtime_to_find=output_runtimes_.find(output_runtime_addr);
    if(runtime_to_find!=output_runtimes_.end()){

      llring_item item(rpc_operation::delete_output_mac, runtime_to_find->second, 0, 0);

      llring_sp_enqueue(rpc2worker_ring_, static_cast<void*>(&item));

      poll_worker2rpc_ring();
    }

    status_ = FINISH;
    responder_.Finish(reply_, Status::OK, this);
  } else {
    GPR_ASSERT(status_ == FINISH);
    delete this;
  }
}

//RPC implementation for MigrateTo
template<>
void derived_call_data<MigrateToReq, MigrateToRep>::Proceed(){
  if (status_ == CREATE) {
    status_ = PROCESS;
    service_->RequestMigrateTo(&ctx_, &request_, &responder_, cq_, cq_, this);
  } else if (status_ == PROCESS) {
    create_itself();
    string dest_addr = concat_with_colon(request_.addr().rpc_ip(),
                                         std::to_string(request_.addr().rpc_port()));

    if(migration_targets_.find(dest_addr)!=migration_targets_.end()){
      runtime_config& migration_target_runtime = migration_targets_.find(dest_addr)->second;

      llring_item tmp_item(rpc_operation::migrate_to, migration_target_runtime, request_.quota(), 0);

      llring_sp_enqueue(rpc2worker_ring_, static_cast<void*>(&tmp_item));

      poll_worker2rpc_ring();
    }


    status_ = FINISH;
    responder_.Finish(reply_, Status::OK, this);
  } else {
    GPR_ASSERT(status_ == FINISH);
    delete this;
  }
}

//RPC implementation for SetMigrationTarget

template<>
void derived_call_data<SetMigrationTargetReq, SetMigrationTargetRep>::Proceed(){
  if (status_ == CREATE) {
    status_ = PROCESS;
    service_->RequestSetMigrationTarget(&ctx_, &request_, &responder_, cq_, cq_, this);
  } else if (status_ == PROCESS) {
    create_itself();
    string local_addr = concat_with_colon(convert_uint32t_ip(local_runtime_.rpc_ip),
                                             std::to_string(local_runtime_.rpc_port));

    for(int i=0; i<request_.addrs_size(); i++){
      string dest_addr = concat_with_colon(request_.addrs(i).rpc_ip(),
                                           std::to_string(request_.addrs(i).rpc_port()));
      if( (dest_addr==local_addr) ||
          (migration_targets_.find(dest_addr) != migration_targets_.end()) ||
          (replicas_.find(dest_addr) != replicas_.end()) ||
          (storages_.find(dest_addr) != storages_.end()) ||
          (input_runtimes_.find(dest_addr) != input_runtimes_.end()) ||
          (output_runtimes_.find(dest_addr) != output_runtimes_.end()) ){
        status_ = FINISH;
        responder_.Finish(reply_, Status::OK, this);
        return;
      }

      std::unique_ptr<Runtime_RPC::Stub> stub(Runtime_RPC::NewStub(
          grpc::CreateChannel(dest_addr, grpc::InsecureChannelCredentials())));

      MigrationNegotiateReq req;
      for(auto it=input_runtimes_.begin();it!=input_runtimes_.end();it++){
        auto addr_ptr = req.add_input_runtime_addrs();
        addr_ptr->set_rpc_ip(convert_uint32t_ip(it->second.rpc_ip));
        addr_ptr->set_rpc_port(it->second.rpc_port);

      }
      for(auto it=output_runtimes_.begin();it!=output_runtimes_.end();it++){
        auto addr_ptr = req.add_output_runtime_addrs();
        addr_ptr->set_rpc_ip(convert_uint32t_ip(it->second.rpc_ip));
        addr_ptr->set_rpc_port(it->second.rpc_port);

      }
      req.mutable_migration_source_config()->CopyFrom(local2protobuf(local_runtime_));
      MigrationNegotiateRep rep;

      ClientContext context;
      std::chrono::system_clock::time_point deadline =
              std::chrono::system_clock::now() + std::chrono::seconds(FLAGS_rpc_timeout);
      context.set_deadline(deadline);

      Status status = stub->MigrationNegotiate(&context, req, &rep);

      if(status.ok() && rep.has_migration_target_runtime()){

        runtime_config migration_target_runtime = protobuf2local(rep.migration_target_runtime());

        if(migration_targets_.find(dest_addr)==migration_targets_.end()){
          migration_targets_.emplace(dest_addr, migration_target_runtime);
        }

        llring_item item(rpc_operation::set_migration_target, migration_target_runtime, 0, 0);

        llring_sp_enqueue(rpc2worker_ring_, static_cast<void*>(&item));

        poll_worker2rpc_ring();
      }
    }


    status_ = FINISH;
    responder_.Finish(reply_, Status::OK, this);
  } else {
    GPR_ASSERT(status_ == FINISH);
    delete this;
  }
}

// RPC implementation for MigrationNegotiation

template<>
void derived_call_data<MigrationNegotiateReq, MigrationNegotiateRep>::Proceed(){
  if (status_ == CREATE) {
    status_ = PROCESS;
    service_->RequestMigrationNegotiate(&ctx_, &request_, &responder_, cq_, cq_, this);
  } else if (status_ == PROCESS) {
    create_itself();

    if( (input_runtimes_.size()!=request_.input_runtime_addrs_size()) ||
        (output_runtimes_.size()!=request_.output_runtime_addrs_size()) ){
      status_ = FINISH;
      responder_.Finish(reply_, Status::OK, this);
      return;
    }

    for(auto i=0; i<request_.input_runtime_addrs_size(); i++){
      string compare_addr = concat_with_colon(request_.input_runtime_addrs(i).rpc_ip(),
                                              std::to_string(request_.input_runtime_addrs(i).rpc_port()));
      if(input_runtimes_.find(compare_addr)==input_runtimes_.end()){
        status_ = FINISH;
        responder_.Finish(reply_, Status::OK, this);
        return;
      }
    }

    for(auto i=0; i<request_.output_runtime_addrs_size(); i++){
      string compare_addr = concat_with_colon(request_.output_runtime_addrs(i).rpc_ip(),
                                              std::to_string(request_.output_runtime_addrs(i).rpc_port()));
      if(output_runtimes_.find(compare_addr)==output_runtimes_.end()){
        status_ = FINISH;
        responder_.Finish(reply_, Status::OK, this);
        return;
      }
    }

    runtime_config migration_source_config = protobuf2local(request_.migration_source_config());
    string migration_source_addr = concat_with_colon(request_.migration_source_config().rpc_ip(),
                                                 std::to_string(request_.migration_source_config().rpc_port()));

    if(migration_sources_.find(migration_source_addr) == migration_sources_.end()){
      migration_sources_.emplace(migration_source_addr, migration_source_config);

      llring_item item(rpc_operation::migration_negotiate, migration_source_config, 0, 0);

      llring_sp_enqueue(rpc2worker_ring_, static_cast<void*>(&item));

      poll_worker2rpc_ring();

      reply_.mutable_migration_target_runtime()->CopyFrom(local2protobuf(local_runtime_));
    }

    status_ = FINISH;
    responder_.Finish(reply_, Status::OK, this);
  } else {
    GPR_ASSERT(status_ == FINISH);
    delete this;
  }
}

//RPC implementation of DeleteMigrationTarget

template<>
void derived_call_data<DeleteMigrationTargetReq, DeleteMigrationTargetRep>::Proceed(){
  if (status_ == CREATE) {
    status_ = PROCESS;
    service_->RequestDeleteMigrationTarget(&ctx_, &request_, &responder_, cq_, cq_, this);
  } else if (status_ == PROCESS) {
    create_itself();

    string migration_target_addr = concat_with_colon(request_.addr().rpc_ip(),
                                                     std::to_string(request_.addr().rpc_port()));

    if(migration_targets_.find(migration_target_addr) != migration_targets_.end()){

      runtime_config& migration_target_config = migration_targets_.find(migration_target_addr)->second;

      llring_item item(rpc_operation::delete_migration_target, migration_target_config, 0, 0);

      llring_sp_enqueue(rpc2worker_ring_, static_cast<void*>(&item));

      poll_worker2rpc_ring();
    }

    status_ = FINISH;
    responder_.Finish(reply_, Status::OK, this);
  } else {
    GPR_ASSERT(status_ == FINISH);
    delete this;
  }
}

//RPC implementation of DeleteMigrationSource

template<>
void derived_call_data<DeleteMigrationSourceReq, DeleteMigrationSourceRep>::Proceed(){
  if (status_ == CREATE) {
    status_ = PROCESS;
    service_->RequestDeleteMigrationSource(&ctx_, &request_, &responder_, cq_, cq_, this);
  } else if (status_ == PROCESS) {
    create_itself();

    string migration_source_addr = concat_with_colon(request_.addr().rpc_ip(),
                                                     std::to_string(request_.addr().rpc_port()));
    if(migration_sources_.find(migration_source_addr)!=migration_sources_.end()){

      runtime_config& migration_source_config = migration_sources_.find(migration_source_addr)->second;

      llring_item item(rpc_operation::delete_migration_source, migration_source_config, 0, 0);

      llring_sp_enqueue(rpc2worker_ring_, static_cast<void*>(&item));

      poll_worker2rpc_ring();
    }

    status_ = FINISH;
    responder_.Finish(reply_, Status::OK, this);
  } else {
    GPR_ASSERT(status_ == FINISH);
    delete this;
  }
}

//RPC implementation for AddReplicas

template<>
void derived_call_data<AddReplicasReq, AddReplicasRep>::Proceed(){
  if (status_ == CREATE) {
    status_ = PROCESS;
    service_->RequestAddReplicas(&ctx_, &request_, &responder_, cq_, cq_, this);
  } else if (status_ == PROCESS) {
    create_itself();

    string local_addr = concat_with_colon(convert_uint32t_ip(local_runtime_.rpc_ip),
                                         std::to_string(local_runtime_.rpc_port));
    RuntimeConfig protobuf_local_runtime =  local2protobuf(local_runtime_);

    for(int i=0; i<request_.addrs_size(); i++){
      string dest_addr = concat_with_colon(request_.addrs(i).rpc_ip(),
                                           std::to_string(request_.addrs(i).rpc_port()));

      if((replicas_.find(dest_addr)!=replicas_.end()) ||
         (dest_addr == local_addr) ||
         (migration_targets_.find(dest_addr) != migration_targets_.end()) ||
         (migration_sources_.find(dest_addr) != migration_sources_.end()) ||
         (input_runtimes_.find(dest_addr) != input_runtimes_.end()) ||
         (output_runtimes_.find(dest_addr) != output_runtimes_.end()) ){
        continue;
      }

      std::unique_ptr<Runtime_RPC::Stub> stub(Runtime_RPC::NewStub(
          grpc::CreateChannel(dest_addr, grpc::InsecureChannelCredentials())));

      ReplicaNegotiateReq req;
      req.mutable_replication_source_info()->CopyFrom(protobuf_local_runtime);

      for(auto it=input_runtimes_.begin(); it!=input_runtimes_.end(); it++){
        auto addr_ptr = req.add_input_runtime_addrs();
        addr_ptr->set_rpc_ip(convert_uint32t_ip(it->second.rpc_ip));
        addr_ptr->set_rpc_port(it->second.rpc_port);
      }

      for(auto it=output_runtimes_.begin(); it!=output_runtimes_.end(); it++){
        auto addr_ptr = req.add_output_runtime_addrs();
        addr_ptr->set_rpc_ip(convert_uint32t_ip(it->second.rpc_ip));
        addr_ptr->set_rpc_port(it->second.rpc_port);
      }

      ReplicaNegotiateRep rep;

      ClientContext context;
      std::chrono::system_clock::time_point deadline =
              std::chrono::system_clock::now() + std::chrono::seconds(FLAGS_rpc_timeout);
      context.set_deadline(deadline);

      Status status = stub->ReplicaNegotiate(&context, req, &rep);

      if(status.ok() && rep.has_replication_target_info()){
        runtime_config target_runtime = protobuf2local(rep.replication_target_info());

        string target_addr = concat_with_colon(convert_uint32t_ip(target_runtime.rpc_ip),
                                               std::to_string(target_runtime.rpc_port));

        replicas_.emplace(target_addr,target_runtime);

        llring_item item(rpc_operation::add_replica, target_runtime, 0, 0);

        llring_sp_enqueue(rpc2worker_ring_, static_cast<void*>(&item));

        poll_worker2rpc_ring();
      }
    }

    status_ = FINISH;
    responder_.Finish(reply_, Status::OK, this);
  } else {
    GPR_ASSERT(status_ == FINISH);
    delete this;
  }
}

// RPC implementation for ReplicaNegotiation

template<>
void derived_call_data<ReplicaNegotiateReq, ReplicaNegotiateRep>::Proceed(){
  if (status_ == CREATE) {
    status_ = PROCESS;
    service_->RequestReplicaNegotiate(&ctx_, &request_, &responder_, cq_, cq_, this);
  } else if (status_ == PROCESS) {
    create_itself();

    if( (input_runtimes_.size()!=request_.input_runtime_addrs_size()) ||
        (output_runtimes_.size()!=request_.output_runtime_addrs_size()) ){
      status_ = FINISH;
      responder_.Finish(reply_, Status::OK, this);
      return;
    }

    for(int i=0; i<request_.input_runtime_addrs_size(); i++){
      string compare_addr = concat_with_colon(request_.input_runtime_addrs(i).rpc_ip(),
                                              std::to_string(request_.input_runtime_addrs(i).rpc_port()));
      if(input_runtimes_.find(compare_addr)==input_runtimes_.end()){
        status_ = FINISH;
        responder_.Finish(reply_, Status::OK, this);
        return;
      }
    }
    for(int i=0;i<request_.output_runtime_addrs_size();i++){
      string compare_addr = concat_with_colon(request_.output_runtime_addrs(i).rpc_ip(),
                                              std::to_string(request_.output_runtime_addrs(i).rpc_port()));
      if(output_runtimes_.find(compare_addr)==output_runtimes_.end()){
        status_ = FINISH;
        responder_.Finish(reply_, Status::OK, this);
        return;
      }
    }

    runtime_config source_runtime = protobuf2local(request_.replication_source_info());

    string source_addr = concat_with_colon(convert_uint32t_ip(source_runtime.rpc_ip),
                                           std::to_string(source_runtime.rpc_port));
    if(storages_.find(source_addr) == storages_.end()){
      storages_.emplace(source_addr,source_runtime);

      llring_item item(rpc_operation::add_storage, source_runtime, 0, 0);

      llring_sp_enqueue(rpc2worker_ring_, static_cast<void*>(&item));

      poll_worker2rpc_ring();

      RuntimeConfig protobuf_local_runtime =  local2protobuf(local_runtime_);

      reply_.mutable_replication_target_info()->CopyFrom(protobuf_local_runtime);
    }

    status_ = FINISH;
    responder_.Finish(reply_, Status::OK, this);
  } else {
    GPR_ASSERT(status_ == FINISH);
    delete this;
  }
}

// RPC implementation for DeleteReplica

template<>
void derived_call_data<DeleteReplicaReq, DeleteReplicaRep>::Proceed(){
  if (status_ == CREATE) {
    status_ = PROCESS;
    service_->RequestDeleteReplica(&ctx_, &request_, &responder_, cq_, cq_, this);
  } else if (status_ == PROCESS) {
    create_itself();

    string delete_replica_addr = concat_with_colon(request_.addrs().rpc_ip(),
                                                   std::to_string(request_.addrs().rpc_port()));
    if((replicas_.find(delete_replica_addr)!=replicas_.end())){

      llring_item item(rpc_operation::remove_replica, replicas_[delete_replica_addr], 0, 0);
      replicas_.erase(delete_replica_addr);

      llring_sp_enqueue(rpc2worker_ring_, static_cast<void*>(&item));

      poll_worker2rpc_ring();
    }

    status_ = FINISH;
    responder_.Finish(reply_, Status::OK, this);
  } else {
    GPR_ASSERT(status_ == FINISH);
    delete this;
  }
}

// RPC implementation for DeleteStorage

template<>
void derived_call_data<DeleteStorageReq, DeleteStorageRep>::Proceed(){
  if (status_ == CREATE) {
    status_ = PROCESS;
    service_->RequestDeleteStorage(&ctx_, &request_, &responder_, cq_, cq_, this);
  } else if (status_ == PROCESS) {
    create_itself();

    string delete_storage_addr = concat_with_colon(request_.addrs().rpc_ip(),
                                                   std::to_string(request_.addrs().rpc_port()));
    if((storages_.find(delete_storage_addr)!=storages_.end())){

      llring_item item(rpc_operation::remove_storage, storages_[delete_storage_addr], 0, 0);
      storages_.erase(delete_storage_addr);

      llring_sp_enqueue(rpc2worker_ring_, static_cast<void*>(&item));

      poll_worker2rpc_ring();
    }

    status_ = FINISH;
    responder_.Finish(reply_, Status::OK, this);
  } else {
    GPR_ASSERT(status_ == FINISH);
    delete this;
  }
}

//RPC implementation for MigrateTo
template<>
void derived_call_data<RecoverReq, RecoverRep>::Proceed(){
  if (status_ == CREATE) {
    status_ = PROCESS;
    service_->RequestRecover(&ctx_, &request_, &responder_, cq_, cq_, this);
  } else if (status_ == PROCESS) {
    create_itself();
    string dest_addr = concat_with_colon(request_.addr().rpc_ip(),
                                         std::to_string(request_.addr().rpc_port()));

    if(storages_.find(dest_addr)!=storages_.end()){
      runtime_config& migration_target_runtime = storages_.find(dest_addr)->second;

      llring_item tmp_item(rpc_operation::recover, migration_target_runtime, 0, 0);

      llring_sp_enqueue(rpc2worker_ring_, static_cast<void*>(&tmp_item));

      poll_worker2rpc_ring();
    }


    status_ = FINISH;
    responder_.Finish(reply_, Status::OK, this);
  } else {
    GPR_ASSERT(status_ == FINISH);
    delete this;
  }
}

// RPC implementation for GetRuntimeState

template<>
void derived_call_data<GetRuntimeStateReq, GetRuntimeStateRep>::Proceed(){
  if (status_ == CREATE) {
    status_ = PROCESS;
    service_->RequestGetRuntimeState(&ctx_, &request_, &responder_, cq_, cq_, this);
  } else if (status_ == PROCESS) {
    create_itself();

    llring_item item(rpc_operation::get_stats, local_runtime_, 0, storages_.size());

    llring_sp_enqueue(rpc2worker_ring_, static_cast<void*>(&item));

    poll_worker2rpc_ring();

    reply_.mutable_port_state()->set_input_port_incoming_pkts(item.stat.input_port_incoming_pkts);
    reply_.mutable_port_state()->set_input_port_outgoing_pkts(item.stat.input_port_outgoing_pkts);
    reply_.mutable_port_state()->set_input_port_dropped_pkts(item.stat.input_port_dropped_pkts);
    reply_.mutable_port_state()->set_output_port_incoming_pkts(item.stat.output_port_incoming_pkts);
    reply_.mutable_port_state()->set_output_port_outgoing_pkts(item.stat.output_port_outgoing_pkts);
    reply_.mutable_port_state()->set_output_port_dropped_pkts(item.stat.output_port_dropped_pkts);
    reply_.mutable_port_state()->set_control_port_incoming_pkts(item.stat.control_port_incoming_pkts);
    reply_.mutable_port_state()->set_control_port_outgoing_pkts(item.stat.control_port_outgoing_pkts);
    reply_.mutable_port_state()->set_control_port_dropped_pkts(item.stat.control_port_dropped_pkts);

    reply_.mutable_flow_state()->set_active_flows(item.stat.active_flows);
    reply_.mutable_flow_state()->set_inactive_flows(item.stat.inactive_flows);


    reply_.mutable_migration_state()->set_migration_index(item.stat.migration_index);
    reply_.mutable_migration_state()->set_migration_target_runtime_id(item.stat.migration_target_runtime_id);
    reply_.mutable_migration_state()->set_migration_qouta(item.stat.migration_qouta);
    reply_.mutable_migration_state()->set_average_flow_migration_completion_time(item.stat.average_flow_migration_completion_time);
    reply_.mutable_migration_state()->set_toal_flow_migration_completion_time(item.stat.toal_flow_migration_completion_time);
    reply_.mutable_migration_state()->set_successful_migration(item.stat.successful_migration);


    for(auto i=0; i<item.stat.array_size; i++){
      StorageState * storage_state_ptr= reply_.add_storage_states();
      storage_state_ptr->set_replication_source_runtime_id(item.stat.array[i].replication_source_runtime_id);
      storage_state_ptr->set_num_of_flow_replicas(item.stat.array[i].num_of_flow_replicas);
      storage_state_ptr->set_total_replay_time(item.stat.array[i].total_replay_time);
    }

    for(auto it=input_runtimes_.begin();it!=input_runtimes_.end();it++){
      RuntimeConfig input_runtime =  local2protobuf(it->second);
      reply_.add_input_runtimes()->CopyFrom(input_runtime);
    }

    for(auto it=output_runtimes_.begin();it!=output_runtimes_.end();it++){
      RuntimeConfig output_runtime =  local2protobuf(it->second);
      reply_.add_output_runtimes()->CopyFrom(output_runtime);
    }

    for(auto it=replicas_.begin();it!=replicas_.end();it++){
      RuntimeConfig replicas_runtime =  local2protobuf(it->second);
      reply_.add_replicas()->CopyFrom(replicas_runtime);
    }

    for(auto it=storages_.begin();it!=storages_.end();it++){
      RuntimeConfig storages_runtime =  local2protobuf(it->second);
      reply_.add_storages()->CopyFrom(storages_runtime);
    }

    // RuntimeConfig migration_runtime =  local2protobuf(migration_target_);
    // reply_.mutable_migration_target()->CopyFrom(migration_runtime);

    RuntimeConfig proto_local_runtime =  local2protobuf(local_runtime_);
    reply_.mutable_local_runtime()->CopyFrom(proto_local_runtime);

    status_ = FINISH;
    responder_.Finish(reply_, Status::OK, this);
  } else {
    GPR_ASSERT(status_ == FINISH);
    delete this;
  }
}

#endif

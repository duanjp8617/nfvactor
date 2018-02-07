#include "server_impl.h"
#include "call_data_impl.h"

#include <glog/logging.h>

bool ServerImpl::Run(string rpc_ip, int32_t rpc_port){
  string server_address = rpc_ip + string(":") + std::to_string(rpc_port);

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service_);

  cq_ = builder.AddCompletionQueue();
  server_ = builder.BuildAndStart();

  if(server_==nullptr){
    LOG(ERROR)<<"RPC server fails to listen on address "<<server_address;
    return false;
  }
  else{
    LOG(INFO)<<"RPC server listsens on address "<<server_address;
    return true;
  }
}

void ServerImpl::HandleRpcs(set<int> cpu_set, int lcore_id, std::atomic<bool>& rpc_server_thread_ready){
  cpu_set_t set;
  CPU_ZERO(&set);
  for(auto cpu_id : cpu_set){
    CPU_SET(cpu_id, &set);
  }
  rte_thread_set_affinity(&set);
  RTE_PER_LCORE(_lcore_id) = lcore_id;

  create_call_data(&service_,
                   cq_.get(),
                   rpc2worker_ring_,
                   worker2rpc_ring_,
                   std::ref(input_runtimes_),
                   std::ref(output_runtimes_),
                   std::ref(replicas_),
                   std::ref(storages_),
                   std::ref(migration_targets_),
                   std::ref(migration_sources_),
                   std::ref(local_runtime_));
  void* tag;
  bool ok;

  rpc_server_thread_ready.store(true);
  while(true){
    GPR_ASSERT(cq_->Next(&tag, &ok));
    GPR_ASSERT(ok);
    static_cast<call_data_base*>(tag)->Proceed();
  }
}

void ServerImpl::HandleRpcs(){
  create_call_data(&service_,
                   cq_.get(),
                   rpc2worker_ring_,
                   worker2rpc_ring_,
                   std::ref(input_runtimes_),
                   std::ref(output_runtimes_),
                   std::ref(replicas_),
                   std::ref(storages_),
                   std::ref(migration_targets_),
                   std::ref(migration_sources_),
                   std::ref(local_runtime_));
  void* tag;
  bool ok;
  while(true){
    GPR_ASSERT(cq_->Next(&tag, &ok));
    GPR_ASSERT(ok);
    static_cast<call_data_base*>(tag)->Proceed();
  }
}

template<class... T>
void ServerImpl::create_call_data(T&&... arg){
  new derived_call_data<LivenessRequest, LivenessReply>(std::forward<T>(arg)...);

  new derived_call_data<AddOutputRtsReq, AddOutputRtsRes>(std::forward<T>(arg)...);
  new derived_call_data<AddInputRtReq, AddInputRtRep>(std::forward<T>(arg)...);
  new derived_call_data<DeleteOutputRtReq, DeleteOutputRtRep>(std::forward<T>(arg)...);
  new derived_call_data<DeleteInputRtReq, DeleteInputRtRep>(std::forward<T>(arg)...);

  new derived_call_data<AddInputMacReq, AddInputMacRep>(std::forward<T>(arg)...);
  new derived_call_data<AddOutputMacReq, AddOutputMacRep>(std::forward<T>(arg)...);
  new derived_call_data<DeleteInputMacReq, DeleteInputMacRep>(std::forward<T>(arg)...);
  new derived_call_data<DeleteOutputMacReq, DeleteOutputMacRep>(std::forward<T>(arg)...);

  new derived_call_data<MigrateToReq, MigrateToRep>(std::forward<T>(arg)...);
  new derived_call_data<SetMigrationTargetReq, SetMigrationTargetRep>(std::forward<T>(arg)...);
  new derived_call_data<MigrationNegotiateReq, MigrationNegotiateRep>(std::forward<T>(arg)...);
  new derived_call_data<DeleteMigrationTargetReq, DeleteMigrationTargetRep>(std::forward<T>(arg)...);
  new derived_call_data<DeleteMigrationSourceReq, DeleteMigrationSourceRep>(std::forward<T>(arg)...);

  new derived_call_data<AddReplicasReq, AddReplicasRep>(std::forward<T>(arg)...);
  new derived_call_data<ReplicaNegotiateReq, ReplicaNegotiateRep>(std::forward<T>(arg)...);
  new derived_call_data<DeleteReplicaReq, DeleteReplicaRep>(std::forward<T>(arg)...);
  new derived_call_data<DeleteStorageReq, DeleteStorageRep>(std::forward<T>(arg)...);
  new derived_call_data<RecoverReq, RecoverRep>(std::forward<T>(arg)...);

  new derived_call_data<GetRuntimeStateReq, GetRuntimeStateRep>(std::forward<T>(arg)...);
}

# grpc related modules
import grpc
import nfa_msg_pb2
import nfa_msg_pb2_grpc

# info modlue
import info

def LivenessCheck(runtime_info):
    channel = grpc.insecure_channel(runtime_info["rpc_ip"]+":"+str(runtime_info["rpc_port"]))

    stub = nfa_msg_pb2_grpc.Runtime_RPCStub(channel)

    response = stub.LivenessCheck(nfa_msg_pb2.LivenessRequest())

    print("LivenessCheck OK!")

def AddOutputRts(source_runtime, target_runtime):
    channel = grpc.insecure_channel( \
    source_runtime["rpc_ip"]+":"+str(source_runtime["rpc_port"]) )

    stub = nfa_msg_pb2_grpc.Runtime_RPCStub(channel)

    request = nfa_msg_pb2.AddOutputRtsReq()
    address = request.addrs.add()
    address.rpc_ip = target_runtime["rpc_ip"]
    address.rpc_port = target_runtime["rpc_port"]

    response = stub.AddOutputRts(request)

    print("AddOutputRts OK!")

def AddOutputMac(source_runtime, target_runtime):
    channel = grpc.insecure_channel( \
    source_runtime["rpc_ip"]+":"+str(source_runtime["rpc_port"]) )

    stub = nfa_msg_pb2_grpc.Runtime_RPCStub(channel)

    request = nfa_msg_pb2.AddOutputMacReq()
    request.addrs.rpc_ip = target_runtime["rpc_ip"]
    request.addrs.rpc_port = target_runtime["rpc_port"]

    response = stub.AddOutputMac(request)

    print("AddOutputMac OK!")

def SetMigrationTarget(source_runtime, target_runtime):
    channel = grpc.insecure_channel( \
    source_runtime["rpc_ip"]+":"+str(source_runtime["rpc_port"]) )

    stub = nfa_msg_pb2_grpc.Runtime_RPCStub(channel)

    request = nfa_msg_pb2.SetMigrationTargetReq()
    address = request.addrs.add()
    address.rpc_ip = target_runtime["rpc_ip"]
    address.rpc_port = target_runtime["rpc_port"]

    response = stub.SetMigrationTarget(request)

    print("SetMigrationTarget OK!")

def MigrateTo(source_runtime, target_runtime, qouta):
    channel = grpc.insecure_channel( \
    source_runtime["rpc_ip"]+":"+str(source_runtime["rpc_port"]) )

    stub = nfa_msg_pb2_grpc.Runtime_RPCStub(channel)

    request = nfa_msg_pb2.MigrateToReq()
    request.addr.rpc_ip = target_runtime["rpc_ip"]
    request.addr.rpc_port = target_runtime["rpc_port"]
    request.quota = qouta

    response = stub.MigrateTo(request)

    print("MigrateTo OK!")

def AddReplicas(source_runtime, target_runtime):
    channel = grpc.insecure_channel( \
    source_runtime["rpc_ip"]+":"+str(source_runtime["rpc_port"]) )

    stub = nfa_msg_pb2_grpc.Runtime_RPCStub(channel)

    request = nfa_msg_pb2.AddReplicasReq()
    address = request.addrs.add()
    address.rpc_ip = target_runtime["rpc_ip"]
    address.rpc_port = target_runtime["rpc_port"]

    response = stub.AddReplicas(request)

    print("AddReplicas OK!")

def Recover(source_runtime, target_runtime):
    channel = grpc.insecure_channel( \
    source_runtime["rpc_ip"]+":"+str(source_runtime["rpc_port"]) )

    stub = nfa_msg_pb2_grpc.Runtime_RPCStub(channel)

    request = nfa_msg_pb2.RecoverReq()
    request.addr.rpc_ip = target_runtime["rpc_ip"]
    request.addr.rpc_port = target_runtime["rpc_port"]

    response = stub.Recover(request)

    print("Recover OK!")

def main():

    AddOutputRts(info.r1_rt1, info.r2_rt1)

    AddOutputRts(info.r1_rt1, info.r3_rt1)

    AddOutputMac(info.r1_rt1, info.r2_rt1)

    #AddOutputMac(info.r1_rt1, info.r3_rt1)

    #SetMigrationTarget(info.r2_rt1, info.r3_rt1)

    #MigrateTo(info.r2_rt1, info.r3_rt1, 1000)

    AddReplicas(info.r2_rt1, info.r3_rt1)

    #Recover(info.r3_rt1, info.r2_rt1)

if __name__ == '__main__':
    main()

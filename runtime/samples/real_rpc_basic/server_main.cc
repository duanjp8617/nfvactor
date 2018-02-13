#include <thread>
#include <chrono>
#include <cassert>
#include <memory>

#include <glog/logging.h>

#include "../../nfaflags.h"
#include "../../nfadpdk.h"
#include "../../bessport/worker.h"
#include "../../bessport/traffic_class.h"
#include "../../bessport/task.h"
#include "../../bessport/scheduler.h"
#include "../../port/sn_port.h"
#include "../../module/port_inc.h"
#include "../../module/port_out.h"
#include "../../module/sink.h"
#include "../../module/timers.h"
#include "../../module/create.h"
#include "../../module/recv_reliable_msgack.h"
#include "../../module/send_reliable_msg.h"
#include "../../module/send_reliable_ack.h"
#include "../../module/forward_ec_scheduler.h"
#include "../../module/reverse_ec_scheduler.h"
#include "../../module/coordinator_mp.h"
#include "../../actor/flow_actor.h"
#include "../../actor/flow_actor_allocator.h"
#include "../../actor/coordinator.h"
#include "../../rpc/llring_holder.h"
#include "../../rpc/server_impl.h"
#include "../../module/handle_command.h"
#include "../../utils/generic_list_item.h"
#include "../../utils/generic_ring_allocator.h"
#include "../../utils/round_rubin_list.h"
#include "../../utils/fast_hash_map.h"
#include "../../utils/fixed_array.h"

using namespace bess;
using namespace std;

int main(int argc, char* argv[]){

  // parse command line options
  google::ParseCommandLineFlags(&argc, &argv, true);
  FLAGS_logtostderr = 1;
  google::InitGoogleLogging(argv[0]);

  // we do some mandatory checking to ensure that the user has given us
  // valid input
  if( (FLAGS_runtime_id == -1) ||
      (FLAGS_input_port_mac == "nil") ||
      (FLAGS_output_port_mac == "nil") ||
      (FLAGS_control_port_mac == "nil") ||
      (FLAGS_rpc_ip == "nil") ||
      (FLAGS_rpc_port == -1) ||
      (FLAGS_input_port == "") ||
      (FLAGS_output_port == "") ||
      (FLAGS_control_port == "") ||
      (FLAGS_worker_core == -1)){
    LOG(ERROR)<<"Invalid command line arguments";
    exit(-1);
  }

  // initialize dpdk environment
  nfa_init_dpdk(argv[0]);

  // create the ZeroCopyVPort in the runtime program
  sn_port input_port;
  if(input_port.init_port(FLAGS_input_port.c_str())==false){
    LOG(ERROR)<<"Fails to open input port "<<FLAGS_input_port;
    exit(EXIT_FAILURE);
  }
  else{
    LOG(INFO)<<"Successfully open input port "<<FLAGS_input_port;
  }

  sn_port output_port;
  if(output_port.init_port(FLAGS_output_port.c_str())==false){
    LOG(ERROR)<<"Fails to open output port "<<FLAGS_output_port;
    exit(EXIT_FAILURE);
  }
  else{
    LOG(INFO)<<"Successfully open output port "<<FLAGS_output_port;
  }

  sn_port control_port;
  if(control_port.init_port(FLAGS_control_port.c_str())==false){
    LOG(ERROR)<<"Fails to open control port "<<FLAGS_control_port;
    exit(EXIT_FAILURE);
  }
  else{
    LOG(INFO)<<"Successfully open control port "<<FLAGS_control_port;
  }

  // create a worker thread
  launch_worker(FLAGS_worker_core, FLAGS_worker_core);

  // create the llring used for communication
  llring_holder communication_ring;

  coordinator coordinator_actor(communication_ring);

  /*LOG(INFO)<<"fast_hash_map test starts";
  fixed_array<int32_t> fa;
  fa.init(2);

  int32_t a = 1;
  fa.add(&a);

  int32_t b = 2;
  fa.add(&b);

  int32_t c = 3;
  fa.add(&c);

  assert(fa.size()==3);
  assert(*(fa.get(0))==1);
  assert(*(fa.get(1))==2);
  assert(*(fa.get(2))==3);


  fa.remove(0);
  assert(fa.size() == 2);
  assert(*(fa.get(0))==2);
  assert(*(fa.get(1))==3);

  fa.remove(0);
  assert(fa.size()==1);
  assert(*(fa.get(0))==3);

  LOG(INFO)<<"fast_hash_map test ends";
  exit(0);*/

  // create module and attach modules to the default traffic class of worker 1.
  // std::unique_ptr<Module> mod_handle_command_ptr(mod_handle_command);
  Module* mod_iport_port_inc = create_module<PortInc>("PortInc", "mod_iport_port_inc", &input_port, 0, 32);
  Module* mod_iport_port_out = create_module<PortOut>("PortOut", "mod_iport_port_out", &input_port);

  Module* mod_oport_port_inc = create_module<PortInc>("PortInc", "mod_oport_port_inc", &output_port, 0, 32);
  Module* mod_oport_port_out = create_module<PortOut>("PortOut", "mod_oport_port_out", &output_port);

  Module* mod_cport_port_inc = create_module<PortInc>("PortInc", "mod_cport_port_inc", &control_port, 0, 32);
  Module* mod_cport_port_out = create_module<PortOut>("PortOut", "mod_cport_port_out", &control_port);

  Module* mod_forward_ec_scheduler = create_module<forward_ec_scheduler>("forward_ec_scheduler",
                                                                         "mod_forward_ec_scheduler",
                                                                         &coordinator_actor);

  Module* mod_reverse_ec_scheduler = create_module<reverse_ec_scheduler>("reverse_ec_scheduler",
                                                                         "mod_reverse_ec_scheduler",
                                                                         &coordinator_actor);

  Module* mod_timers = create_module<timers>("timers", "mod_timer", &coordinator_actor);

  Module* mod_handle_command = create_module<handle_command>("handle_command",
                                                             "mod_handle_command",
                                                             &coordinator_actor);

  Module* mod_recv_reliable_msgack = create_module<recv_reliable_msgack>("recv_reliable_msgack",
                                                                         "mod_recv_reliable_msgack",
                                                                         &coordinator_actor);

  Module* mod_send_reliable_ack = create_module<send_reliable_ack>("send_reliable_ack",
                                                                   "mod_send_reliable_ack",
                                                                   &coordinator_actor);

  Module* mod_send_reliable_msg = create_module<send_reliable_msg>("send_reliable_msg",
                                                                   "mod_send_reliable_msg",
                                                                   &coordinator_actor);

  Module* mod_coordinator_mp = create_module<coordinator_mp>("coordinator_mp",
                                                             "mod_coordinator_mp",
                                                             &coordinator_actor);

  int f1 = mod_iport_port_inc->ConnectModules(0, mod_forward_ec_scheduler, 0);
  int f2 = mod_forward_ec_scheduler->ConnectModules(0, mod_oport_port_out, 0);
  if(f1!=0 || f2!=0 ){
    LOG(ERROR)<<"Error connecting mod_iport_port_inc->mod_forward_ec_scheduler->mod_oport_port_out";
    exit(-1);
  }

  int f3 = mod_oport_port_inc->ConnectModules(0, mod_reverse_ec_scheduler, 0);
  int f4 = mod_reverse_ec_scheduler->ConnectModules(0, mod_iport_port_out, 0);
  if(f3!=0 || f4!=0){
    LOG(ERROR)<<"Error connecting mod_oport_port_inc->mod_reverse_ec_scheduler->mod_iport_port_out";
    exit(-1);
  }

  int f5 = mod_cport_port_inc->ConnectModules(0, mod_recv_reliable_msgack, 0);
  int f5_1 = mod_recv_reliable_msgack->ConnectModules(0, mod_iport_port_out, 0);
  int f5_2 = mod_recv_reliable_msgack->ConnectModules(1, mod_oport_port_out, 0);
  if(f5!=0 || f5_1!=0 || f5_2!=0){
    LOG(ERROR)<<"Error connecting mod_cport_port_inc->mod_recv_reliable_msgack";
    exit(-1);
  }

  int f6 = mod_send_reliable_msg->ConnectModules(0, mod_iport_port_out, 0);
  int f7 = mod_send_reliable_msg->ConnectModules(1, mod_oport_port_out, 0);
  int f8 = mod_send_reliable_msg->ConnectModules(2, mod_cport_port_out, 0);
  if(f6!=0 || f7!=0 || f8!=0){
    LOG(ERROR)<<"Error connecting mod_send_reliable_msg->mod_i/o/cport_port_out";
    exit(-1);
  }

  int f9  = mod_send_reliable_ack->ConnectModules(0, mod_iport_port_out, 0);
  int f10 = mod_send_reliable_ack->ConnectModules(1, mod_oport_port_out, 0);
  int f11 = mod_send_reliable_ack->ConnectModules(2, mod_cport_port_out, 0);
  if(f9!=0 || f10!=0 || f11!=0){
    LOG(ERROR)<<"Error connecting mod_send_reliable_ack->mod_i/o/cport_port_out";
    exit(-1);
  }

  resume_all_workers();

  // create the rpc server
  LOG(INFO)<<"Prepare server";
  ServerImpl rpc_server(communication_ring.rpc2worker_ring(), communication_ring.worker2rpc_ring());
  rpc_server.Run(FLAGS_rpc_ip, FLAGS_rpc_port);
  rpc_server.HandleRpcs();
}


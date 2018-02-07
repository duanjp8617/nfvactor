#include "coordinator.h"
#include "flow_actor_allocator.h"
#include "../actor/base/local_send.h"
#include "../nf/base/network_function_register.h"
#include "./base/actor_misc.h"

#include <glog/logging.h>

coordinator::coordinator(llring_holder& holder){
  allocator_.init(num_flow_actors);

  htable_.Init(flow_key_size, sizeof(flow_actor*));

  actorid_htable_.Init(sizeof(uint64_t), sizeof(flow_actor*));

  deadend_flow_actor_ = allocator_.allocate();

  nfa_ipv4_field::nfa_init_ipv4_field(fields_);

  static_nf_register::get_register().init(allocator_.get_max_actor());

  service_chain_ = static_nf_register::get_register().get_service_chain(parse_service_chain(FLAGS_service_chain));
  total_fs_size_ = 0;
  for(int i=0; i<service_chain_.size(); i++){
    total_fs_size_ += service_chain_[i]->get_nf_state_size();
  }

  mac_list_item_allocator_.init(reliable_send_queue_size*10);

  gp_collector_.init();

  idle_flow_list_.init_list(flow_actor_idle_timeout);
  req_timer_list_.init_list(request_timeout);

  rpc2worker_ring_ = holder.rpc2worker_ring();
  worker2rpc_ring_ = holder.worker2rpc_ring();

  local_runtime_.runtime_id = FLAGS_runtime_id;
  local_runtime_.input_port_mac = convert_string_mac(FLAGS_input_port_mac);
  local_runtime_.output_port_mac = convert_string_mac(FLAGS_output_port_mac);
  local_runtime_.control_port_mac = convert_string_mac(FLAGS_control_port_mac);
  local_runtime_.rpc_ip = convert_string_ip(FLAGS_rpc_ip);
  local_runtime_.rpc_port = FLAGS_rpc_port;
  default_input_mac_ = convert_string_mac(FLAGS_default_input_mac);
  default_output_mac_ = convert_string_mac(FLAGS_default_output_mac);

  migration_qouta_ = 0;
  migration_target_rt_id_ = 0;
  outgoing_migrations_ = 0;
  migration_targets_.init(relloc_size);

  mac_to_reliables_.Init(sizeof(uint64_t), sizeof(reliable_p2p*));

  passive_migration_iteration_ = 0;
  total_passive_migration_ = 0;
  successful_passive_migration_ = 0;
  failed_passive_migration_ = 0;
  null_passive_migration_ = 0;
  migration_source_loss_counter_ = 0;
  current_iteration_start_time_ = 0;
  current_iteration_end_time_ = 0;

  migration_target_loss_counter_ = 0;
  migration_target_buffer_size_counter_ = 0;
  migrated_in_flow_num_ = 0;

  storage_rtid_ = 0;
  out_going_recovery_ = 0;

  recovery_iteration_ = 0;
  successful_recovery_ = 0;
  unsuccessful_recovery_ = 0;
  current_recovery_iteration_start_time_ = 0;
  current_recovery_iteration_end_time_ = 0;

  next_msg_id_ = message_id_start;

  collective_buffer_.init(buffer_batch_size*bess::PacketBatch::kMaxBurst);
}

void coordinator::handle_message(remove_flow_t, flow_actor* flow_actor, flow_key_t* flow_key){

  htable_.Del(flow_key);

  uint64_t actor_id_64 = flow_actor->get_id_64();
  actorid_htable_.Del(&actor_id_64);

  if(flow_actor!=deadend_flow_actor_){
    flow_actor->get_idle_timer()->invalidate();
    flow_actor->get_migration_timer()->invalidate();
    flow_actor->get_replication_timer()->invalidate();
    cdlist_del(reinterpret_cast<cdlist_item*>(flow_actor));
    allocator_.deallocate(flow_actor);
  }
  else{
  }
}

void coordinator::handle_message(ping_t, int32_t sender_rtid, uint32_t sender_actor_id, uint32_t msg_id,
                                 ping_cstruct* cstruct_ptr){

  pong_cstruct cstruct;
  cstruct.val = 1024;
  bool flag = reliables_.find(sender_rtid)->reliable_send(77364, 1, 1, pong_t::value, &cstruct);
  if(flag==false){
    LOG(INFO)<<"Fail to send the message";
  }
  else{
    LOG(INFO)<<"Succeed to send the message";
  }

  LOG(INFO)<<"Recevie ping message!";

  /*if(counter==0){
    start_time = ctx.current_ns();
  }

  counter += 1;

  if(counter == 2*32*1000000){
    LOG(INFO)<<"Receive "<<counter<<" messages.";
    uint64_t total_time = ctx.current_ns()-start_time;
    LOG(INFO)<<"The total transmission time is "<<(total_time/1000000)<<"ms";
  }*/
}

void coordinator::handle_message(pong_t, int32_t sender_rtid, uint32_t sender_actor_id, uint32_t msg_id,
                                 pong_cstruct* cstruct_ptr){
  LOG(INFO)<<"Receiving pong from actor "<<sender_actor_id<<" in runtime "<<sender_rtid;
}

void coordinator::handle_message(create_migration_target_actor_t,
                                 int32_t sender_rtid,
                                 uint32_t sender_actor_id,
                                 uint32_t msg_id,
                                 create_migration_target_actor_cstruct* cstruct_ptr){
  /*LOG(INFO)<<"Receive create_migration_target_actor message sent from runtime "<<sender_rtid
           <<", actor id "<<sender_actor_id
           <<", msg id "<<msg_id
           <<", input runtime id "<<cstruct_ptr->input_header.dest_rtid
           <<", output_runtime_id "<<cstruct_ptr->output_header.dest_rtid;*/


  flow_actor* actor = allocator_.allocate();

  if(unlikely(actor==nullptr)){
    LOG(WARNING)<<"No available flow actors to allocate";
    //TODO: error handling
    return;
  }

  active_flows_rrlist_.add_to_tail(actor);

  send(actor, flow_actor_init_with_cstruct_t::value,
       this,
       &(cstruct_ptr->flow_key),
       service_chain_,
       cstruct_ptr);


  htable_.Set(&(cstruct_ptr->flow_key), &actor);

  uint64_t actor_id_64 = actor->get_id_64();
  actorid_htable_.Set(&actor_id_64, &actor);

  uint32_t response_msg_id = allocate_msg_id();

  start_migration_response_cstruct cstruct;
  cstruct.request_msg_id = msg_id;
  cstruct.migration_target_actor_id = actor->get_id();

  reliables_.find(sender_rtid)->reliable_send(response_msg_id,
                                              coordinator_actor_id,
                                              sender_actor_id,
                                              start_migration_response_t::value,
                                              &cstruct);
}

void coordinator::handle_message(change_vswitch_route_t,
                                 int32_t sender_rtid,
                                 uint32_t sender_actor_id,
                                 uint32_t msg_id,
                                 change_vswitch_route_request_cstruct* cstruct_ptr){
  //LOG(INFO)<<"Receive change_vswitch_route message sent from runtime "<<sender_rtid
  //         <<" with actor id "<<sender_actor_id;

  flow_actor** actor_ptr = htable_.Get(&(cstruct_ptr->flow_key));
  flow_actor* actor = 0;
  change_vswitch_route_response_cstruct cstruct;
  cstruct.request_msg_id = msg_id;

  if(likely(actor_ptr!=nullptr)){
    reliable_p2p* r = reliables_.find(cstruct_ptr->new_output_rt_id);

    actor = *actor_ptr;
    actor->update_output_header(cstruct_ptr->new_output_rt_id, r->get_rt_config()->input_port_mac);

    cstruct.change_route_succeed = 0;
  }
  else{
    cstruct.change_route_succeed = 1;
  }

  uint32_t response_msg_id = allocate_msg_id();
  reliables_.find(sender_rtid)->reliable_send(response_msg_id,
                                              coordinator_actor_id,
                                              sender_actor_id,
                                              change_vswitch_route_response_t::value,
                                              &cstruct);
}

void coordinator::handle_message(replica_recover_t,
                                 int32_t sender_rtid,
                                 uint32_t sender_actor_id,
                                 uint32_t msg_id,
                                 replica_recover_cstruct* cstruct_ptr){
  flow_actor** actor_ptr = htable_.Get(&(cstruct_ptr->flow_key));
  flow_actor* actor = 0;
  change_vswitch_route_response_cstruct cstruct;
  cstruct.request_msg_id = msg_id;

  if(likely(actor_ptr!=nullptr)){
    reliable_p2p* r = reliables_.find(cstruct_ptr->new_output_rt_id);

    actor = *actor_ptr;
    actor->update_output_header(cstruct_ptr->new_output_rt_id, r->get_rt_config()->input_port_mac);

    cstruct.change_route_succeed = 0;
  }
  else{
    cstruct.change_route_succeed = 1;
  }

  uint32_t response_msg_id = allocate_msg_id();
  reliables_.find(sender_rtid)->reliable_send(response_msg_id,
                                              coordinator_actor_id,
                                              sender_actor_id,
                                              replica_recover_response_t::value,
                                              &cstruct);
}

uint64_t coordinator::parse_service_chain(string str){
	std::string::size_type pos;
	std::string pattern(",");
	str+=pattern;
	uint64_t service_chain=0;
	uint8_t nf_id;
	int size=str.size();
	if(str=="null"){
		return service_chain;
	}
  for(int i=0; i<size; i++)
  {
      pos=str.find(pattern,i);
      if(pos<size)
      {
				std::string s=str.substr(i,pos-i);

				nf_id=static_nf_register::get_register().look_up_id(s);

				if(nf_id==0){
					LOG(ERROR)<<"unrecognized service chain flag";
				}else{
					service_chain=(service_chain<<8)|nf_id;
				}


				i=pos+pattern.size()-1;
      }
  }
  LOG(INFO)<<"service_chain: "<<hex<<service_chain;
  return service_chain;
}


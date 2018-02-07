#include "flow_actor.h"
#include "coordinator.h"
#include "./base/local_send.h"
#include "../bessport/utils/time.h"

// flow actor initialization functions
void flow_actor::handle_message(flow_actor_init_with_pkt_t,
                                coordinator* coordinator_actor,
                                flow_key_t* flow_key,
                                vector<network_function_base*>& service_chain,
                                bess::Packet* first_packet,
                                generic_list_item* replica_item){

  current_state_ = flow_actor_normal_processing;

  flow_key_ = *flow_key;
  coordinator_actor_ = coordinator_actor;

  pkt_counter_ = 0;
  sample_counter_ = 0;

  if(replica_item!=nullptr){
    replication_state_ = have_replica;
    r_ = coordinator_actor_->reliables_.find(replica_item->replica_rtid_);
    r_->inc_ref_cnt();
    // LOG(INFO)<<"Flow is initailized with replica on runtime "<<r_->get_rt_config()->runtime_id;
  }
  else{
    replication_state_ = no_replica;
    // LOG(INFO)<<"No replica";
  }

  int32_t input_rtid;
  uint64_t input_rt_output_mac =  (*(first_packet->head_data<uint64_t*>(6)) & 0x0000FFffFFffFFfflu);
  reliable_p2p** r_ptr = coordinator_actor->mac_to_reliables_.Get(&input_rt_output_mac);
  if(unlikely(r_ptr == nullptr)){
    input_rtid = 0;
  }
  else{
    input_rtid = (*r_ptr)->get_rt_config()->runtime_id;
  }
  input_header_.init(input_rtid, input_rt_output_mac, coordinator_actor->local_runtime_.input_port_mac);

  int32_t output_rtid;
  uint64_t output_rt_input_mac;
  generic_list_item* first_item = coordinator_actor->output_runtime_mac_rrlist_.rotate();
  if(unlikely(first_item==nullptr)){
    output_rtid = 0;
    output_rt_input_mac = coordinator_actor->default_output_mac_;
  }
  else{
    output_rt_input_mac = first_item->dst_mac_addr;
    output_rtid = first_item->dst_rtid;
  }
  output_header_.init(output_rtid, output_rt_input_mac, coordinator_actor->local_runtime_.output_port_mac);

  size_t i = 0;
  service_chain_length_ = service_chain.size();

  for(; i<service_chain_length_; i++){
    char* fs_state_ptr = service_chain[i]->allocate();

    if(unlikely(fs_state_ptr == nullptr)){
      LOG(WARNING)<<"flow state allocation failed";
      for(size_t j=0; j<i; j++){
        nfs_.nf[j]->deallocate(fs_.nf_flow_state_ptr[j]);
      }
      service_chain_length_ = 0;
      break;
    }

    nfs_.nf[i] = service_chain[i];
    fs_.nf_flow_state_ptr[i] = fs_state_ptr;
    fs_size_.nf_flow_state_size[i] = service_chain[i]->get_nf_state_size();

    nfs_.nf[i]->nf_init_fs(fs_.nf_flow_state_ptr[i]);
  }

  coordinator_actor_->idle_flow_list_.add_timer(&idle_timer_,
                                                ctx.current_ns(),
                                                idle_message_id,
                                                static_cast<uint16_t>(flow_actor_messages::check_idle));

  cdlist_head_init(&buffer_head_);
}

void flow_actor::handle_message(flow_actor_init_with_cstruct_t,
                                coordinator* coordinator_actor,
                                flow_key_t* flow_key,
                                vector<network_function_base*>& service_chain,
                                create_migration_target_actor_cstruct* cstruct){
  current_state_ = flow_actor_migration_target;
  replication_state_ = no_replica;

  flow_key_ = *flow_key;
  coordinator_actor_ = coordinator_actor;

  pkt_counter_ = 0;
  sample_counter_ = 0;

  input_header_.init(cstruct->input_header.dest_rtid,
                     &(cstruct->input_header.ethh.d_addr),
                     coordinator_actor->local_runtime_.input_port_mac);
  output_header_.init(cstruct->output_header.dest_rtid,
                      &(cstruct->output_header.ethh.d_addr),
                      coordinator_actor->local_runtime_.output_port_mac);

  size_t i = 0;
  service_chain_length_ = service_chain.size();

  for(; i<service_chain_length_; i++){
    char* fs_state_ptr = service_chain[i]->allocate();

    if(unlikely(fs_state_ptr == nullptr)){
      LOG(WARNING)<<"flow state allocation failed";
      for(size_t j=0; j<i; j++){
        nfs_.nf[j]->deallocate(fs_.nf_flow_state_ptr[j]);
      }
      service_chain_length_ = 0;
      break;
    }

    nfs_.nf[i] = service_chain[i];
    fs_.nf_flow_state_ptr[i] = fs_state_ptr;
    fs_size_.nf_flow_state_size[i] = service_chain[i]->get_nf_state_size();

    nfs_.nf[i]->nf_init_fs(fs_.nf_flow_state_ptr[i]);
  }

  coordinator_actor_->idle_flow_list_.add_timer(&idle_timer_,
                                                ctx.current_ns(),
                                                idle_message_id,
                                                static_cast<uint16_t>(flow_actor_messages::check_idle));

  cdlist_head_init(&buffer_head_);
}

void flow_actor::handle_message(flow_actor_init_with_first_rep_pkt_t,
                                coordinator* coordinator_actor,
                                flow_key_t* flow_key,
                                vector<network_function_base*>& service_chain,
                                bess::PacketBatch* first_fs_msg_batch){
  current_state_ = flow_actor_normal_processing;
  replication_state_ = is_replica;

  flow_key_ = *flow_key;
  coordinator_actor_ = coordinator_actor;

  pkt_counter_ = 0;
  sample_counter_ = 0;

  uint32_t input_rtid = *(first_fs_msg_batch->pkts()[0]->head_data<uint32_t*>());
  uint64_t input_rt_output_mac =
      (*(first_fs_msg_batch->pkts()[0]->head_data<uint64_t*>(sizeof(uint32_t))) & 0x0000FFffFFffFFfflu);
  input_header_.init(input_rtid, input_rt_output_mac, coordinator_actor->local_runtime_.input_port_mac);

  uint32_t output_rtid =
      *(first_fs_msg_batch->pkts()[0]->head_data<uint32_t*>(sizeof(uint32_t)+sizeof(struct ether_addr)));
  uint64_t output_rt_input_mac =
      (*(first_fs_msg_batch->pkts()[0]->head_data<uint64_t*>(2*sizeof(uint32_t)+sizeof(struct ether_addr))) & 0x0000FFffFFffFFfflu);
  output_header_.init(output_rtid, output_rt_input_mac, coordinator_actor->local_runtime_.output_port_mac);

  size_t i = 0;
  service_chain_length_ = service_chain.size();

  for(; i<service_chain_length_; i++){
    char* fs_state_ptr = service_chain[i]->allocate();

    if(unlikely(fs_state_ptr == nullptr)){
      LOG(WARNING)<<"flow state allocation failed";
      for(size_t j=0; j<i; j++){
        nfs_.nf[j]->deallocate(fs_.nf_flow_state_ptr[j]);
      }
      service_chain_length_ = 0;
      break;
    }

    nfs_.nf[i] = service_chain[i];
    fs_.nf_flow_state_ptr[i] = fs_state_ptr;
    fs_size_.nf_flow_state_size[i] = service_chain[i]->get_nf_state_size();
  }

  coordinator_actor_->idle_flow_list_.add_timer(&idle_timer_,
                                                ctx.current_ns(),
                                                idle_message_id,
                                                static_cast<uint16_t>(flow_actor_messages::check_idle));

  cdlist_head_init(&buffer_head_);
}

// flow actor idle checking
void flow_actor::handle_message(check_idle_t){
  idle_timer_.invalidate();

  if((current_state_&0x00000001) == 0x00000000){
    sample_counter_ = pkt_counter_;
    coordinator_actor_->idle_flow_list_.add_timer(&idle_timer_,
                                                  ctx.current_ns(),
                                                  idle_message_id,
                                                  static_cast<uint16_t>(flow_actor_messages::check_idle));

    return;
  }

  if(sample_counter_ == pkt_counter_){
    if(current_state_==flow_actor_migration_target){
      cdlist_item* list_item = cdlist_pop_head(&buffer_head_);

      while(list_item!=nullptr){
        buffered_packet* buf_pkt = reinterpret_cast<buffered_packet*>(list_item);
        coordinator_actor_->gp_collector_.collect(buf_pkt->packet);

        coordinator_actor_->collective_buffer_.deallocate(buf_pkt);

        list_item = cdlist_pop_head(&buffer_head_);
      }
    }

    if(replication_state_ == have_replica){
      r_->dec_ref_cnt();
    }

    for(size_t i=0; i<service_chain_length_; i++){
      nfs_.nf[i]->nf_flow_leave(fs_.nf_flow_state_ptr[i], flow_leave_status::idle);
      nfs_.nf[i]->deallocate(fs_.nf_flow_state_ptr[i]);
    }

    send(coordinator_actor_, remove_flow_t::value, this, &flow_key_);
  }
  else{
    sample_counter_ = pkt_counter_;
    coordinator_actor_->idle_flow_list_.add_timer(&idle_timer_,
                                                  ctx.current_ns(),
                                                  idle_message_id,
                                                  static_cast<uint16_t>(flow_actor_messages::check_idle));
  }
}

// replica handling packet and flow state message
void flow_actor::handle_message(rep_fs_pkt_msg_t, bess::PacketBatch* fs_msg_batch){
  if(unlikely(replication_state_ != is_replica)){
    return;
  }

  size_t total_offset = 2*(sizeof(uint32_t)+sizeof(struct ether_addr))+sizeof(flow_key_t);
  int pkt_len = fs_msg_batch->pkts()[0]->total_len() - total_offset - coordinator_actor_->total_fs_size_;

  bess::Packet* pkt = bess::Packet::Alloc();
  pkt->set_data_off(SNBUF_HEADROOM);
  pkt->set_total_len(pkt_len);
  pkt->set_data_len(pkt_len);
  rte_memcpy(pkt->head_data(),
             fs_msg_batch->pkts()[0]->head_data(total_offset+coordinator_actor_->total_fs_size_),
             pkt_len);

  pkt_counter_ += 1;

  // copy the fs_msg_batch.
  bess::Packet* fs_pkt = fs_msg_batch->pkts()[0];
  for(size_t i=0; i<service_chain_length_; i++){
    rte_memcpy(fs_.nf_flow_state_ptr[i], fs_pkt->head_data<char*>(total_offset), fs_size_.nf_flow_state_size[i]);
    total_offset += fs_size_.nf_flow_state_size[i];
  }


  coordinator_actor_->gb_.add_pkt_set_gate(pkt, 1);
}

// normal flow actor handling packet message
void flow_actor::handle_message(pkt_msg_t, bess::Packet* pkt){
  (this->*funcs_[current_state_])(pkt);
}

void flow_actor::no_replication_output(bess::Packet* pkt, bool result){
  coordinator_actor_->ec_scheduler_batch_.add(pkt);
}

void flow_actor::replication_output(bess::Packet* pkt, bool result){
  if(unlikely(r_->check_connection_status()==false)){
    replication_state_ = no_replica;
    r_->dec_ref_cnt();
    coordinator_actor_->ec_scheduler_batch_.add(pkt);
    return;
  }

  bess::Packet* fs_state_pkt = bess::Packet::Alloc();
  assert(fs_state_pkt!=nullptr);

  fs_state_pkt->set_data_off(SNBUF_HEADROOM+pkt_msg_offset);
  int total_size = 2*(sizeof(uint32_t)+sizeof(struct ether_addr))+sizeof(flow_key_t);
  total_size += coordinator_actor_->total_fs_size_;
  fs_state_pkt->set_total_len(total_size+pkt->total_len());
  fs_state_pkt->set_data_len(total_size+pkt->total_len());

  rte_memcpy(fs_state_pkt->head_data(),
             &input_header_,
             sizeof(uint32_t)+sizeof(struct ether_addr));
  rte_memcpy(fs_state_pkt->head_data(sizeof(uint32_t)+sizeof(struct ether_addr)),
             &output_header_,
             sizeof(uint32_t)+sizeof(struct ether_addr));
  rte_memcpy(fs_state_pkt->head_data(2*sizeof(uint32_t)+2*sizeof(struct ether_addr)),
             &flow_key_,
             sizeof(flow_key_t));

  int offset = 2*(sizeof(uint32_t)+sizeof(struct ether_addr))+sizeof(flow_key_t);
  for(size_t i=0; i<service_chain_length_; i++){
    rte_memcpy(fs_state_pkt->head_data(offset), fs_.nf_flow_state_ptr[i], fs_size_.nf_flow_state_size[i]);
    offset += fs_size_.nf_flow_state_size[i];
  }

  rte_memcpy(fs_state_pkt->head_data(total_size), pkt->head_data(), pkt->total_len());
  coordinator_actor_->gp_collector_.collect(pkt);

  bess::PacketBatch batch;
  batch.clear();
  batch.add(fs_state_pkt);

  // LOG(INFO)<<"Sending packets to replica";
  uint32_t msg_id = coordinator_actor_->allocate_msg_id();
  bool flag = r_->reliable_send(msg_id,
                                actor_id_,
                                coordinator_actor_id,
                                replication_msg_t::value,
                                &batch);

  if(unlikely(flag == false)){
    // LOG(INFO)<<"Fail to send packets to replica";
    coordinator_actor_->gp_collector_.collect(&batch);
  }

  // assert(flag == true);
}

void flow_actor::pkt_normal_nf_processing(bess::Packet* pkt){
  pkt_counter_+=1;

  bool result = true;
  for(size_t i=0; i<service_chain_length_; i++){
    rte_prefetch0(fs_.nf_flow_state_ptr[i]);
    result = nfs_.nf[i]->nf_logic(pkt, fs_.nf_flow_state_ptr[i]);

    if(unlikely(result == false)){
      break;
    }
  }

  rte_memcpy(pkt->head_data(), &(output_header_.ethh), sizeof(struct ether_hdr));

  (this->*replication_funcs_[replication_state_])(pkt, result);
}

void flow_actor::pkt_migration_target_processing(bess::Packet* pkt){
  pkt_counter_ += 1;

  buffered_packet* buf_pkt = coordinator_actor_->collective_buffer_.allocate();

  if(unlikely(buf_pkt == nullptr)){
    coordinator_actor_->gp_collector_.collect(pkt);
    coordinator_actor_->migration_target_loss_counter_ += 1;
    return;
  }

  buf_pkt->packet = pkt;

  cdlist_add_tail(&buffer_head_, &(buf_pkt->list_item));
}

void flow_actor::pkt_process_after_route_change(bess::Packet* pkt){
  // assert(1==0);
  coordinator_actor_->gp_collector_.collect(pkt);
  coordinator_actor_->migration_source_loss_counter_ += 1;
}

// the first migration transaction
void flow_actor::handle_message(start_migration_t, int32_t migration_target_rtid){
  if(replication_state_ == have_replica){
    // modify state
    current_state_ = flow_actor_normal_processing;

    // add itself to the tail of the active_flow_actor_list
    coordinator_actor_->active_flows_rrlist_.add_to_tail(this);

    // decrease outgoing_migration
    coordinator_actor_->outgoing_migrations_ -= 1;

    // update stats
    coordinator_actor_->failed_passive_migration_ += 1;

    return;
  }

  current_state_ = flow_actor_migration_source;

  create_migration_target_actor_cstruct cstruct;
  rte_memcpy(&(cstruct.input_header), &input_header_, sizeof(flow_ether_header));
  rte_memcpy(&(cstruct.output_header), &output_header_, sizeof(flow_ether_header));
  rte_memcpy(&(cstruct.flow_key), &flow_key_, sizeof(flow_key_t));

  uint32_t msg_id = coordinator_actor_->allocate_msg_id();
  coordinator_actor_->reliables_.find(migration_target_rtid)->reliable_send(
                                      msg_id,
                                      actor_id_,
                                      coordinator_actor_id,
                                      create_migration_target_actor_t::value,
                                      &cstruct);

  coordinator_actor_->req_timer_list_.add_timer(&migration_timer_,
                                                ctx.current_ns(),
                                                msg_id,
                                                static_cast<uint16_t>(flow_actor_messages::start_migration_timeout));
}

void flow_actor::handle_message(start_migration_timeout_t){
  migration_timer_.invalidate();
  //LOG(INFO)<<"start_migration_timeout is triggered";

  // modify state
  current_state_ = flow_actor_normal_processing;

  // add itself to the tail of the active_flow_actor_list
  coordinator_actor_->active_flows_rrlist_.add_to_tail(this);

  // decrease outgoing_migration
  coordinator_actor_->outgoing_migrations_ -= 1;

  // update stats
  coordinator_actor_->failed_passive_migration_ += 1;
}

void flow_actor::handle_message(start_migration_response_t, start_migration_response_cstruct* cstruct_ptr){
  if(unlikely(cstruct_ptr->request_msg_id != migration_timer_.request_msg_id_)){
    //LOG(INFO)<<"The timer has been triggered, the response is autoamtically discared";
    return;
  }

  //LOG(INFO)<<"The response is successfully received, the id of the migration target is "
  //         <<cstruct_ptr->migration_target_actor_id;
  migration_timer_.invalidate();

  migration_target_actor_id_ = cstruct_ptr->migration_target_actor_id;

  change_vswitch_route_request_cstruct cstruct;
  cstruct.new_output_rt_id = coordinator_actor_->migration_target_rt_id_;
  rte_memcpy(&(cstruct.flow_key), &flow_key_, sizeof(flow_key_t));

  uint32_t msg_id = coordinator_actor_->allocate_msg_id();
  coordinator_actor_->reliables_.find(input_header_.dest_rtid)->reliable_send(
                                      msg_id,
                                      actor_id_,
                                      coordinator_actor_id,
                                      change_vswitch_route_t::value,
                                      &cstruct);

  coordinator_actor_->req_timer_list_.add_timer(&migration_timer_,
                                                ctx.current_ns(),
                                                msg_id,
                                                static_cast<uint16_t>(flow_actor_messages::change_vswitch_route_timeout));
}

// the second migration transaction, plus failure handling
void flow_actor::failure_handling(){
  current_state_ = flow_actor_migration_failure_processing;

  change_vswitch_route_request_cstruct cstruct;
  cstruct.new_output_rt_id = coordinator_actor_->local_runtime_.runtime_id;
  rte_memcpy(&(cstruct.flow_key), &flow_key_, sizeof(flow_key_t));

  uint32_t msg_id = coordinator_actor_->allocate_msg_id();
  coordinator_actor_->reliables_.find(input_header_.dest_rtid)->reliable_send(
                                      msg_id,
                                      actor_id_,
                                      coordinator_actor_id,
                                      change_vswitch_route_t::value,
                                      &cstruct);

  coordinator_actor_->idle_flow_list_.add_timer(&migration_timer_,
                                                ctx.current_ns(),
                                                msg_id,
                                                static_cast<uint16_t>(flow_actor_messages::change_vswitch_route_timeout));
}

void flow_actor::handle_message(change_vswitch_route_timeout_t){
  migration_timer_.invalidate();
  //LOG(INFO)<<"change_vswitch_route_timeout is triggered";

  reliable_p2p* r = coordinator_actor_->reliables_.find(input_header_.dest_rtid);
  if(r->check_connection_status()==false){
    // modify state
    current_state_ = flow_actor_normal_processing;

    // add itself to the tail of the active_flow_actor_list
    coordinator_actor_->active_flows_rrlist_.add_to_tail(this);

    // decrease outgoing_migration
    coordinator_actor_->outgoing_migrations_ -= 1;

    // update stats
    coordinator_actor_->failed_passive_migration_ += 1;
  }
  else{
    failure_handling();
  }
}

void flow_actor::handle_message(change_vswitch_route_response_t, change_vswitch_route_response_cstruct* cstruct_ptr){
  if(unlikely(cstruct_ptr->request_msg_id != migration_timer_.request_msg_id_)){
    LOG(INFO)<<"The timer has been triggered, the response is autoamtically discared";
    return;
  }

  // LOG(INFO)<<"The response is successfully received, the route has been changed";
  migration_timer_.invalidate();

  if( (current_state_ == flow_actor_migration_failure_processing) ||
      (cstruct_ptr->change_route_succeed == 1) ){
    // modify state
    current_state_ = flow_actor_normal_processing;

    // add itself to the tail of the active_flow_actor_list
    coordinator_actor_->active_flows_rrlist_.add_to_tail(this);

    // decrease outgoing_migration
    coordinator_actor_->outgoing_migrations_ -= 1;

    // update stats
    coordinator_actor_->failed_passive_migration_ += 1;

    return;
  }

  current_state_ = flow_actor_migration_source_after_route_change;

  bess::Packet* pkt = bess::Packet::Alloc();
  pkt->set_data_off(SNBUF_HEADROOM+pkt_msg_offset);
  pkt->set_total_len(coordinator_actor_->total_fs_size_);
  pkt->set_data_len(coordinator_actor_->total_fs_size_);

  int offset = 0;
  for(size_t i=0; i<service_chain_length_; i++){
    rte_memcpy(pkt->head_data(offset), fs_.nf_flow_state_ptr[i], fs_size_.nf_flow_state_size[i]);
    offset += fs_size_.nf_flow_state_size[i];
  }

  // LOG(INFO)<<"The size of the total flow states is "<<coordinator_actor_->total_fs_size_;


  bess::PacketBatch batch;
  batch.clear();
  batch.add(pkt);

  uint32_t msg_id = coordinator_actor_->allocate_msg_id();
  bool flag = coordinator_actor_->reliables_.find(coordinator_actor_->migration_target_rt_id_)->reliable_send(
                                                  msg_id,
                                                  actor_id_,
                                                  migration_target_actor_id_,
                                                  migrate_flow_state_t::value,
                                                  &batch);
  if(flag == false){
    coordinator_actor_->gp_collector_.collect(&batch);
    failure_handling();
  }
  else{
    coordinator_actor_->req_timer_list_.add_timer(&migration_timer_,
                                                  ctx.current_ns(),
                                                  msg_id,
                                                  static_cast<uint16_t>(flow_actor_messages::migrate_flow_state_timeout));
  }
}

// the final migration transaction
void flow_actor::handle_message(migrate_flow_state_t,
                                int32_t sender_rtid,
                                uint32_t sender_actor_id,
                                uint32_t request_msg_id,
                                bess::PacketBatch* fs_pkt_batch){
  // LOG(INFO)<<"Receive fs_pkt_batch!!!";

  size_t total_offset = 0;
  bess::Packet* fs_pkt = fs_pkt_batch->pkts()[0];
  // LOG(INFO)<<"The size of fs_pkt is "<<fs_pkt->total_len();
  for(size_t i=0; i<service_chain_length_; i++){
    rte_memcpy(fs_.nf_flow_state_ptr[i], fs_pkt->head_data<char*>(total_offset), fs_size_.nf_flow_state_size[i]);
    total_offset += fs_size_.nf_flow_state_size[i];
    nfs_.nf[i]->nf_flow_arrive(fs_.nf_flow_state_ptr[i], flow_arrive_status::migrate_in);
  }


  uint32_t msg_id = coordinator_actor_->allocate_msg_id();
  migrate_flow_state_response_cstruct cstruct;
  cstruct.request_msg_id = request_msg_id;

  coordinator_actor_->reliables_.find(sender_rtid)->reliable_send(
                                                  msg_id,
                                                  actor_id_,
                                                  sender_actor_id,
                                                  migrate_flow_state_response_t::value,
                                                  &cstruct);

  current_state_ = flow_actor_normal_processing;

  coordinator_actor_->active_flows_rrlist_.add_to_tail(this);

  coordinator_actor_->migrated_in_flow_num_ += 1;

  cdlist_item* list_item = cdlist_pop_head(&buffer_head_);
  while(list_item!=nullptr){
    buffered_packet* buf_pkt = reinterpret_cast<buffered_packet*>(list_item);
    bess::Packet* pkt = buf_pkt->packet;

    bool result = true;
    for(size_t nf_index=0; nf_index<service_chain_length_; nf_index++){
      rte_prefetch0(fs_.nf_flow_state_ptr[nf_index]);
      result = nfs_.nf[nf_index]->nf_logic(pkt, fs_.nf_flow_state_ptr[nf_index]);

      if(unlikely(result == false)){
        break;
      }
    }

    if(result == true){
      rte_memcpy(pkt->head_data(), &(output_header_.ethh), sizeof(struct ether_hdr));
      coordinator_actor_->gb_.add_pkt_set_gate(pkt, 1);
    }
    else{
      coordinator_actor_->gp_collector_.collect(pkt);
    }

    pkt_counter_+=1;
    coordinator_actor_->migration_target_buffer_size_counter_ += 1;

    coordinator_actor_->collective_buffer_.deallocate(buf_pkt);

    list_item = cdlist_pop_head(&buffer_head_);
  }
}


void flow_actor::handle_message(migrate_flow_state_timeout_t){
  migration_timer_.invalidate();
  //LOG(INFO)<<"Receive migrate_flow_state_timeout";

  failure_handling();
}

void flow_actor::handle_message(migrate_flow_state_response_t, migrate_flow_state_response_cstruct* cstruct_ptr){
  if(unlikely(cstruct_ptr->request_msg_id != migration_timer_.request_msg_id_)){
    //LOG(INFO)<<"The timer has been triggered, the response is autoamtically discared";
    return;
  }

  //LOG(INFO)<<"The response is successfully received, the migration has completed";
  migration_timer_.invalidate();

  // modify state
  current_state_ = flow_actor_normal_processing;

  // decrease outgoing_migration
  coordinator_actor_->outgoing_migrations_ -= 1;

  // update stats
  coordinator_actor_->successful_passive_migration_ += 1;

  for(size_t i=0; i<service_chain_length_; i++){
    nfs_.nf[i]->nf_flow_leave(fs_.nf_flow_state_ptr[i], flow_leave_status::migrate_out);
  }

  idle_timer_.invalidate();
  send(coordinator_actor_, remove_flow_t::value, this, &flow_key_);
}

// replica messages
void flow_actor::start_recover(){
  replica_recover_cstruct cstruct;
  cstruct.new_output_rt_id = coordinator_actor_->local_runtime_.runtime_id;
  rte_memcpy(&(cstruct.flow_key), &flow_key_, sizeof(flow_key_t));

  uint32_t msg_id = coordinator_actor_->allocate_msg_id();
  coordinator_actor_->reliables_.find(input_header_.dest_rtid)->reliable_send(
                                      msg_id,
                                      actor_id_,
                                      coordinator_actor_id,
                                      replica_recover_t::value,
                                      &cstruct);

  coordinator_actor_->req_timer_list_.add_timer(&replication_timer_,
                                                ctx.current_ns(),
                                                msg_id,
                                                static_cast<uint16_t>(flow_actor_messages::replica_recover_timeout));
}

void flow_actor::handle_message(replica_recover_timeout_t){
  replication_timer_.invalidate();

  reliable_p2p* r = coordinator_actor_->reliables_.find(input_header_.dest_rtid);
  if(r->check_connection_status()==false){

    replication_state_ = no_replica;

    // add itself to the tail of the active_flow_actor_list
    coordinator_actor_->active_flows_rrlist_.add_to_tail(this);

    // replication stat accouting.
    coordinator_actor_->unsuccessful_recovery_ += 1;

    coordinator_actor_->out_going_recovery_ -= 1;
  }
  else{
    start_recover();
  }
}

void flow_actor::handle_message(replica_recover_response_t, replica_recover_response_cstruct* cstruct_ptr){
  if(unlikely(cstruct_ptr->request_msg_id != replication_timer_.request_msg_id_)){
    //LOG(INFO)<<"The timer has been triggered, the response is autoamtically discared";
    return;
  }

  replication_timer_.invalidate();

  replication_state_ = no_replica;

  // add itself to the tail of the active_flow_actor_list
  coordinator_actor_->active_flows_rrlist_.add_to_tail(this);

  // replication stat accouting.
  coordinator_actor_->successful_recovery_ += 1;

  coordinator_actor_->out_going_recovery_ -= 1;

  for(size_t i=0; i<service_chain_length_; i++){
    nfs_.nf[i]->nf_flow_arrive(fs_.nf_flow_state_ptr[i], flow_arrive_status::recover_on);
  }
}

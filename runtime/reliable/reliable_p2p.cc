#include "reliable_p2p.h"
#include "../actor/coordinator.h"
#include "../bessport/worker.h"

reliable_p2p::reliable_p2p(uint64_t local_rt_mac, uint64_t dest_rt_mac,
                           int local_rtid, int dest_rtid, coordinator* coordinator_actor,
                           uint16_t output_gate, runtime_config* remote_rt_config) :
  send_queue_(local_rt_mac, dest_rt_mac), next_seq_num_to_recv_(1), ref_cnt_(0),
  local_rtid_(local_rtid), dest_rtid_(dest_rtid), coordinator_actor_(coordinator_actor){

  local_runtime_mac_addr_ = *(reinterpret_cast<struct ether_addr*>(&local_rt_mac));
  dst_runtime_mac_addr_ = *(reinterpret_cast<struct ether_addr*>(&dest_rt_mac));

  cur_msg_.send_runtime_id = dest_rtid;

  ack_header_.ethh.d_addr = dst_runtime_mac_addr_;
  ack_header_.ethh.s_addr = local_runtime_mac_addr_;
  ack_header_.ethh.ether_type = 0x0008;
  ack_header_.iph.version_ihl = 0x45;
  ack_header_.iph.total_length = rte_cpu_to_be_16(sizeof(struct ipv4_hdr)+sizeof(uint8_t)+sizeof(uint32_t));
  ack_header_.iph.fragment_offset = rte_cpu_to_be_16(IPV4_HDR_DF_FLAG);
  ack_header_.iph.time_to_live = 64;
  ack_header_.iph.next_proto_id = 0xFF;
  ack_header_.iph.src_addr = 0x0A0A0102;
  ack_header_.iph.dst_addr = 0x0A0A0101;
  ack_header_.iph.hdr_checksum = rte_ipv4_cksum(&(ack_header_.iph));
  ack_header_.magic_num = ack_magic_num;

  output_gate_ = output_gate;

  next_seq_num_to_recv_snapshot_ = 1;

  next_check_time_ = ctx.current_ns() + initial_check_times*send_queue_.peek_rtt();
  last_check_head_seq_num_ = send_queue_.peek_head_seq_num();
  consecutive_counter_ = 0;

  remote_rt_config_ = *remote_rt_config;

  batch_.clear();
  cur_msg_.init();

  is_connection_up_ = true;

  // print_timer_ = 0;
  // error_counter_ = 0;
}

reliable_single_msg* reliable_p2p::recv(bess::Packet* pkt){
  reliable_header* rh = pkt->head_data<reliable_header *>();

  if(unlikely(rh->magic_num == ack_magic_num)){
    send_queue_.pop(rh->seq_num, &(coordinator_actor_->gp_collector_));
    coordinator_actor_->gp_collector_.collect(pkt);
    return nullptr;
  }

  if(unlikely(rh->seq_num != next_seq_num_to_recv_)){
    error_counter_+=1;
    if(error_counter_>16){
      // LOG(INFO)<<"Expecting: "<<next_seq_num_to_recv_;
      // LOG(INFO)<<"Receiving: "<<rh->seq_num;
      error_counter_=0;
      next_seq_num_to_recv_snapshot_ = next_seq_num_to_recv_-1;
    }
    coordinator_actor_->gp_collector_.collect(pkt);
    return nullptr;
  }

  error_counter_ = 0;

  next_seq_num_to_recv_ += 1;
  if(batch_.cnt()==0){
    reliable_message_header* rmh = reinterpret_cast<reliable_message_header*>(rh+1);
    rte_memcpy(&(cur_msg_.rmh), rmh, sizeof(reliable_message_header));
    pkt->adj(sizeof(reliable_header)+sizeof(reliable_message_header));
    batch_.add(pkt);
  }
  else{
    pkt->adj(sizeof(reliable_header));
    batch_.add(pkt);
  }

  if(batch_.cnt() == cur_msg_.rmh.msg_pkt_num){
    bool flag = cur_msg_.format(&batch_);
    if(unlikely(flag == false)){
      coordinator_actor_->gp_collector_.collect(&batch_);
      batch_.clear();
      cur_msg_.clean(&(coordinator_actor_->gp_collector_));
      return nullptr;
    }

    batch_.clear();
    return &cur_msg_;
  }
  else{
    return nullptr;
  }
}

void reliable_p2p::check(uint64_t current_ns){
  if(unlikely(next_check_time_<current_ns)){
    if(last_check_head_seq_num_==send_queue_.peek_head_seq_num() && send_queue_.peek_cur_size()>0){
      uint64_t num_to_send = send_queue_.reset_window_pos();
      prepend_to_reliable_send_list(num_to_send);

      consecutive_counter_ += 1;
      if(consecutive_counter_ == 5000){ // around 10s to connection down.
        LOG(INFO)<<"Connection to "<<dest_rtid_<<" down!!!!!";
        is_connection_up_ = false;
        reset();
        next_seq_num_to_recv_ = 0;
        next_seq_num_to_recv_snapshot_ = 0;
      }
    }
    else{
      consecutive_counter_ = 0;
    }

    next_check_time_ = current_ns + next_check_times*send_queue_.peek_rtt();
    last_check_head_seq_num_ = send_queue_.peek_head_seq_num();
  }
  /*if(ctx.current_ns()>print_timer_ && remote_rt_config_.runtime_id!=2){
    LOG(INFO)<<"Runtime id: "<<remote_rt_config_.runtime_id<<"\n"
             <<"Conection status: "<<is_connection_up_<<"\n"
             <<"consecutive_counter_: "<<consecutive_counter_<<"\n"
             <<"last_check_head_seq_num_: "<<last_check_head_seq_num_<<"\n"
             <<"next_seq_num_to_recv_snapshot_: "<<next_seq_num_to_recv_snapshot_<<"\n"
             <<"next_seq_num_to_recv_: "<<next_seq_num_to_recv_;
    send_queue_.print();

    print_timer_ = ctx.current_ns()+3000000000;
  }*/
}

void reliable_p2p::reset(){
  send_queue_.reset(&(coordinator_actor_->gp_collector_));
  next_seq_num_to_recv_ = 1;
  cur_msg_.clean(&(coordinator_actor_->gp_collector_));
  batch_.clear();
}

void reliable_p2p::add_to_reliable_send_list(int pkt_num){
  generic_list_item* last_item = coordinator_actor_->reliable_send_list_.peek_tail();

  if(unlikely(last_item==nullptr || last_item->reliable_rtid != dest_rtid_)){
    generic_list_item* list_item = coordinator_actor_->get_list_item_allocator()->allocate();

    list_item->pkt_num = pkt_num;
    list_item->reliable_rtid = dest_rtid_;
    list_item->output_gate = output_gate_;

    coordinator_actor_->reliable_send_list_.add_to_tail(list_item);

    return;
  }

  last_item->pkt_num += pkt_num;
}

void reliable_p2p::prepend_to_reliable_send_list(int pkt_num){
  generic_list_item* first_item = coordinator_actor_->reliable_send_list_.peek_head();

  if(unlikely(first_item==nullptr || first_item->reliable_rtid != dest_rtid_)){
    generic_list_item* list_item = coordinator_actor_->get_list_item_allocator()->allocate();

    list_item->pkt_num = pkt_num;
    list_item->reliable_rtid = dest_rtid_;
    list_item->output_gate = output_gate_;

    coordinator_actor_->reliable_send_list_.add_to_head(list_item);

    return;
  }

  first_item->pkt_num += pkt_num;
}

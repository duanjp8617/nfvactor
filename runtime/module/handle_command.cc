//
#include "handle_command.h"
#include "../bessport/kmod/llring.h"

#include <glog/logging.h>

void handle_command::customized_init(coordinator* coordinator_actor){
  RegisterTask(nullptr);
  coordinator_actor_ = coordinator_actor;
}

struct task_result handle_command::RunTask(void *arg){
  struct task_result ret;
  ret = (struct task_result){
      .packets = 0, .bits = 0,
  };

  void* dequeue_output[1];

  int flag = llring_sc_dequeue(coordinator_actor_->rpc2worker_ring_, dequeue_output);

  if(unlikely(flag == 0)){
    // do the processing

    llring_item* item = static_cast<llring_item*>(dequeue_output[0]);

    LOG(INFO) << "Receive "<<opcode2string(item->op_code)<<" ring message.";
    print_config(item->rt_config);
    LOG(INFO) << "migration_qouta-> "<<item->migration_qouta;
    print_stat(item->op_code, item->stat);

    switch(item->op_code){
      case rpc_operation::add_input_runtime :{
        reliable_p2p* r =
            coordinator_actor_->reliables_.emplace(item->rt_config.runtime_id,
                                                   coordinator_actor_->local_runtime_.input_port_mac,
                                                   item->rt_config.output_port_mac,
                                                   coordinator_actor_->local_runtime_.runtime_id,
                                                   item->rt_config.runtime_id,
                                                   coordinator_actor_,
                                                   0,
                                                   &(item->rt_config));

        coordinator_actor_->mac_to_reliables_.Set(&(item->rt_config.output_port_mac), &r);
        LOG(INFO)<<"Finish creating a reliable to input runtime";
        break;
      }
      case rpc_operation::add_output_runtime :{
        reliable_p2p* r = coordinator_actor_->reliables_.emplace(item->rt_config.runtime_id,
                                                                 coordinator_actor_->local_runtime_.output_port_mac,
                                                                 item->rt_config.input_port_mac,
                                                                 coordinator_actor_->local_runtime_.runtime_id,
                                                                 item->rt_config.runtime_id,
                                                                 coordinator_actor_,
                                                                 1,
                                                                 &(item->rt_config));

        coordinator_actor_->mac_to_reliables_.Set(&(item->rt_config.input_port_mac), &r);

        uint32_t msg_id = coordinator_actor_->allocate_msg_id();
        ping_cstruct cstruct;
        bool flag = r->reliable_send(
                                     msg_id,
                                     coordinator_actor_id,
                                     coordinator_actor_id,
                                     ping_t::value,
                                     &cstruct);
        assert(flag == true);

        LOG(INFO)<<"Finish creating a reliable to output runtime";
        break;
      }
      case rpc_operation::delete_input_runtime :{
        coordinator_actor_->reliables_.erase(item->rt_config.runtime_id);
        coordinator_actor_->mac_to_reliables_.Del(&(item->rt_config.output_port_mac));

        cdlist_item *c_item = nullptr;
        cdlist_for_each(c_item, coordinator_actor_->input_runtime_mac_rrlist_.get_list_head()){
          generic_list_item* g_item = reinterpret_cast<generic_list_item*>(c_item);
          if(g_item->dst_mac_addr == item->rt_config.output_port_mac){
            cdlist_del(c_item);
            coordinator_actor_->get_list_item_allocator()->deallocate(g_item);
            break;
          }
        }
        break;
      }
      case rpc_operation::delete_output_runtime :{
        coordinator_actor_->reliables_.erase(item->rt_config.runtime_id);
        coordinator_actor_->mac_to_reliables_.Del(&(item->rt_config.input_port_mac));

        cdlist_item *c_item = nullptr;
        cdlist_for_each(c_item, coordinator_actor_->output_runtime_mac_rrlist_.get_list_head()){
          generic_list_item* g_item = reinterpret_cast<generic_list_item*>(c_item);
          if(g_item->dst_mac_addr == item->rt_config.input_port_mac){
            cdlist_del(c_item);
            coordinator_actor_->get_list_item_allocator()->deallocate(g_item);
            break;
          }
        }
        break;
      }
      case rpc_operation::add_input_mac :{
        generic_list_item* list_item = coordinator_actor_->get_list_item_allocator()->allocate();
        list_item->dst_mac_addr = item->rt_config.output_port_mac;
        list_item->dst_rtid = item->rt_config.runtime_id;
        coordinator_actor_->input_runtime_mac_rrlist_.add_to_tail(list_item);
        break;
      }
      case rpc_operation::add_output_mac :{
        generic_list_item* list_item = coordinator_actor_->get_list_item_allocator()->allocate();
        list_item->dst_mac_addr = item->rt_config.input_port_mac;
        list_item->dst_rtid = item->rt_config.runtime_id;
        coordinator_actor_->output_runtime_mac_rrlist_.add_to_tail(list_item);
        break;
      }
      case rpc_operation::delete_input_mac :{
        cdlist_item *c_item = nullptr;
        cdlist_for_each(c_item, coordinator_actor_->input_runtime_mac_rrlist_.get_list_head()){
          generic_list_item* g_item = reinterpret_cast<generic_list_item*>(c_item);
          if(g_item->dst_mac_addr == item->rt_config.output_port_mac){
            cdlist_del(c_item);
            coordinator_actor_->get_list_item_allocator()->deallocate(g_item);
            break;
          }
        }
        break;
      }
      case rpc_operation::delete_output_mac :{
        cdlist_item *c_item = nullptr;
        cdlist_for_each(c_item, coordinator_actor_->output_runtime_mac_rrlist_.get_list_head()){
          generic_list_item* g_item = reinterpret_cast<generic_list_item*>(c_item);
          if(g_item->dst_mac_addr == item->rt_config.input_port_mac){
            cdlist_del(c_item);
            coordinator_actor_->get_list_item_allocator()->deallocate(g_item);
            break;
          }
        }
        break;
      }
      case rpc_operation::migrate_to :{
        if(coordinator_actor_->migration_qouta_==0){
          coordinator_actor_->migration_qouta_ = item->migration_qouta;
          coordinator_actor_->migration_target_rt_id_ = item->rt_config.runtime_id;
          coordinator_actor_->outgoing_migrations_ = 0;

          coordinator_actor_->passive_migration_iteration_+=1;
          coordinator_actor_->total_passive_migration_ = item->migration_qouta;
          coordinator_actor_->successful_passive_migration_ = 0;
          coordinator_actor_->failed_passive_migration_ = 0;
          coordinator_actor_->null_passive_migration_ = 0;
          coordinator_actor_->migration_source_loss_counter_ = 0;
          coordinator_actor_->current_iteration_start_time_ = ctx.current_ns();
        }
        break;
      }
      case rpc_operation::set_migration_target :{
        reliable_p2p* r = coordinator_actor_->reliables_.find(item->rt_config.runtime_id);
        if(r==nullptr){
          r = coordinator_actor_->reliables_.emplace(item->rt_config.runtime_id,
                                            coordinator_actor_->local_runtime_.control_port_mac,
                                            item->rt_config.control_port_mac,
                                            coordinator_actor_->local_runtime_.runtime_id,
                                            item->rt_config.runtime_id,
                                            coordinator_actor_,
                                            2,
                                            &(item->rt_config));


          coordinator_actor_->mac_to_reliables_.Set(&(item->rt_config.control_port_mac), &r);

          r->inc_ref_cnt();

          uint32_t msg_id = coordinator_actor_->allocate_msg_id();
          ping_cstruct cstruct;
          bool flag = r->reliable_send(
                                       msg_id,
                                       coordinator_actor_id,
                                       coordinator_actor_id,
                                       ping_t::value,
                                       &cstruct);
          assert(flag == true);
        }
        else{
          r->inc_ref_cnt();
        }

        coordinator_actor_->migration_targets_.add(&(item->rt_config.runtime_id));
        break;
      }
      case rpc_operation::migration_negotiate :{
        reliable_p2p* r = coordinator_actor_->reliables_.find(item->rt_config.runtime_id);
        if(r==nullptr){
          r = coordinator_actor_->reliables_.emplace(item->rt_config.runtime_id,
                                            coordinator_actor_->local_runtime_.control_port_mac,
                                            item->rt_config.control_port_mac,
                                            coordinator_actor_->local_runtime_.runtime_id,
                                            item->rt_config.runtime_id,
                                            coordinator_actor_,
                                            2,
                                            &(item->rt_config));


          coordinator_actor_->mac_to_reliables_.Set(&(item->rt_config.control_port_mac), &r);

          r->inc_ref_cnt();
        }
        else{
          r->inc_ref_cnt();
        }
        break;
      }
      case rpc_operation::delete_migration_target :{
        reliable_p2p* r = coordinator_actor_->reliables_.find(item->rt_config.runtime_id);
        r->dec_ref_cnt();
        if(r->is_ref_cnt_zero()){
          coordinator_actor_->mac_to_reliables_.Del(&(r->get_rt_config()->control_port_mac));
          coordinator_actor_->reliables_.erase(item->rt_config.runtime_id);
        }
        for(size_t i=0; i<coordinator_actor_->migration_targets_.size(); i++){
          int32_t rtid = *(coordinator_actor_->migration_targets_.get(i));
          if(rtid == item->rt_config.runtime_id){
            coordinator_actor_->migration_targets_.remove(i);
            break;
          }
        }
        break;
      }
      case rpc_operation::delete_migration_source :{
        reliable_p2p* r = coordinator_actor_->reliables_.find(item->rt_config.runtime_id);
        r->dec_ref_cnt();
        if(r->is_ref_cnt_zero()){
          coordinator_actor_->mac_to_reliables_.Del(&(r->get_rt_config()->control_port_mac));
          coordinator_actor_->reliables_.erase(item->rt_config.runtime_id);
        }
        break;
      }
      case rpc_operation::add_replica :{
        generic_list_item* list_item = coordinator_actor_->get_list_item_allocator()->allocate();
        list_item->replica_rtid_ = item->rt_config.runtime_id;
        coordinator_actor_->replicas_rrlist_.add_to_tail(list_item);

        reliable_p2p* r = coordinator_actor_->reliables_.find(item->rt_config.runtime_id);
        if(r==nullptr){
          r = coordinator_actor_->reliables_.emplace(item->rt_config.runtime_id,
                                            coordinator_actor_->local_runtime_.control_port_mac,
                                            item->rt_config.control_port_mac,
                                            coordinator_actor_->local_runtime_.runtime_id,
                                            item->rt_config.runtime_id,
                                            coordinator_actor_,
                                            2,
                                            &(item->rt_config));

          coordinator_actor_->mac_to_reliables_.Set(&(item->rt_config.control_port_mac), &r);

          r->inc_ref_cnt();

          uint32_t msg_id = coordinator_actor_->allocate_msg_id();
          ping_cstruct cstruct;
          bool flag = r->reliable_send(
                                       msg_id,
                                       coordinator_actor_id,
                                       coordinator_actor_id,
                                       ping_t::value,
                                       &cstruct);
          assert(flag == true);
        }
        else{
          r->inc_ref_cnt();
        }

        break;
      }
      case rpc_operation::add_storage :{
        reliable_p2p* r = coordinator_actor_->reliables_.find(item->rt_config.runtime_id);
        if(r==nullptr){
          r = coordinator_actor_->reliables_.emplace(item->rt_config.runtime_id,
                                            coordinator_actor_->local_runtime_.control_port_mac,
                                            item->rt_config.control_port_mac,
                                            coordinator_actor_->local_runtime_.runtime_id,
                                            item->rt_config.runtime_id,
                                            coordinator_actor_,
                                            2,
                                            &(item->rt_config));

          coordinator_actor_->mac_to_reliables_.Set(&(item->rt_config.control_port_mac), &r);

          coordinator_actor_->replica_flow_lists_.emplace(item->rt_config.runtime_id);

          cdlist_head_init(coordinator_actor_->replica_flow_lists_.find(item->rt_config.runtime_id));

          r->inc_ref_cnt();
        }
        else{
          r->inc_ref_cnt();
        }

        break;
      }
      case rpc_operation::remove_replica :{
        cdlist_item *c_item = nullptr;
        cdlist_for_each(c_item, coordinator_actor_->replicas_rrlist_.get_list_head()){
          generic_list_item* g_item = reinterpret_cast<generic_list_item*>(c_item);
          if(g_item->replica_rtid_ == item->rt_config.runtime_id){
            cdlist_del(c_item);
            coordinator_actor_->get_list_item_allocator()->deallocate(g_item);
            break;
          }
        }

        reliable_p2p* r = coordinator_actor_->reliables_.find(item->rt_config.runtime_id);
        r->dec_ref_cnt();
        if(r->is_ref_cnt_zero()){
          coordinator_actor_->mac_to_reliables_.Del(&(r->get_rt_config()->control_port_mac));
          coordinator_actor_->reliables_.erase(item->rt_config.runtime_id);
        }

        break;
      }
      case rpc_operation::remove_storage :{
        // TODO:  remove all the storage flow actors
        reliable_p2p* r = coordinator_actor_->reliables_.find(item->rt_config.runtime_id);
        r->dec_ref_cnt();
        if(r->is_ref_cnt_zero()){
          coordinator_actor_->mac_to_reliables_.Del(&(r->get_rt_config()->control_port_mac));
          coordinator_actor_->reliables_.erase(item->rt_config.runtime_id);
          coordinator_actor_->replica_flow_lists_.erase(item->rt_config.runtime_id);
        }

        break;
      }
      case rpc_operation::recover: {
        if(coordinator_actor_->storage_rtid_ == 0){
          coordinator_actor_->storage_rtid_ = item->rt_config.runtime_id;

          coordinator_actor_->recovery_iteration_ +=1;
          coordinator_actor_->successful_recovery_ = 0;
          coordinator_actor_->unsuccessful_recovery_ = 0;
          coordinator_actor_->current_recovery_iteration_start_time_ = ctx.current_ns();
        }
        break;
      }
      case rpc_operation::get_stats :{
        break;
      }
      default :
        break;
    }

    LOG(INFO)<<"The worker thread put the item to the ring";
    llring_sp_enqueue(coordinator_actor_->worker2rpc_ring_, static_cast<void*>(item));
  }

  return ret;
}

ADD_MODULE(handle_command, "handle_command", "handle rpc command received from the rpc thread")

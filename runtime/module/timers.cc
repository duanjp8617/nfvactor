//
#include "timers.h"

void timers::customized_init(coordinator* coordinator_actor){
  RegisterTask(nullptr);
  coordinator_actor_ = coordinator_actor;
}

struct task_result timers::RunTask(void *arg) {
  struct task_result ret;
  ret = (struct task_result){
      .packets = 0, .bits = 0,
  };

  for(size_t i=0; i<bess::PacketBatch::kMaxBurst; i++){
    if(unlikely(coordinator_actor_->idle_flow_list_.timeout_occur(ctx.current_ns()))){
      coordinator_actor_->idle_flow_list_.trigger_timer();
    }
    else{
      break;
    }
  }

  for(size_t i=0; i<bess::PacketBatch::kMaxBurst; i++){
    if(unlikely(coordinator_actor_->req_timer_list_.timeout_occur(ctx.current_ns()))){
      coordinator_actor_->req_timer_list_.trigger_timer();
    }
    else{
      break;
    }
  }

  return ret;
}

ADD_MODULE(timers, "timers module", "check the timeout for fixed timers")

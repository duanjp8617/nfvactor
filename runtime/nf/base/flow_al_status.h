#ifndef FLOW_AL_STATUS_H
#define FLOW_AL_STATUS_H

// flow arrive leave status
enum class flow_arrive_status{
  migrate_in,
  recover_on
};

enum class flow_leave_status{
  idle,
  migrate_out
};

#endif

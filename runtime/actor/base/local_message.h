#ifndef LOCAL_MESSAGE_H
#define LOCAL_MESSAGE_H

#include <cstdint>

struct local_message_base{
};

template<uint16_t N>
struct local_message_derived : local_message_base{
public:
  static local_message_derived value;
  static const uint16_t specifier = N;
};

#define local_message(actor_msg_enum, msg) local_message_derived<static_cast<uint16_t>(actor_msg_enum::msg)>

#endif

#ifndef LOCAL_SEND_H
#define LOCAL_SEND_H

#include "local_message.h"

template<class TActor, class Mid, class... TArgs>
inline void send(TActor* dest, Mid mid, TArgs&&... args){
  static_assert(std::is_base_of<local_message_base, Mid>::value, "Invalid argument when using send");
  dest->handle_message(mid, std::forward<TArgs>(args)...);
}


#endif


#ifndef __SMART_LINK_LOG_H_
#define __SMART_LINK_LOG_H_

unsigned int smart_link_log_type_set(unsigned int type);
unsigned int smart_link_log_type_unset(unsigned int type);

#define smart_link_log_func_decl(type)  \
void smart_link_log_##type(char *format, ...);

smart_link_log_func_decl(event)
smart_link_log_func_decl(debug)
smart_link_log_func_decl(packet)
smart_link_log_func_decl(error)

int smart_link_sync_log_event(int pri, char* format, ...);

#endif

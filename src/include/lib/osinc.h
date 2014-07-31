/*include all linux header */

#ifndef _LIB_OSINC_H
#define _LIB_OSINC_H



#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <sys/fcntl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include <syslog.h>
#include <sys/time.h>
#include <time.h>
#include <sys/uio.h>
#include <sys/utsname.h>
#include <sys/un.h>
#include <limits.h>
#include <strings.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/igmp.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/if_ether.h> 
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/filter.h>
#include <stddef.h>
#include <arpa/inet.h>
#include <netinet/icmp6.h>
#include <netinet/ip6.h>
#include <netinet/if_ether.h>
#include <pthread.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <assert.h>
#include <dbus/dbus.h>
#include <linux/tipc.h>
#include <getopt.h>
#include <dirent.h>
#include <sys/poll.h>


/* MAX / MIN are not commonly defined, but useful */
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif 
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#define IN
#define OUT
#define INOUT

#endif /* _ZEBRA_H */

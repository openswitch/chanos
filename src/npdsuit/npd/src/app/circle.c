
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* circle.c
*
*
* CREATOR:
*
* DESCRIPTION:
* asd module
*
*
*******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>


#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/uio.h>

#include <sys/time.h>

#include "os.h"
#include "circle.h"
#include "npd_log.h"

/*

struct circle_sock {
	int sock;
	void *circle_data;
	void *user_data;
	circle_sock_handler handler;
};

struct circle_timeout {
	struct os_time time;
	void *circle_data;
	void *user_data;
	circle_timeout_handler handler;
	struct circle_timeout *next;
};

struct circle_signal {
	int sig;
	void *user_data;
	circle_signal_handler handler;
	int signaled;
};

struct circle_sock_table {
	int count;
	struct circle_sock *table;
	int changed;
};

struct circle_data {
	void *user_data;

	int max_sock;

	struct circle_sock_table readers;
	struct circle_sock_table writers;
	struct circle_sock_table exceptions;

	struct circle_timeout *timeout;

	int signal_count;
	struct circle_signal *signals;
	int signaled;
	int pending_terminate;

	int terminate;
	int reader_table_changed;
};
*/
struct circle_data circle;


int circle_init(void *user_data)
{
	os_memset(&circle, 0, sizeof(circle));
	circle.user_data = user_data;
	return 0;
}


static int circle_sock_table_add_sock(struct circle_sock_table *table,
                                     int sock, circle_sock_handler handler,
                                     void *circle_data, void *user_data)
{
	struct circle_sock *tmp;

	if (table == NULL)
		return -1;

	tmp = (struct circle_sock *)
		os_realloc(table->table,
			   (table->count + 1) * sizeof(struct circle_sock));
	if (tmp == NULL)
		return -1;

	tmp[table->count].sock = sock;
	tmp[table->count].circle_data = circle_data;
	tmp[table->count].user_data = user_data;
	tmp[table->count].handler = handler;
	table->count++;
	table->table = tmp;
	if (sock > circle.max_sock)
		circle.max_sock = sock;
	table->changed = 1;

	return 0;
}


static void circle_sock_table_remove_sock(struct circle_sock_table *table,
                                         int sock)
{
	int i;

	if (table == NULL || table->table == NULL || table->count == 0)
		return;

	for (i = 0; i < table->count; i++) {
		if (table->table[i].sock == sock)
			break;
	}
	if (i == table->count)
		return;
	if (i != table->count - 1) {
		os_memmove(&table->table[i], &table->table[i + 1],
			   (table->count - i - 1) *
			   sizeof(struct circle_sock));
	}
	table->count--;
	table->changed = 1;
}


static void circle_sock_table_set_fds(struct circle_sock_table *table,
				     fd_set *fds)
{
	int i;

	FD_ZERO(fds);

	if (table->table == NULL)
		return;

	for (i = 0; i < table->count; i++)
		FD_SET(table->table[i].sock, fds);
}


static void circle_sock_table_dispatch(struct circle_sock_table *table,
				      fd_set *fds)
{
	int i;

	if (table == NULL || table->table == NULL)
		return;

	table->changed = 0;
	for (i = 0; i < table->count; i++) {
		if (FD_ISSET(table->table[i].sock, fds)) {
			table->table[i].handler(table->table[i].sock,
						table->table[i].circle_data,
						table->table[i].user_data);
			if (table->changed)
				break;
		}
	}
}


static void circle_sock_table_destroy(struct circle_sock_table *table)
{
	if (table) {
		int i;
		for (i = 0; i < table->count && table->table; i++) {
			npd_syslog_dbg("circle: remaining socket: sock=%d "
			       "circle_data=%p user_data=%p handler=%p\n",
			       table->table[i].sock,
			       table->table[i].circle_data,
			       table->table[i].user_data,
			       table->table[i].handler);
		}
		os_free(table->table);
	}
}


int circle_register_read_sock(int sock, circle_sock_handler handler,
			     void *circle_data, void *user_data)
{
	return circle_register_sock(sock, EVENT_TYPE_READ, handler,
				   circle_data, user_data);
}


void circle_unregister_read_sock(int sock)
{
	circle_unregister_sock(sock, EVENT_TYPE_READ);
}


static struct circle_sock_table *circle_get_sock_table(circle_event_type type)
{
	switch (type) {
	case EVENT_TYPE_READ:
		return &circle.readers;
	case EVENT_TYPE_WRITE:
		return &circle.writers;
	case EVENT_TYPE_EXCEPTION:
		return &circle.exceptions;
	}

	return NULL;
}


int circle_register_sock(int sock, circle_event_type type,
			circle_sock_handler handler,
			void *circle_data, void *user_data)
{
	struct circle_sock_table *table;

	table = circle_get_sock_table(type);
	return circle_sock_table_add_sock(table, sock, handler,
					 circle_data, user_data);
}


void circle_unregister_sock(int sock, circle_event_type type)
{
	struct circle_sock_table *table;

	table = circle_get_sock_table(type);
	circle_sock_table_remove_sock(table, sock);
}


int circle_register_timeout(unsigned int secs, unsigned int usecs,
			   circle_timeout_handler handler,
			   void *circle_data, void *user_data)
{
	struct circle_timeout *timeout, *tmp, *prev;

	timeout = os_malloc(sizeof(*timeout));
	if (timeout == NULL)
		return -1;
	os_get_time(&timeout->time);
	timeout->time.sec += secs;
	timeout->time.usec += usecs;
	while (timeout->time.usec >= 1000000) {
		timeout->time.sec++;
		timeout->time.usec -= 1000000;
	}
	timeout->circle_data = circle_data;
	timeout->user_data = user_data;
	timeout->handler = handler;
	timeout->next = NULL;

	if (circle.timeout == NULL) {
		circle.timeout = timeout;
		return 0;
	}

	prev = NULL;
	tmp = circle.timeout;
	while (tmp != NULL) {
		if (os_time_before(&timeout->time, &tmp->time))
			break;
		prev = tmp;
		tmp = tmp->next;
	}

	if (prev == NULL) {
		timeout->next = circle.timeout;
		circle.timeout = timeout;
	} else {
		timeout->next = prev->next;
		prev->next = timeout;
	}
	return 0;
}


int circle_cancel_timeout(circle_timeout_handler handler,
			 void *circle_data, void *user_data)
{
	struct circle_timeout *timeout, *prev, *next;
	int removed = 0;

	prev = NULL;
	timeout = circle.timeout;
	while (timeout != NULL) {
		next = timeout->next;

		if (timeout->handler == handler &&
		    (timeout->circle_data == circle_data ||
		     circle_data == circle_ALL_CTX) &&
		    (timeout->user_data == user_data ||
		     user_data == circle_ALL_CTX)) {
			if (prev == NULL)
				circle.timeout = next;
			else
				prev->next = next;
			os_free(timeout);
			removed++;
		} else
			prev = timeout;

		timeout = next;
	}

	return removed;
}


static void circle_handle_alarm(int sig)
{
	npd_syslog_err("circle: could not process SIGINT or SIGTERM in two "
		"seconds. Looks like there\n"
		"is a bug that ends up in a busy loop that "
		"prevents clean shutdown.\n"
		"Killing program forcefully.\n");
	exit(1);
}


static void circle_handle_signal(int sig)
{
	int i;

	if ((sig == SIGINT || sig == SIGTERM) && !circle.pending_terminate) {
		/* Use SIGALRM to break out from potential busy loops that
		 * would not allow the program to be killed. */
		circle.pending_terminate = 1;
		signal(SIGALRM, circle_handle_alarm);
		alarm(2);
	}

	circle.signaled++;
	for (i = 0; i < circle.signal_count; i++) {
		if (circle.signals[i].sig == sig) {
			circle.signals[i].signaled++;
			break;
		}
	}
}


static void circle_process_pending_signals(void)
{
	int i;

	if (circle.signaled == 0)
		return;
	circle.signaled = 0;

	if (circle.pending_terminate) {
		alarm(0);
		circle.pending_terminate = 0;
	}

	for (i = 0; i < circle.signal_count; i++) {
		if (circle.signals[i].signaled) {
			circle.signals[i].signaled = 0;
			circle.signals[i].handler(circle.signals[i].sig,
						 circle.user_data,
						 circle.signals[i].user_data);
		}
	}
}


int circle_register_signal(int sig, circle_signal_handler handler,
			  void *user_data)
{
	struct circle_signal *tmp;

	tmp = (struct circle_signal *)
		os_realloc(circle.signals,
			   (circle.signal_count + 1) *
			   sizeof(struct circle_signal));
	if (tmp == NULL)
		return -1;

	tmp[circle.signal_count].sig = sig;
	tmp[circle.signal_count].user_data = user_data;
	tmp[circle.signal_count].handler = handler;
	tmp[circle.signal_count].signaled = 0;
	circle.signal_count++;
	circle.signals = tmp;
	signal(sig, circle_handle_signal);

	return 0;
}


int circle_register_signal_terminate(circle_signal_handler handler,
				    void *user_data)
{
	int ret = circle_register_signal(SIGINT, handler, user_data);
	if (ret == 0)
		ret = circle_register_signal(SIGTERM, handler, user_data);
	return ret;
}


int circle_register_signal_reconfig(circle_signal_handler handler,
				   void *user_data)
{
	return circle_register_signal(SIGHUP, handler, user_data);
}


void circle_run(void)
{
	fd_set *rfds, *wfds, *efds;
	int res;
	struct timeval _tv;
	struct timeval _tv2;
	struct os_time tv, now;
	
	rfds = os_malloc(sizeof(*rfds));
	wfds = os_malloc(sizeof(*wfds));
	efds = os_malloc(sizeof(*efds));
	if (rfds == NULL || wfds == NULL || efds == NULL) {
		npd_syslog_err("%s :malloc fail.\n",__func__);
		exit(1);
	}
	npd_syslog_dbg("start circle\n");

	while (!circle.terminate &&
	       (circle.timeout || circle.readers.count > 0 ||
		circle.writers.count > 0 || circle.exceptions.count > 0)) {
		if (circle.timeout) {
			os_get_time(&now);
			if (os_time_before(&now, &circle.timeout->time))
				os_time_sub(&circle.timeout->time, &now, &tv);
			else{
				tv.sec = 0;
				tv.usec = 0;
			}

			if (tv.sec < 1)
				npd_syslog_dbg("next timeout in %lu.%06lu sec\n",
			       	tv.sec, tv.usec);

			_tv.tv_sec = tv.sec;
			_tv.tv_usec = tv.usec;
		}
		_tv2.tv_sec = 3;
		_tv2.tv_usec = 0;
		circle_sock_table_set_fds(&circle.readers, rfds);
		circle_sock_table_set_fds(&circle.writers, wfds);
		circle_sock_table_set_fds(&circle.exceptions, efds);
		
		res = select(circle.max_sock + 1, rfds, wfds, efds,
			((circle.timeout) && (_tv.tv_sec < _tv2.tv_sec)) ? &_tv : &_tv2);
		if (res < 0 && errno != EINTR && errno != 0) {
			npd_syslog_err("%s select %s\n",__func__,strerror(errno));
			perror("select");
			//continue;
			//goto out;
		}
		circle_process_pending_signals();

		/* check if some registered timeouts have occurred */
		if (circle.timeout) {
			struct circle_timeout *tmp;

			os_get_time(&now);
			if (!os_time_before(&now, &circle.timeout->time)) {
				tmp = circle.timeout;
				circle.timeout = circle.timeout->next;
				tmp->handler(tmp->circle_data,
					     tmp->user_data);
				os_free(tmp);
			}

		}

		//if (res <= 0)
			//continue;

		circle_sock_table_dispatch(&circle.readers, rfds);
		circle_sock_table_dispatch(&circle.writers, wfds);
		circle_sock_table_dispatch(&circle.exceptions, efds);
	}
	npd_syslog_dbg("end circle\n");

out:
	os_free(rfds);
	os_free(wfds);
	os_free(efds);
}


void circle_terminate(void)
{
	circle.terminate = 1;
}


void circle_destroy(void)
{
	struct circle_timeout *timeout, *prev;
	struct os_time now;

	timeout = circle.timeout;
	if (timeout)
		os_get_time(&now);
	while (timeout != NULL) {
		int sec, usec;
		prev = timeout;
		timeout = timeout->next;
		sec = prev->time.sec - now.sec;
		usec = prev->time.usec - now.usec;
		if (prev->time.usec < now.usec) {
			sec--;
			usec += 1000000;
		}
		npd_syslog_dbg("circle: remaining timeout: %d.%06d circle_data=%p "
		       "user_data=%p handler=%p\n",
		       sec, usec, prev->circle_data, prev->user_data,
		       prev->handler);
		os_free(prev);
	}
	circle_sock_table_destroy(&circle.readers);
	circle_sock_table_destroy(&circle.writers);
	circle_sock_table_destroy(&circle.exceptions);
	os_free(circle.signals);
}


int circle_terminated(void)
{
	return circle.terminate;
}


void circle_wait_for_read_sock(int sock)
{
	fd_set rfds;

	if (sock < 0)
		return;

	FD_ZERO(&rfds);
	FD_SET(sock, &rfds);
	select(sock + 1, &rfds, NULL, NULL, NULL);
}


void * circle_get_user_data(void)
{
	return circle.user_data;
}

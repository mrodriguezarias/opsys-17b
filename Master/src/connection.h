#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <socket.h>

void connect_to_yama(void);

void request_job_for_file(const char *file);

t_socket connect_to_worker(const char *ip, const char *port);

#endif /* CONNECTION_H_ */

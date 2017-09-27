#ifndef MASTER_H_
#define MASTER_H_

#include <socket.h>

typedef struct {
	t_socket yama_socket;
	t_socket worker_socket;
} t_master;

extern t_master master;

void terminate(void);

#endif /* MASTER_H_ */

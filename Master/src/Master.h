#ifndef MASTER_H_
#define MASTER_H_

#include <socket.h>
#include <commons/string.h>
#include "funcionesMaster.h"

struct{
	int total;
	int transf;
	int reducc;
}tareasParalelo;

t_socket yama_socket;

time_t job_init;
time_t job_end;

void terminate(void);

#endif /* MASTER_H_ */

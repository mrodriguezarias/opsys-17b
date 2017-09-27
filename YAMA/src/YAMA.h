#ifndef YAMA_H_
#define YAMA_H_

#include <socket.h>

typedef struct {
	t_socket fs_socket;
} t_yama;

extern t_yama yama;

#endif /* YAMA_H_ */

#ifndef SERVER_H_
#define SERVER_H_

#include <yfile.h>
#include <socket.h>

enum { NODE_PING, NODE_SEND, NODE_RECV };

typedef struct {
	int opcode;
	int blockno;
	t_serial *block;
} t_nodeop;

t_socket yama_socket;

void server_start(void);

t_nodeop *server_nodeop(int opcode, int blockno, t_serial *block);

void server_set_current_file(t_yfile *file);

void server_end(void);

#endif /* SERVER_H_ */

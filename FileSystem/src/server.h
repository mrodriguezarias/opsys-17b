#ifndef SERVER_H_
#define SERVER_H_

#include <yfile.h>
#include <socket.h>

enum { NODE_PING, NODE_SEND,NODE_RECV_BLOCK, NODE_RECV };

typedef struct {
	int opcode;
	int blockno;
	void *block;
} t_nodeop;

t_socket yama_socket;

void server(void);

t_nodeop *server_nodeop(int opcode, int blockno, void *block);

#endif /* SERVER_H_ */

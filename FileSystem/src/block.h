#ifndef BLOCK_H_
#define BLOCK_H_

#include <stdbool.h>
#include <socket.h>
#include <yfile.h>

void block_request(int blockno, bool send, t_socket socket);

void block_print(t_block *block);

#endif /* BLOCK_H_ */

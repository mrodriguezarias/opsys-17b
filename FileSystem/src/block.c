#include "block.h"

#include <serial.h>
#include <protocol.h>

void block_request(int blockno, bool send, t_socket socket) {
	t_serial *serial = serial_pack("ii", blockno, send);
	t_packet packet = protocol_packet(OP_REQUEST_BLOCK, serial);
	protocol_send_packet(packet, socket);
	serial_destroy(serial);
}

void block_print(t_block *block) {
	printf("Block size: %d\n", block->size);
	printf("First copy: block #%d of node %s\n", block->copies[0].blockno, block->copies[0].node);
	printf("Second copy: block #%d of node %s\n", block->copies[1].blockno, block->copies[1].node);
}

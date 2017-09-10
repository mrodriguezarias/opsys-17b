#include "protocol.h"
#include "serial.h"
#include "socket.h"
#include <stdlib.h>
#include <string.h>

static const size_t HEADER_SIZE = sizeof(t_process) + sizeof(t_operation) + sizeof(size_t);

t_packet protocol_packet(t_operation operation, size_t size, char *content) {
	t_packet packet;
	packet.sender = process_current();
	packet.operation = operation;
	packet.size = size;
	packet.content = content;
	return packet;
}

void protocol_send(t_packet packet, t_socket socket) {
	char *header = malloc(HEADER_SIZE);
	serial_pack(header, "iii", packet.sender, packet.operation, packet.size);
	socket_send_bytes(socket, header, HEADER_SIZE);
	free(header);

	if(packet.size > 0 && packet.content != NULL) {
		socket_send_bytes(socket, packet.content, packet.size);
	}
}

t_packet protocol_receive(t_socket socket) {
	t_packet packet;
	memset(&packet, 0, sizeof packet);

	char *header = malloc(HEADER_SIZE);
	if(socket_receive_bytes(socket, header, HEADER_SIZE)) {
		serial_unpack(header, "iii", &packet.sender, &packet.operation, &packet.size);
	}
	free(header);

	if(packet.operation != OP_UNDEFINED && packet.size > 0) {
		packet.content = malloc(packet.size);
		socket_receive_bytes(socket, packet.content, packet.size);
	}

	return packet;
}

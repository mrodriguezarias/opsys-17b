#include "protocol.h"
#include "serial.h"
#include "socket.h"
#include <stdlib.h>
#include <string.h>

static const size_t HEADER_SIZE = sizeof(t_process) + sizeof(t_operation) + sizeof(size_t);

t_packet protocol_packet(t_operation operation, t_serial content) {
	t_packet packet;
	packet.sender = process_current();
	packet.operation = operation;
	packet.content = content;
	return packet;
}

void protocol_send(t_packet packet, t_socket socket) {
	t_serial header = serial_pack("iii", packet.sender, packet.operation, packet.content.size);
	socket_send_bytes(socket, header.data, header.size);
	free(header.data);

	if(packet.content.size > 0) {
		socket_send_bytes(socket, packet.content.data, packet.content.size);
	}
}

t_packet protocol_receive(t_socket socket) {
	t_packet packet;
	memset(&packet, 0, sizeof packet);

	t_serial header = {malloc(HEADER_SIZE), HEADER_SIZE};
	if(socket_receive_bytes(socket, header.data, header.size)) {
		serial_unpack(header, "iii", &packet.sender, &packet.operation, &packet.content.size);
	}
	free(header.data);

	if(packet.operation != OP_UNDEFINED && packet.content.size > 0) {
		packet.content.data = malloc(packet.content.size);
		socket_receive_bytes(socket, packet.content.data, packet.content.size);
	}

	return packet;
}

void protocol_handshake(t_socket socket) {
	protocol_send(protocol_packet(OP_HANDSHAKE, (t_serial){NULL, 0}), socket);
//	char saludo[19];
//	recv(socket,&saludo,19,0);
}

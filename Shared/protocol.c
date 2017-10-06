#include "protocol.h"
#include "serial.h"
#include "socket.h"
#include <stdlib.h>
#include <string.h>

static const size_t HEADER_SIZE = sizeof(t_process) + sizeof(t_operation) + sizeof(size_t);

t_packet protocol_packet(t_operation operation, t_serial *content) {
	t_packet packet;
	packet.sender = process_current();
	packet.operation = operation;
	packet.content = content;
	return packet;
}

bool protocol_send_packet(t_packet packet, t_socket socket) {
	size_t packet_size = packet.content == NULL ? 0 : packet.content->size;
	packet.sender = process_current();
	t_serial *header = serial_pack("iii", packet.sender, packet.operation, packet_size);
	size_t bytes = socket_send_bytes(socket, header->data, header->size);
	serial_destroy(header);
	if(bytes != header->size) return false;

	if(packet_size > 0) {
		bytes = socket_send_bytes(socket, packet.content->data, packet_size);
		if(bytes != packet_size) return false;
	}
	return true;
}

t_packet protocol_receive_packet(t_socket socket) {
	t_packet packet;
	memset(&packet, 0, sizeof packet);
	packet.content = serial_create(NULL, 0);

	t_serial *header = serial_create(malloc(HEADER_SIZE), HEADER_SIZE);
	if(socket_receive_bytes(socket, header->data, header->size)) {
		serial_unpack(header, "iii", &packet.sender, &packet.operation, &packet.content->size);
	}
	serial_destroy(header);

	if(packet.operation != OP_UNDEFINED && packet.content->size > 0) {
		packet.content->data = malloc(packet.content->size);
		socket_receive_bytes(socket, packet.content->data, packet.content->size);
	}

	return packet;
}

void protocol_send_handshake(t_socket socket) {
	protocol_send_packet(protocol_packet(OP_HANDSHAKE, NULL), socket);
}

bool protocol_receive_handshake(t_socket socket, t_process process) {
	t_packet packet = protocol_receive_packet(socket);
	bool result = packet.operation == OP_HANDSHAKE && packet.sender == process;
	serial_destroy(packet.content);
	return result;
}

void protocol_send_response(t_socket socket, int code) {
	t_serial *serial = serial_pack("i", code);
	protocol_send_packet(protocol_packet(OP_RESPONSE, serial), socket);
	serial_destroy(serial);
}

int protocol_receive_response(t_socket socket) {
	t_packet packet = protocol_receive_packet(socket);
	int code = -1;
	if(packet.operation == OP_RESPONSE) {
		serial_unpack(packet.content, "i", &code);
	}
	serial_destroy(packet.content);
	return code;
}

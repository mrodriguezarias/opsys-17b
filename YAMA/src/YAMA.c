#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "socket.h"
#include "serial.h"

#define IP "127.0.0.1"
#define PORT "3790"
#define MAXSIZE 1024

struct data {
	unsigned d;
	short h;
	double f;
	char s[16];
};

int main(void) {
	char buffer[MAXSIZE];
	struct data m;
	m.d = 782915742;
	m.h = -21409;
	m.f = 458691.306;
	strcpy(m.s, "teststring");

	puts("Hi, I'm a client.");

	puts("Connecting...");
	socket_t socket = socket_connect(IP, PORT);
	puts("Connected.");

	puts("Sending message...");
	size_t size = serial_pack(buffer, "Ihds", m.d, m.h, m.f, m.s);
	size_t n = socket_send_bytes(socket, buffer, size);

	printf("Sent %d bytes.\n", n);
	return EXIT_SUCCESS;
}

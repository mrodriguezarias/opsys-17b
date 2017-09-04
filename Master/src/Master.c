#include <stdio.h>
#include <stdlib.h>
#include "socket.h"
#include "serial.h"

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

	puts("Hello, I am a server.");

	puts("Listening for connections...");
	socket_t socket = socket_listen(PORT);
	puts("Connected.");

	puts("Receiving message...");
	size_t n = socket_receive_bytes(socket, buffer, sizeof(struct data));
	serial_unpack(buffer, "Ihds", &m.d, &m.h, &m.f, m.s);

	printf("Received %d bytes:\n", n);
	printf("m.d = %u\n", m.d);
	printf("m.h = %hd\n", m.h);
	printf("m.f = %lf\n", m.f);
	printf("m.s = %s\n", m.s);

	return EXIT_SUCCESS;
}

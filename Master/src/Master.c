#include <config.h>
#include <log.h>
#include <process.h>
#include <protocol.h>
#include <socket.h>
#include <stdlib.h>

// Esto es temporal, para probar la conexión con el Worker.
#define WORKER_IP "127.0.0.1"
#define WORKER_PORT "5050"

struct {
	t_socket yama_socket;
	t_socket worker_socket;
} master;

void connect_to_yama(void);
void connect_to_worker(void);
void terminate(void);

int main(int argc,char*argv[]) {
	process_init(PROC_MASTER);
	connect_to_yama();

	// Probando conexión
	socket_send_string(master.yama_socket, "Hi there, I'm the Master.");

	connect_to_worker();

	terminate();
	return EXIT_SUCCESS;
}

void connect_to_yama() {
	const char *ip = config_get("YAMA_IP");
	const char *port = config_get("YAMA_PUERTO");

	t_socket socket = socket_connect(ip, port);
	if(socket == -1) {
		logep("YAMA no está corriendo en %s:%s", ip, port);
		exit(EXIT_FAILURE);
	}

	protocol_handshake(socket);
	logif("Conectado a YAMA en %s:%s por el socket %i", ip, port, socket);
	master.yama_socket = socket;
}

void connect_to_worker() {
	t_socket socket = socket_connect(WORKER_IP, WORKER_PORT);
	if(socket == -1) {
		logep("Worker no está corriendo en %s:%s", WORKER_IP, WORKER_PORT);
		exit(EXIT_FAILURE);
	}

	protocol_handshake(socket);
	logif("Conectado a Worker en %s:%s por el socket %i", WORKER_IP, WORKER_PORT, socket);
	master.worker_socket = socket;
}

void terminate() {
	socket_close(master.yama_socket);
	socket_close(master.worker_socket);
}

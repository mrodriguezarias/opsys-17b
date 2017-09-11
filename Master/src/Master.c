#include <config.h>
#include <log.h>
#include <process.h>
#include <protocol.h>
#include <socket.h>
#include <stdlib.h>
#include <file.h>

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
//	connect_to_worker();

	// Prueba de apertura y propagación de archivo.
	// Créense un archivo test.txt en ~/yatpos para probarlo.

	const char *fname = "test.txt";
	printf("Abriendo archivo %s...", fname);
	t_file *f = file_open(fname);
	printf(" ok\n");

	printf("Leyendo archivo %s...", file_path(f));
	char *lines = file_readlines(f);
	file_close(f);
	printf(" ok\n");
	char *size = file_sizep(fname);
	printf("Contenido del archivo (%s):\n", size);
	free(size);
	printf("%s\n", lines);

	printf("Enviando archivo a YAMA...");
	socket_send_string(master.yama_socket, lines);
	printf(" ok\n");

//	printf("Enviando archivo a Worker...");
//	socket_send_string(master.worker_socket, lines);
//	printf(" ok\n");

	free(lines);

	terminate();
	return EXIT_SUCCESS;
}

void connect_to_yama() {
	const char *ip = config_get("YAMA_IP");
	const char *port = config_get("YAMA_PUERTO");

	t_socket socket = socket_connect(ip, port);
	if(socket == -1) {
		log_report("YAMA no está corriendo en %s:%s", ip, port);
		exit(EXIT_FAILURE);
	}

	protocol_handshake(socket);
	log_inform("Conectado a YAMA en %s:%s por el socket %i", ip, port, socket);
	master.yama_socket = socket;
}

void connect_to_worker() {
	t_socket socket = socket_connect(WORKER_IP, WORKER_PORT);
	if(socket == -1) {
		log_report("Worker no está corriendo en %s:%s", WORKER_IP, WORKER_PORT);
		exit(EXIT_FAILURE);
	}

	protocol_handshake(socket);
	log_inform("Conectado a Worker en %s:%s por el socket %i", WORKER_IP, WORKER_PORT, socket);
	master.worker_socket = socket;
}

void terminate() {
	socket_close(master.yama_socket);
	socket_close(master.worker_socket);
}

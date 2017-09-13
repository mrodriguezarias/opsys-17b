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

	t_packet * paquete;	//paquete recivido desde llama
	t_packet packet;

	do{
		packet = protocol_receive(master.yama_socket);
		manejador_yama(packet);

	}
	while(true);







//	printf("Enviando archivo a Worker...");
//	socket_send_string(master.worker_socket, lines);
//	printf(" ok\n");



	free(lines);






	terminate();
	return EXIT_SUCCESS;
}



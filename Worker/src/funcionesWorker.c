#include "funcionesWorker.h"


void mostrar_configuracion(){
	printf("IP_FILESYSTEM: %s\n",config_get("IP_FILESYSTEM"));
	printf("PUERTO_FILESYSTEM: %s\n",config_get("PUERTO_FILESYSTEM"));
	printf("NOMBRE_NODO: %s\n",config_get("NOMBRE_NODO"));
	printf("PUERTO_WORKER: %s\n",config_get("PUERTO_WORKER"));
	printf("PUERTO_DATANODE: %s\n",config_get("PUERTO_DATANODE"));
	printf("RUTA_DATABIN: %s\n",config_get("RUTA_DATABIN"));
}



void listen_to_master() {
	t_socket socketEscucha = socket_init(NULL, config_get("PUERTO_WORKER"));
	t_fdset socket_set = socket_set_create();
	socket_set_add(socketEscucha, &socket_set);

	while (true) {
		t_fdset selected = socket_select(socket_set);

		for (t_socket socketFor = 3; socketFor <= socket_set.max; socketFor++) {
			if (!socket_set_contains(socketFor, &selected))continue;
			if (socketFor == socketEscucha) {
				t_socket cli_sock = socket_accept(socketEscucha);
				t_packet packet = protocol_receive(cli_sock);
				printf("Socket sock: %d\n", socketFor);
				if (packet.operation == OP_HANDSHAKE && packet.sender == PROC_MASTER) {
					printf("Conectado proceso Master por socket %d\n",cli_sock);
					socket_set_add(cli_sock, &socket_set);
					log_inform("Conectado proceso Master por socket %i",cli_sock);
					printf("Socket sock: %d\n", socketFor);
					socket_send_string(cli_sock, "Bienvenido a Worker!");
				} else {
					printf("El proceso que requirio acceso, no posee los permisos adecuados\n");
					log_inform("El proceso que requirio acceso, no posee los permisos adecuados\n");
					socket_send_string(cli_sock,"No posee los permisos adecuados");
					socket_close(cli_sock);
				}
			}


			t_packet packet = protocol_receive(socketFor);
			switch (packet.operation) {
			case OP_INICIAR_TRANSFORMACION:
					manejador_fork();
				printf("Socket sock: %d\n", socketFor);
				break;
			default:
				break;
			}
		}

	}

}


void manejador_fork() {
	pid_t pid;
	if ((pid =fork()) == 0) {
		/* Lógica del proceso HIJO */
		printf("Hola soy el hijo de pid %d \n",pid);
		//exit(0);
	} else {
		/* Lógica del proceso PADRE*/
		printf("Hola soy el proceso padre de pid  %d \n",pid);
	}
}

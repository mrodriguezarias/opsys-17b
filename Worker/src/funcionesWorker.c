#include "funcionesWorker.h"


void listen_to_master() {
	log_print("Escuchando puertos de master");
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
					socket_set_add(cli_sock, &socket_set);
					log_print("Conectado proceso Master por socket %i",cli_sock);
					socket_send_string(cli_sock, "Bienvenido a Worker!");
				} else {
					log_print("El proceso que requirio acceso, no posee los permisos adecuados\n");
					socket_send_string(cli_sock,"No posee los permisos adecuados");
					socket_close(cli_sock);
				}
			}else{
				t_packet packet = protocol_receive(socketFor);
				manejador_fork(packet, socketFor);
			}

		}

	}

}

void recibir_script(int socket,char * nombre_script){
	char buffer[BUFSIZ];
	FILE *script_transformador;
	ssize_t len;
	int file_size;
	int stat;
	if((stat =recv(socket, buffer, BUFSIZ, 0))<0){
		log_report("Error al recibir el tamanio del archivo");
	}
	file_size = atoi(buffer);
	script_transformador = fopen(nombre_script, "w");
	if (script_transformador == NULL) {
		log_report("No se pudo recivir el script");
	}
	if(((len = recv(socket, buffer, BUFSIZ, 0)) < 0)){
		log_report("Error al recivir el archivo");
	}
	fwrite(buffer, sizeof(char), len, script_transformador);
	fclose(script_transformador);
}

void manejador_fork(t_packet packet,int socketFor){
	log_print("Manejador fork");
	tEtapaTransformacionBis et;

	serial_unpack(packet.content,"sii",
			&et.et.archivo_etapa,
			&et.et.bloque,
			&et.et.bytes_ocupados);

//	printf("archivo etapa: %s\n"
//					"bloque:%d\n"
//					"bytes_ocupados: %d \n",
//					et.et.archivo_etapa,
//					et.et.bloque,
//					et.et.bytes_ocupados
//					);
	socket_send_string(socketFor,"Ya podés mandar el script Master");
	recibir_script(socketFor,"transformador.sh");

	int pipe_padreAHijo[2];
	int pipe_hijoAPadre[2];
	int status;
	char * buffer = malloc(1024);
	pid_t pid;
	tEtapaReduccionLocal rl;
	tEtapaReduccionGlobal rg;

	pipe(pipe_padreAHijo);
	pipe(pipe_hijoAPadre);

	if ((pid =fork()) == 0) {
		/* Lógica del proceso HIJO */
		log_print("Proceso hijo: %d",pid);
		switch (packet.operation) {
		case OP_INICIAR_TRANSFORMACION:
			log_print("OP_INICIAR_TRANSFORMACION (HIJO-%d)",pid);
			dup2(pipe_padreAHijo[0],STDIN_FILENO);
			dup2(pipe_hijoAPadre[1],STDOUT_FILENO);
			close(pipe_padreAHijo[1]);
			close(pipe_hijoAPadre[0]);
			close(pipe_hijoAPadre[1]);
			close(pipe_padreAHijo[0]);
			char *argv[] = { NULL };
			char *envp[] = { NULL };
			execve("transformador.sh", argv, envp);
			exit(1);
			//exit(pid);
			break;
		case OP_INICIAR_REDUCCION_LOCAL:
			log_print("OP_INICIAR_REDUCCION_LOCAL (HIJO-%d)",pid);
			//exit(pid);
			break;
		case OP_INICIAR_REDUCCION_GLOBAL:
			log_print("OP_INICIAR_REDUCCION_GLOBAL (HIJO-%d)",pid);
			//exit(pid);
			break;
		case OP_INICIAR_ALMACENAMIENTO:
			log_print("OP_INICIAR_ALMACENAMIENTO (HIJO-%d)",pid);
			//exit(pid);
			break;
		default:
			break;
		}
	} else {
		/* Lógica del proceso PADRE*/
		switch (packet.operation) {
		case OP_INICIAR_TRANSFORMACION:
			log_print("Proceso padre de pid:%d",pid);
			close( pipe_padreAHijo[0] ); //Lado de lectura de lo que el padre le pasa al hijo.
			close( pipe_hijoAPadre[1] ); //Lado de escritura de lo que hijo le pasa al padre.
			write( pipe_padreAHijo[1],"hola pepe",strlen("hola pepe"));
			close( pipe_padreAHijo[1]);
			waitpid(pid,&status,0);
			read( pipe_hijoAPadre[0], buffer, 1024 );
			close(pipe_hijoAPadre[0]);
			FILE* fd = fopen(et.et.archivo_etapa, "w");
			fputs(buffer, fd);
			fclose(fd);

			free(buffer);
			break;
		case OP_INICIAR_REDUCCION_LOCAL:
			break;
		case OP_INICIAR_REDUCCION_GLOBAL:
				break;
		case OP_INICIAR_ALMACENAMIENTO:
				break;
		default:
			break;
		}
	}
}

#include "funcionesWorker.h"

void listen_to_master() {
	log_print("Escuchando puertos de master");
	t_socket socketEscucha = socket_init(NULL, config_get("PUERTO_WORKER"));
	t_fdset socket_set = socket_set_create();
	socket_set_add(socketEscucha, &socket_set);

	while (true) {
		t_fdset selected = socket_select(socket_set);

		for (t_socket socketFor = 3; socketFor <= socket_set.max; socketFor++) {
			if (!socket_set_contains(socketFor, &selected))
				continue;
			if (socketFor == socketEscucha) {
				t_socket cli_sock = socket_accept(socketEscucha);
				t_packet packet = protocol_receive(cli_sock);

				if (packet.operation == OP_HANDSHAKE
						&& packet.sender == PROC_MASTER) {
					socket_set_add(cli_sock, &socket_set);
					log_print("Conectado proceso Master por socket %i",
							cli_sock);
					socket_send_string(cli_sock, "Bienvenido a Worker!");
				} else {
					log_print(
							"El proceso que requirio acceso, no posee los permisos adecuados\n");
					socket_send_string(cli_sock,
							"No posee los permisos adecuados");
					socket_close(cli_sock);
				}
			} else {
				t_packet packet = protocol_receive(socketFor);
				manejador_fork2(packet, socketFor);
			}

		}

	}

}
#define MANDANDO_SCRIPT 1
#define MANDAR_SCRIPT 2

t_file * recibir_script(int socket, t_packet paquete) {
	char buffer[BUFSIZ];
	FILE *script;
	ssize_t len;
	size_t file_size;
	char mode[] = "0777";
	int stat, auxChmod;
	if ((stat = recv(socket, buffer, BUFSIZ, 0)) < 0) {
		log_report("Error al recibir el tamanio del archivo");
	}
	t_packet packet;
	packet.operation = MANDAR_SCRIPT;
	protocol_send(packet, socket);
	log_print("Tamanio del archivo recibido: %d", stat);
	file_size = atoi(buffer);
//	script = fopen(nombre_script, "w");
	t_file * script2;
	if (paquete.operation == OP_INICIAR_TRANSFORMACION)
		script2 = file_create("transformador_worker3.sh");
	else if (paquete.operation == OP_INICIAR_REDUCCION_LOCAL)
		script2 = file_create("reductor_worker.sh");
	file_open(file_path(script2));
//	if (script == NULL) {
//		log_report("No se pudo recivir el script");
//	}
	if (((len = recv(socket, buffer, BUFSIZ, 0)) < 0)) {
		log_report("Error al recivir el archivo");
	}
	log_print("Tamanio del archivo recibido: %d", len);
	fwrite(buffer, sizeof(char), len, file_pointer(script2));
	fclose(file_pointer(script2));
	auxChmod = strtol(mode, 0, 8);
	if (chmod(file_path(script2), auxChmod) < 0) {
		log_report("NO SE PUDO DAR PERMISOS DE EJECUCION AL ARCHIVO");
	}
	return script2;
}

//void manejador_fork(t_packet packet, int socketFor) {
//	log_print("Manejador fork");
//	tEtapaTransformacionBis et;
//	serial_unpack(packet.content, "sii", &et.et.archivo_etapa, &et.et.bloque,
//			&et.et.bytes_ocupados);
//
//	char *buffer;
//	socket_send_string(socketFor, "Ya podés mandar el script Master");
//	recibir_script(socketFor, packet);
//
//	int pipe_padreAHijo[2];
//	int pipe_hijoAPadre[2];
//	int status;
//	pid_t pid;
//	tEtapaReduccionLocal rl;
//	tEtapaReduccionGlobal rg;
//
//	pipe(pipe_padreAHijo);
//	pipe(pipe_hijoAPadre);
//
//	if ((pid = fork()) == 0) {
//		/* Lógica del proceso HIJO */
//		log_print("Proceso hijo: %d", pid);
//		switch (packet.operation) {
//		case OP_INICIAR_TRANSFORMACION:
//			log_print("OP_INICIAR_TRANSFORMACION (HIJO-%d)", pid);
//			dup2(pipe_padreAHijo[0], STDIN_FILENO);
//			dup2(pipe_hijoAPadre[1], STDOUT_FILENO);
//			close(pipe_padreAHijo[1]);
//			close(pipe_hijoAPadre[0]);
//			close(pipe_hijoAPadre[1]);
//			close(pipe_padreAHijo[0]);
//			char *argv[] = { NULL };
//			char *envp[] = { NULL };
//
//			execlp("transformador.sh", "/home/utnso/yatpos/WBAN.csv", envp);
//
//			//exit(pid);
//			break;
//		case OP_INICIAR_REDUCCION_LOCAL:
//			log_print("OP_INICIAR_REDUCCION_LOCAL (HIJO-%d)", pid);
//			//exit(pid);
//			break;
//		case OP_INICIAR_REDUCCION_GLOBAL:
//			log_print("OP_INICIAR_REDUCCION_GLOBAL (HIJO-%d)", pid);
//			//exit(pid);
//			break;
//		case OP_INICIAR_ALMACENAMIENTO:
//			log_print("OP_INICIAR_ALMACENAMIENTO (HIJO-%d)", pid);
//			//exit(pid);
//			break;
//		default:
//			break;
//		}
//	} else {
//		/* Lógica del proceso PADRE*/
//		switch (packet.operation) {
//		case OP_INICIAR_TRANSFORMACION:
//			log_print("Proceso padre de pid:%d", pid);
//			close(pipe_padreAHijo[0]); //Lado de lectuinicializarTablaDirectoriora de lo que el padre le pasa al hijo.
//			close(pipe_hijoAPadre[1]); //Lado de escritura de lo que hijo le pasa al padre.
//			write(pipe_padreAHijo[1], "hola pepe", strlen("hola pepe"));
//			close(pipe_padreAHijo[1]);
//			waitpid(pid, &status, 0);
//			read(pipe_hijoAPadre[0], buffer, 1024);
//			close(pipe_hijoAPadre[0]);
//			FILE* fd = fopen(et.et.archivo_etapa, "w");
//			fputs(buffer, fd);
//			fclose(fd);
//
//			free(buffer);
//			break;
//		case OP_INICIAR_REDUCCION_LOCAL:
//			break;
//		case OP_INICIAR_REDUCCION_GLOBAL:
//			break;
//		case OP_INICIAR_ALMACENAMIENTO:
//			break;
//		default:
//			break;
//		}
//	}
//}

void manejador_fork2(t_packet packet, int socketFor) {
	log_print("Manejador fork");
	tEtapaTransformacionBis et;
	serial_unpack(packet.content, "sii", &et.et.archivo_etapa, &et.et.bloque,
			&et.et.bytes_ocupados);

	socket_send_string(socketFor, "Ya podés mandar el script Master");
	t_file * script;
	char* buffer = malloc(SIZE);
	t_file * archivo_temporal = file_create(et.et.archivo_etapa);
	script = recibir_script(socketFor, packet);
	printf("Archivo temporal: %s\n", file_path(archivo_temporal));
	int pipe_padreAHijo[2];
	int pipe_hijoAPadre[2];
	int status;
	pid_t pid;
	tEtapaReduccionLocal rl;
	tEtapaReduccionGlobal rg;

	pipe(pipe_padreAHijo);
	pipe(pipe_hijoAPadre);

	if ((pid = fork()) == 0) {
		/* Lógica del proceso HIJO */
		log_print("Proceso hijo: %d", pid);
		switch (packet.operation) {
		case OP_INICIAR_TRANSFORMACION:
			dup2(LECTURA_PADRE, STDIN_FILENO);
			dup2(ESCRITURA_HIJO, STDOUT_FILENO);
			char path[64];
			close(ESCRITURA_PADRE);
			close(LECTURA_HIJO);
			close(ESCRITURA_HIJO);
			//	read(LECTURA_PADRE,path,SIZE);
			puts("Hola marolo");
			close(LECTURA_PADRE);
			char *argv[] = {NULL};
			char *envp[] = {NULL};
			execve(script, argv, envp);
			exit(1);
			break;
			case OP_INICIAR_REDUCCION_LOCAL:
			log_print("OP_INICIAR_REDUCCION_LOCAL (HIJO-%d)", pid);
			//exit(pid);
			break;
			case OP_INICIAR_REDUCCION_GLOBAL:
			log_print("OP_INICIAR_REDUCCION_GLOBAL (HIJO-%d)", pid);
			//exit(pid);
			break;
			case OP_INICIAR_ALMACENAMIENTO:
			log_print("OP_INICIAR_ALMACENAMIENTO (HIJO-%d)", pid);
			//exit(pid);
			break;
			default:
			break;
		}
	} else if(pid >0) {
		/* Lógica del proceso PADRE*/
		switch (packet.operation) {
			case OP_INICIAR_TRANSFORMACION:
			log_print("Proceso padre de pid:%d", pid);
			close( LECTURA_PADRE ); //Lado de lectura de lo que el padre le pasa al hijo.
			close( ESCRITURA_HIJO );//Lado de escritura de lo que hijo le pasa al padre.
			write(ESCRITURA_PADRE,file_path(script),file_size(script));
			FILE*file = fopen(archivo_temporal,"r");
			char buffer2[512];
			size_t elementosLeidos;
			while((elementosLeidos =fread(buffer2,sizeof(buffer2),1,file))>0)
				write( ESCRITURA_PADRE,buffer2,elementosLeidos);

			close( ESCRITURA_PADRE);
			waitpid(pid,&status,0);
			read( LECTURA_HIJO, buffer, SIZE );
			close( LECTURA_HIJO);
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
	} else if(pid < 0) {
		log_report("NO SE PUDO HACER EL FORK");
	}

	FILE* fd = fopen(file_path(archivo_temporal), "w");
//	FILE* fd = fopen("holo", "w");
	fputs(buffer, fd);
	fclose(fd);
	free(buffer);
}

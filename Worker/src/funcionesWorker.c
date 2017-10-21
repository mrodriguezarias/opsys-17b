#include "funcionesWorker.h"

void listen_to_master() {
	log_print("Escuchando puertos");
	socketEscuchaMaster = socket_init(NULL, config_get("PUERTO_WORKER"));
	socketEscuchaWorker = socket_init(NULL, config_get("PUERTO_WORKER"));
	t_fdset socket_set_read = socket_set_create(), socket_set_master =
			socket_set_create(); //No es un set especial para el proceso master, es la bolsa "MASTER"

	socket_set_add(socketEscuchaMaster, &socket_set_master);
	socket_set_add(socketEscuchaWorker, &socket_set_master);
	while (true) {
		socket_set_read = socket_set_master;
		t_fdset selected = socket_select(socket_set_master);

		for (t_socket socketFor = 3; socketFor <= socket_set_master.max;
				socketFor++) {
			if (!socket_set_contains(socketFor, &selected))
				continue;
			if (socketFor == socketEscuchaMaster) {
				t_socket cli_sock = socket_accept(socketEscuchaMaster);
				t_packet packet = protocol_receive_packet(cli_sock);

				if (packet.operation == OP_HANDSHAKE
						&& packet.sender == PROC_MASTER) {
					socket_set_add(cli_sock, &socket_set_master);
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
			} else if (socketFor < 0) {

			} else {
				t_packet packet = protocol_receive_packet(socketFor);
				printf("Valor del socket: %d\n", socketFor);
				//			manejador_fork2(packet, socketFor);
				manejador_master(&packet, socketFor);
			}

			if (socketFor == socketEscuchaWorker) {
				t_packet packet = protocol_receive_packet(socketFor);
				manejador_worker(&packet, socketFor);
			}

		}

	}

	FD_ZERO(&socket_set_read.set);
	FD_ZERO(&socket_set_master.set);
	close(socketEscuchaMaster);
	close(socketEscuchaWorker);
}

t_file* crearScript(char * bufferScript, int etapa) {
	int aux, auxChmod;
	char mode[] = "0777";
	t_file*script;
	aux = string_length(bufferScript);
	printf("size archivo:%d\n", aux);
	if (etapa == OP_INICIAR_TRANSFORMACION)
		script = file_create("transformador.sh");
	else
		script = file_create("reductor.sh");
	file_open(file_path(script));
	fwrite(bufferScript, sizeof(char), aux, file_pointer(script));
	auxChmod = strtol(mode, 0, 8);
	if (chmod(file_path(script), auxChmod) < 0) {
		log_report("NO SE PUDO DAR PERMISOS DE EJECUCION AL ARCHIVO");
	}
	fclose(file_pointer(script));

	return script;

}

tEtapaReduccionLocalWorker * etapa_rl_unpack_bis(t_serial * serial) {
	tEtapaReduccionLocalWorker * rl = malloc(
			sizeof(tEtapaReduccionLocalWorker));
	char * elemento;
	rl->archivosTemporales = mlist_create();
	serial_remove(serial, "s", &rl->script); //En realidad no es nodo, pero como nodo es (char*) aprobecho para almacenar el script
	serial_remove(serial, "i", &rl->lenLista);
	for (int i = 0; i < rl->lenLista; i++) {
		serial_remove(serial, "s", &elemento);
		mlist_append(rl->archivosTemporales, elemento);
	}
	serial_remove(serial, "s", &rl->archivoTemporal);
	return rl;
}

tEtapaReduccionGlobalWorker * rg_unpack(t_serial * serial) {
	tEtapaReduccionGlobalWorker * rg_w = malloc(
			sizeof(tEtapaReduccionGlobalWorker));
	rg_w->datosWorker = mlist_create();

	serial_remove(serial, "s", &rg_w->scriptReduccion);
	serial_remove(serial, "i", &rg_w->lenLista);

	for (int i = 0; i < rg_w->lenLista; i++) {
		tEtapaReduccionGlobal *rg = malloc(sizeof(tEtapaReduccionGlobal));
		serial_remove(serial, "ssss", &rg->nodo, &rg->ip, &rg->puerto,
				&rg->archivo_temporal_de_rl);
		mlist_append(rg_w->datosWorker, rg);
	}

	serial_remove(serial, "s", &rg_w->archivoEtapa);

	return rg_w;
}

void manejador_master(t_packet* packet, int socketFor) {
	log_print("Manejador Master");
	char * bufferScript, *archivoEtapa, *archivoPreReduccion = "preReduccion";
	int bloque = 6, bytesOcupados = 105615, status;
	int offset = 0;
	char * command;
	char * rutaDatabin;
	t_file*scriptTransformacion, *scriptReduccion;
	tEtapaReduccionLocalWorker* rl;
	tEtapaReduccionGlobalWorker * rg;
	//	t_file * scriptAux;
//	scriptAux = file_create("/home/utnso/yama-test1/transformador.sh");
	const char * direccion = system_userdir();
	rutaDatabin = mstring_create("%s/%s", direccion,
			config_get("RUTA_DATABIN"));

	switch (packet->operation) {
	case OP_INICIAR_TRANSFORMACION:
		log_print("OP_INICIAR_TRANSFORMACION");
		serial_unpack(packet->content, "ssii", &bufferScript, &archivoEtapa,
				&bloque, &bytesOcupados);
		scriptTransformacion = crearScript(bufferScript,
				OP_INICIAR_TRANSFORMACION);
		free(bufferScript);
		offset = (bloque - 1) * BLOCK_SIZE + bytesOcupados;
		command = mstring_create(
				"head -c %d < %s | tail -c %d | sh %s | sort > %s%s", offset,
				rutaDatabin, bytesOcupados, file_path(scriptTransformacion),
				system_userdir(), archivoEtapa);
		if ((status = system(command)) < 0) {
			log_report(
					"NO SE PUDO EJECTUAR EL COMANDO EN SYSTEM, FALLA TRANSFORMACION");
			free(command);
			free(archivoEtapa);
			protocol_send_response(socketFor, -1);
			break;
		}
		free(command);
		free(archivoEtapa);
		log_print("Status:%d", status);
		protocol_send_response(socketFor, 1);
		break;
	case OP_INICIAR_REPLANIFICACION:
		log_print("OP_INICIAR_REPLANIFICACION");
		break;
	case OP_INICIAR_REDUCCION_LOCAL:
		log_print("OP_INICIAR_REDUCCION_LOCAL");
		rl = etapa_rl_unpack_bis(packet->content);
		scriptReduccion = crearScript(rl->script, OP_INICIAR_REDUCCION_LOCAL);
		path_merge(rl->archivosTemporales, archivoPreReduccion);
		command = mstring_create(" %s | sh %s > %s ", archivoPreReduccion,
				rl->script, rl->archivoTemporal);
		archivoTemporalDeReduccionLocal = mstring_create("%s",rl->archivoTemporal);
		system(command);
		if ((status = system(command)) < 0) {
			log_report(
					"NO SE PUDO EJECTUAR EL COMANDO EN SYSTEM, FALLA REDUCCION LOCAL");
			free(command);
			free(archivoEtapa);
			protocol_send_response(socketFor, -1);
			break;
		}
		free(command);
		free(archivoEtapa);
		log_print("Status:%d", status);
		protocol_send_response(socketFor, 1);
		break;
	case OP_INICIAR_REDUCCION_GLOBAL:
		log_print("OP_INICIAR_REDUCCION_GLOBAL");
		rg = rg_unpack(packet->content);

		path_merge(rg->datosWorker, rg->archivoEtapa);

		socketWorker = connect_to_worker(rg->rg->ip, rg->rg->nodo);
		mandarDatosAWorkerHomologo(rg->rg, socketFor);

		break;
	case OP_INICIAR_ALMACENAMIENTO:
		log_print("OP_INICIAR_ALMACENAMIENTO");
		break;
	default:
		break;
	}

}

void manejador_worker(t_packet * packet, int socketWorker) {

}
t_socket connect_to_worker(const char *ip, const char *port) { // La ip y el puerto son obtenidos mediante YAMA
	t_socket socket = socket_connect(ip, port);
	if (socket == -1) {
		log_report("Worker no está corriendo en %s:%s", ip, port);
		exit(EXIT_FAILURE);
	}

	protocol_send_handshake(socket);
	log_inform("Conectado a Worker en %s:%s por el socket %i", ip, port,
			socket);
	return socket;
}

void mandarDatosAWorkerHomologo(tEtapaReduccionGlobal * rg,int socket) {
	t_packet packet;

}

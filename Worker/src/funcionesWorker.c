#include "funcionesWorker.h"

t_file* crearScript(char * bufferScript, int etapa) {
	int aux, auxChmod;
	char mode[] = "0777";
	t_file*script;
	aux = string_length(bufferScript);
	if (etapa == OP_INICIAR_TRANSFORMACION)
		script = file_create("transformador.sh");
	else if (etapa == OP_INICIAR_REDUCCION_LOCAL)
		script = file_create("reductor.pl");
	else
		script = file_create("reductorGlobal.pl");
	file_open(file_path(script));
	fwrite(bufferScript, sizeof(char), aux, file_pointer(script));
	auxChmod = strtol(mode, 0, 8);
	if (chmod(file_path(script), auxChmod) < 0) {
		log_report("NO SE PUDO DAR PERMISOS DE EJECUCION AL ARCHIVO");
	}
	fclose(file_pointer(script));

	return script;

}

t_file * crearArchivo(char * bufferArchivo, char*nombreArchivo) {
	t_file * archivo = file_create(nombreArchivo);
	int aux = string_length(bufferArchivo);
	file_open(file_path(archivo));
	fwrite(bufferArchivo, sizeof(char), aux, file_pointer(archivo));
	fclose(file_pointer(archivo));
	return archivo;
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
		char * aux = mstring_create("%s%s", system_userdir(), elemento);
		mlist_append(rl->archivosTemporales, aux);
	}
	serial_remove(serial, "s", &rl->archivoTemporal);
	return rl;
}

tEtapaReduccionGlobalWorker * rg_unpack(t_serial * serial) {
	log_print("DESENPACANDO RG");
	tEtapaReduccionGlobalWorker * rg_w = malloc(
			sizeof(tEtapaReduccionGlobalWorker));
	rg_w->datosWorker = mlist_create();

	serial_remove(serial, "s", &rg_w->scriptReduccion);
	serial_remove(serial, "i", &rg_w->lenLista);

	for (int i = 0; i < rg_w->lenLista; i++) {
		tEtapaReduccionGlobal *rg = malloc(sizeof(tEtapaReduccionGlobal));
		serial_remove(serial, "ssss", &rg->nodo, &rg->ip, &rg->puerto,
				&rg->archivo_temporal_de_rl);
		printf("Archivo temporal_de_rl: %s\n",rg->archivo_temporal_de_rl);
		mlist_append(rg_w->datosWorker, rg);
	}

	serial_remove(serial, "s", &rg_w->archivoEtapa);

	return rg_w;
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

void asignarOffset(int * offset, int bloque, int bytesOcuapdos) {
	if (bloque == 0)
		*offset = bytesOcuapdos; // 0 * BLOCK_SIZE = 0 //Con saber los bytes ocuapdos alcanza
	else
		*offset = (bloque - 1) * BLOCK_SIZE + bytesOcuapdos;
}

void ejecutarComando(char * command, int socketAceptado) {
	int status;
	printf("COMANDO:%s\n", command);
	system(command);
	if ((status = system(command)) < 0) {
		log_report(
				"NO SE PUDO EJECTUAR EL COMANDO EN SYSTEM, FALLA REDUCCION LOCAL");
		free(command);
		protocol_send_response(socketAceptado, -1);
		exit(1);
	} else {
		log_print("Se manda respuesta correcta a master");
		protocol_send_response(socketAceptado, 1);
	}
}

char * crearListaParaReducir(tEtapaReduccionGlobalWorker * rg) {
	log_print("CREANDO RUTA");
	mlist_t * archivosAReducir = mlist_create();
	mlist_t * socketsAWorker = mlist_create();
	int socket;
	char * bufferArchivoTemporal;
	char * archivoAReducir ;
	if (rg->lenLista > 1) {
		printf("tamanio lista:%d\n",rg->lenLista);
		archivoAReducir = mstring_create("%s/%s",system_userdir(),"archivo");
		for (int i = 0; i < rg->lenLista; i++) {
			rg->rg = mlist_get(rg->datosWorker, i);
			if (!mstring_equal(rg->rg->ip, config_get("IP"))
					&& !mstring_equal(rg->rg->puerto,
							config_get("PUERTO_WORKER"))) {
				t_socket socketWorker = connect_to_worker(rg->rg->ip,
						rg->rg->puerto);
				socket = socketWorker;
				mlist_append(socketsAWorker, &socket);
				t_packet paquete;
				paquete.content = serial_pack("s",
						rg->rg->archivo_temporal_de_rl);
				paquete.operation = OP_MANDAR_ARCHIVO;
				protocol_send_packet(paquete, socketWorker);
				paquete = protocol_receive_packet(socketWorker);
				serial_unpack(paquete.content, "s", &bufferArchivoTemporal);
				t_file * archivo = crearArchivo(bufferArchivoTemporal,
						rg->rg->archivo_temporal_de_rl);
				mlist_append(archivosAReducir, (char *) file_path(archivo));

			}else{
				char * aux = mstring_create("%s%s",system_userdir(),rg->rg->archivo_temporal_de_rl);
				mlist_append(archivosAReducir, aux);
			}
	path_merge(archivosAReducir, archivoAReducir);
		}
	}else{
		log_print("ESTO SIGNIFICA QUE SOLO HAY 1 NODO LABURANDO");
		rg->rg = mlist_get(rg->datosWorker,0);
		archivoAReducir = mstring_create("%s%s",system_userdir(),rg->rg->archivo_temporal_de_rl);
		log_print("Archivo a reducir: %s\n",archivoAReducir);
		return archivoAReducir;
	}
	printf("Archivo a reducir: %s\n",archivoAReducir);
	free(bufferArchivoTemporal);
	return archivoAReducir;
}

tEtapaAlmacenamientoWorker * af_unpack(t_serial * serial) {
	log_print("DESAMPAQUETANDO AF");
	tEtapaAlmacenamientoWorker * af = malloc(sizeof af);
	serial_remove(serial, "ss", &af->archivoReduccion, &af->archivoFinal);
	log_print("Archivo de reduccion: %s , archivoFinal: %s \n",af->archivoReduccion,af->archivoFinal);
	return af;
}
int connect_to_filesystem() {
	t_socket socket = socket_connect(config_get("IP_FILESYSTEM"),
			config_get("PUERTO_FILESYSTEM"));
	protocol_send_handshake(socket);
	int response = protocol_receive_response(socket);
	if (response == RESPONSE_ERROR) {
		log_print("Conexión rechazada. El FileSystem no se encuentra estable.");
		socket_close(socket);
	} else {
		log_inform("Conectado a proceso FileSystem por socket %i", socket);
		socketFileSystem = socket;
	}
	return response;
}

void listen_to_master() {
	log_print("Escuchando puertos");
	socketEscuchaMaster = socket_init(NULL, config_get("PUERTO_WORKER"));
	t_socket socketAceptado;
	char * bufferScript, *archivoEtapa, *archivoPreReduccion = "preReduccion";

	char * command;
	char * rutaDatabin;
	tEtapaTransformacionWorker * trans;
	tEtapaReduccionLocalWorker* rl;
	tEtapaReduccionGlobalWorker * rg;
	tEtapaAlmacenamientoWorker * af;


	const char * direccion = system_userdir();
	rutaDatabin = mstring_create("%s/%s", direccion,
			config_get("RUTA_DATABIN"));
	while (true) {
		pid_t pid;
		socketAceptado = socket_accept(socketEscuchaMaster);
		printf("Socket Aceptado:%d\n", socketAceptado);
		if (protocol_receive_handshake(socketAceptado, PROC_MASTER)) {
			log_print("HANDSHAKE CON PROC_MASTER");
			if ((pid = fork()) == 0) {
				log_print("PROCESO_HIJO:%d", pid);
				t_packet packet = protocol_receive_packet(socketAceptado);
				switch (packet.operation) {
				case OP_INICIAR_TRANSFORMACION:
					log_print("OP_INICIAR_TRANSFORMACION");
					int offset = 0;
					printf("offset: %d\n",offset);
					t_file*scriptTransformacion;
					serial_unpack(packet.content, "ssii", &bufferScript,
							&archivoEtapa, &trans->bloque,
							&trans->bytesOcupados);
					scriptTransformacion = crearScript(bufferScript,
							OP_INICIAR_TRANSFORMACION);
					printf("bloque: %d, bytes: %d \n",trans->bloque,trans->bytesOcupados);
					free(bufferScript);
					if(trans->bloque == 0){
						offset = trans->bytesOcupados;
						command =
								mstring_create(
										"head -c %d < %s | sh %s | sort > %s%s",
										offset, rutaDatabin,
										file_path(scriptTransformacion),
										system_userdir(), archivoEtapa);
					}else{
						offset = trans->bloque * BLOCK_SIZE + trans->bytesOcupados;
					command =
							mstring_create(
									"head -c %d < %s | tail -c %d | sh %s | sort > %s%s",
									offset, rutaDatabin, trans->bytesOcupados,
									file_path(scriptTransformacion),
									system_userdir(), archivoEtapa);
					}
					printf("offset: %d\n",offset);
					log_print("COMMAND: %s",command);
					ejecutarComando(command, socketAceptado);
					exit(1);
					break;
				case OP_INICIAR_REDUCCION_LOCAL:
					log_print("OP_INICIAR_REDUCCION_LOCAL");
					rl = etapa_rl_unpack_bis(packet.content);
					t_file *scriptReduccion = crearScript(rl->script,
							OP_INICIAR_REDUCCION_LOCAL);
					char * aux = mstring_create("%s/%s", system_userdir(),
							archivoPreReduccion);
					t_file * archivo = file_create(aux);
					archivo = file_open(aux);
					path_merge(rl->archivosTemporales, file_path(archivo));
//					t_file *archivoTemporalDeReduccionLocal = file_create(rl->archivoTemporal);
					command = mstring_create(" cat %s | perl %s > %s%s ",
							file_path(archivo), file_path(scriptReduccion),
							system_userdir(), rl->archivoTemporal);
					ejecutarComando(command, socketAceptado);
					free(command);
					exit(1);
					break;
				case OP_INICIAR_REDUCCION_GLOBAL:
					log_print("OP_INICIAR_REDUCCION_GLOBAL");
					char *archivoAReducir;
					rg = rg_unpack(packet.content);
					t_file *scriptReduccionGlobal;
					scriptReduccionGlobal = crearScript(rg->scriptReduccion,
							OP_INICIAR_REDUCCION_GLOBAL);
					archivoAReducir = crearListaParaReducir(rg);
					command = mstring_create("cat %s | perl %s > %s%s",
							archivoAReducir, file_path(scriptReduccionGlobal),system_userdir(),
							rg->archivoEtapa);
					ejecutarComando(command, socketAceptado);
					free(command);
					exit(1);
					break;
				case OP_INICIAR_ALMACENAMIENTO:
					log_print("OP_INICIAR_ALMACENAMIENTO");
					af = af_unpack(packet.content);
					t_file * archivoReduccion;
					char * bufferArchivoReduccion;
					char * aux2 = mstring_create("%s%s",system_userdir(),af->archivoReduccion);
					printf("Archivo reduccion:%s\n",aux2);
					archivoReduccion = file_open(aux2);
					bufferArchivoReduccion = file_map(archivoReduccion);
					t_serial *serialFileSystem = serial_pack("ssi", bufferArchivoReduccion, af->archivoFinal, file_size(archivoReduccion));
					printf("file_size: %d\n", file_size(archivoReduccion));
					t_packet paquete = protocol_packet(
							OP_INICIAR_ALMACENAMIENTO, serialFileSystem);
					int response = connect_to_filesystem();
					if (response < 0) {
						log_report("No se pudo conectar al filesystem");
						protocol_send_response(socketAceptado, -1);
					} else {
						protocol_send_packet(paquete, socketFileSystem);
						int estado = protocol_receive_response(
								socketFileSystem);
						if (estado == RESPONSE_OK) {
							log_print("Se informa a Master el termino del job");
							protocol_send_response(socketAceptado, 1);
						} else {
							log_print("Se informa a Master la falla del job");
							protocol_send_response(socketAceptado, -1);
						}
					}
					exit(1);
					break;
				default:
					break;
				}
			} else if (pid > 0) {
				log_print("PROCESO_PADRE:%d", pid);
			} else if (pid < 0) {
				log_report("NO SE PUDO HACER EL FORK");
			}

		} else if (protocol_receive_handshake(socketAceptado, PROC_WORKER)) {
			log_print("Handshake de Worker Homologo");
			if ((pid = fork()) == 0) {
				log_print("Proceso hijo de worker homologo de pid: #%d", pid);
				t_packet paquete = protocol_receive_packet(socketAceptado);
				t_file * archivo;
				char * nombreDelArchivo;
				void *bufferArchivo;
				switch (paquete.operation) {
				case (OP_MANDAR_ARCHIVO): //OP_MANDAR_ARCHIVO
					serial_unpack(paquete.content, "s", &nombreDelArchivo);
					archivo = file_open(nombreDelArchivo);
					bufferArchivo = file_map(archivo);
					paquete.content = serial_pack("s", bufferArchivo);
					paquete.operation = OP_MANDAR_ARCHIVO;
					protocol_send_packet(paquete, socketAceptado);
					exit(1);
					break;
				default:
					log_report("OP_UNDIFINED");
					exit(1);
					break;
				}
			} else if (pid > 0) {
				log_print("PROCESO PADRE HOMOLOG");
			} else if (pid < 0) {
				log_report("NO SE PUDO CREAR EL PROCESO HIJO HOMOLOGO");
			}
		}
	}
}


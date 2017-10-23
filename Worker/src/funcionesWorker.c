#include "funcionesWorker.h"

t_file* crearScript(char * bufferScript, int etapa) {
	int aux, auxChmod;
	char mode[] = "0777";
	t_file*script;
	aux = string_length(bufferScript);
	printf("size archivo:%d\n", aux);
	if (etapa == OP_INICIAR_TRANSFORMACION)
		script = file_create("transformador.sh");
	else if(etapa == OP_INICIAR_REDUCCION_LOCAL)
		script = file_create("reductor.sh");
	else
		script = file_create("reductorGlobal.sh");
	file_open(file_path(script));
	fwrite(bufferScript, sizeof(char), aux, file_pointer(script));
	auxChmod = strtol(mode, 0, 8);
	if (chmod(file_path(script), auxChmod) < 0) {
		log_report("NO SE PUDO DAR PERMISOS DE EJECUCION AL ARCHIVO");
	}
	fclose(file_pointer(script));

	return script;

}

t_file * crearArchivo(char * bufferArchivo, char*nombreArchivo){
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


void asignarOffset(int * offset,int bloque,int bytesOcuapdos){
	if (bloque == 0)
		*offset = bytesOcuapdos; // 0 * BLOCK_SIZE = 0 //Con saber los bytes ocuapdos alcanza
	else
		*offset = (bloque - 1) * BLOCK_SIZE + bytesOcuapdos;
}

void ejecutarComando(char * command, int socketAceptado) {
	int status;
	system(command);
	if ((status = system(command)) < 0) {
		log_report("NO SE PUDO EJECTUAR EL COMANDO EN SYSTEM, FALLA REDUCCION LOCAL");
		free(command);
		protocol_send_response(socketAceptado, -1);
		exit(1);
	}
}

mlist_t * crearListaParaReducir(tEtapaReduccionGlobalWorker * rg) {
		mlist_t * archivosAReducir = mlist_create();
		mlist_t * socketsAWorker = mlist_create();
		int socket;
		char * bufferArchivoTemporal;
	for (int i = 0; i < rg->lenLista; i++) {
		rg->rg = mlist_get(rg->datosWorker, i);
		if (!mstring_equal(rg->rg->ip, config_get("IP"))
				&& !mstring_equal(rg->rg->puerto,
						config_get("PUERTO_WORKER"))) {
			t_socket socketWorker = connect_to_worker(rg->rg->ip,
					rg->rg->puerto);
			socket = socketWorker;
			mlist_append(socketsAWorker,&socket);
			t_packet paquete;
			paquete.content = serial_pack("s", rg->rg->archivo_temporal_de_rl);
			paquete.operation = 3;
			protocol_send_packet(paquete, socketWorker);
			paquete = protocol_receive_packet(socketWorker);
			serial_unpack(paquete.content, "s", &bufferArchivoTemporal);
			t_file * archivo = crearArchivo(bufferArchivoTemporal,
					rg->rg->archivo_temporal_de_rl);
			mlist_append(archivosAReducir, (char *) file_path(archivo));
		}
	}
	free(bufferArchivoTemporal);
	return archivosAReducir;
}

void listen_to_master() {
	log_print("Escuchando puertos");
	socketEscuchaMaster = socket_init(NULL, config_get("PUERTO_WORKER"));
	socketEscuchaWorker = socket_init(NULL, config_get("PUERTO_WORKER"));
	t_socket socketAceptado;
	char * bufferScript, *archivoEtapa, *archivoPreReduccion = "preReduccion";
	int bloque = 6, bytesOcupados = 105615, status;
	int offset = 0;
	char * command;
	char * rutaDatabin;
	t_file*scriptTransformacion, *scriptReduccion,*archivoTemporalDeReduccionLocal,*scriptReduccionGlobal;
	tEtapaReduccionLocalWorker* rl;
	tEtapaReduccionGlobalWorker * rg;
	mlist_t * archivosAReducir = mlist_create();
	const char * direccion = system_userdir();
	rutaDatabin = mstring_create("%s/%s", direccion,
			config_get("RUTA_DATABIN"));
	char * bufferArchivoTemporalLocal,*archivoAReducir;
	while (true) {
		pid_t pid;
		socketAceptado = socket_accept(socketEscuchaMaster);
		printf("Socket Aceptado:%d\n", socketAceptado);
		if (protocol_receive_handshake(socketAceptado, PROC_MASTER)) {
			if ((pid = fork()) == 0) {
				log_print("PROCESO_HIJO:%d", pid);
				t_packet packet = protocol_receive_packet(socketAceptado);
				switch (packet.operation) {
				case OP_INICIAR_TRANSFORMACION:
					log_print("OP_INICIAR_TRANSFORMACION");
					serial_unpack(packet.content, "ssii", &bufferScript,
							&archivoEtapa, &bloque, &bytesOcupados);
					scriptTransformacion = crearScript(bufferScript,OP_INICIAR_TRANSFORMACION);
					free(bufferScript);
					asignarOffset(&offset,bloque,bytesOcupados);
					command =	mstring_create(
									"head -c %d < %s | tail -c %d | sh %s | sort > %s%s",
									offset, rutaDatabin, bytesOcupados,
									file_path(scriptTransformacion),
									system_userdir(), archivoEtapa);
					ejecutarComando(command,socketAceptado);
					free(command);
					protocol_send_response(socketAceptado, 1);
					exit(1);
					break;
				case OP_INICIAR_REDUCCION_LOCAL:
					log_print("OP_INICIAR_REDUCCION_LOCAL");
					rl = etapa_rl_unpack_bis(packet.content);
					scriptReduccion = crearScript(rl->script,OP_INICIAR_REDUCCION_LOCAL);
					path_merge(rl->archivosTemporales, archivoPreReduccion);
				archivoTemporalDeReduccionLocal = file_create(rl->archivoTemporal);
					command = mstring_create(" %s | sh %s > %s ",archivoPreReduccion, rl->script,rl->archivoTemporal);
					free(command);
					protocol_send_response(socketAceptado, 1);
					exit(1);
					break;
				case OP_INICIAR_REDUCCION_GLOBAL:
					log_print("OP_INICIAR_REDUCCION_GLOBAL");
					rg = rg_unpack(packet.content);
					scriptReduccionGlobal = crearScript(rg->scriptReduccion,OP_INICIAR_REDUCCION_GLOBAL);
					archivosAReducir = crearListaParaReducir(rg);
					path_merge(archivosAReducir, archivoAReducir);
					command = mstring_create("%s | sh %s > %s",archivoAReducir,file_path(scriptReduccionGlobal),rg->archivoEtapa);
					ejecutarComando(command,socketAceptado);
					free(command);
					exit(1);
					break;
				case OP_INICIAR_ALMACENAMIENTO:
					log_print("OP_INICIAR_ALMACENAMIENTO");
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
			if((pid = fork()) == 0){
				t_packet paquete = protocol_receive_packet(socketAceptado);
				t_file * archivo;
				char * nombreDelArchivo;
				void *bufferArchivo;
				switch(paquete.operation){
				case(3)://OP_MANDAR_ARCHIVO
				serial_unpack(paquete.content,"s",&nombreDelArchivo);
				archivo = file_open(nombreDelArchivo);
				bufferArchivo = file_map(archivo);
				paquete.content = serial_pack("s",bufferArchivo); paquete.operation = 4;//OP_MANDAR_ARCHIVO
				protocol_send_packet(paquete,socketAceptado);
				exit(1);
				break;
				default:
					log_report("OP_UNDIFINED");
					exit(1);
					break;
				}
			}else if(pid >0){

			}else if(pid<0){

			}
		}
	}
}




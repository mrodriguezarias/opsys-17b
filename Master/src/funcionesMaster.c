#include "funcionesMaster.h"
#include "manejadores.h"
//t_log * logTrace;
//void crearLogger() {
//   char *pathLogger = string_new();
//
//   char cwd[1024];
//
//   string_append(&pathLogger, getcwd(cwd, sizeof(cwd)));
//
//   string_append(&pathLogger, "/Master_LOG.log");
//
//   char *logmaster = strdup("Master_LOG.log");
//
//   logTrace = log_create(pathLogger, logmaster, false, LOG_LEVEL_INFO);
//
//   free(pathLogger);
//   free(logmaster);
//}
tMaster *getConfigMaster() {
	printf("Ruta del archivo de configuracion: %s\n", RUTA_CONFIG);
	tMaster *masterAux = malloc(sizeof(tMaster));

	//t_config *masterConfig = config_create("/home/utnso/tp-2017-2c-YATPOS/master/src/config_master");

	t_config *masterConfig = config_create(RUTA_CONFIG);

	masterAux->YAMA_IP     = malloc(MAX_IP_LEN);
	masterAux->YAMA_PUERTO = malloc(MAX_PORT_LEN);
	masterAux->WORKER_IP     = malloc(MAX_IP_LEN);
	masterAux->PUERTO_WORKER       = malloc(MAX_PORT_LEN);

	strcpy(masterAux->YAMA_IP,
			config_get_string_value(masterConfig, "YAMA_IP"));
	strcpy(masterAux->YAMA_PUERTO,
			config_get_string_value(masterConfig, "YAMA_PUERTO"));
	strcpy(masterAux->WORKER_IP,
				config_get_string_value(masterConfig, "WORKER_IP"));
	strcpy(masterAux->PUERTO_WORKER,
			config_get_string_value(masterConfig, "PUERTO_WORKER"));

	config_destroy(masterConfig);
	return masterAux;
}

void mostrarConfiguracion(tMaster *master) {
	printf("YAMA_IP: %s\n", master->YAMA_IP);
	printf("YAMA_PUERTO: %s\n", master->YAMA_PUERTO);
	printf("WORKER_IP: %s\n", master->WORKER_IP);
	printf("PUERTO_WORKER: %s\n", master->PUERTO_WORKER);
}

void liberarConfiguracionMaster(tMaster*masterAux) {

	free(masterAux->YAMA_IP);
	masterAux->YAMA_IP = NULL;
	free(masterAux->YAMA_PUERTO);
	masterAux->YAMA_PUERTO = NULL;
	free(masterAux->YAMA_IP);
	masterAux->YAMA_IP = NULL;
	free(masterAux->PUERTO_WORKER);
	masterAux->PUERTO_WORKER = NULL;

}

void iniciar_master(tMaster * masterAux){
	//	crearLogger();
		masterAux = getConfigMaster();
		mostrarConfiguracion(masterAux);
		connect_to_yama(masterAux);
	}




void connect_to_yama() {
	const char *ip = config_get("YAMA_IP");
	const char *port = config_get("YAMA_PUERTO");

	t_socket socket = socket_connect(ip, port);
	if(socket == -1) {
		//logep("YAMA no está corriendo en %s:%s", ip, port);
		exit(EXIT_FAILURE);
	}

	protocol_handshake(socket);
	//logif("Conectado a YAMA en %s:%s por el socket %i", ip, port, socket);
	master.yama_socket = socket;
}

void connect_to_worker(const char* ip,const char * port) { //La ip y el puerto son obtenidos mediante YAMA

	t_socket socket = socket_connect(ip, port);
	if(socket == -1) {
		log_report("Worker no está corriendo en %s:%s", ip, port);
		exit(EXIT_FAILURE);
	}

	protocol_handshake(socket);
	log_inform("Conectado a Worker en %s:%s por el socket %i", ip, port, socket);
	master.worker_socket = socket;
}

void terminate() {
	socket_close(master.yama_socket);
	socket_close(master.worker_socket);
}





void liberarRecursos(tMaster * masterAux){
	liberarConfiguracionMaster(masterAux);
}

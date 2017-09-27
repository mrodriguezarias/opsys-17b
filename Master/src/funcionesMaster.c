#include "funcionesMaster.h"

#include <string.h>

#include "Master.h"

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

void mostrar_configuracion() {

	printf("YAMA_IP: %s\n",config_get("YAMA_IP"));
	printf("YAMA_PUERTO: %s\n",config_get("YAMA_PUERTO"));
	printf("PUERTO_WORKER: %s\n",config_get("WORKER_PUERTO"));

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
	masterAux = getConfigMaster();
	connect_to_yama(masterAux);
}


void liberarRecursos(tMaster * masterAux){
	liberarConfiguracionMaster(masterAux);
}

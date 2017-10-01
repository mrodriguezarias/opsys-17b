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

	masterAux->YAMA_IP = malloc(MAX_IP_LEN);
	masterAux->YAMA_PUERTO = malloc(MAX_PORT_LEN);
	masterAux->WORKER_IP = malloc(MAX_IP_LEN);
	masterAux->PUERTO_WORKER = malloc(MAX_PORT_LEN);

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

	printf("YAMA_IP: %s\n", config_get("YAMA_IP"));
	printf("YAMA_PUERTO: %s\n", config_get("YAMA_PUERTO"));
	printf("PUERTO_WORKER: %s\n", config_get("WORKER_PUERTO"));

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

void iniciar_master(tMaster * masterAux) {
	masterAux = getConfigMaster();
	connect_to_yama(masterAux);
}

void liberarRecursos(tMaster * masterAux) {
	liberarConfiguracionMaster(masterAux);
}

void mandar_script(int socket, char * ruta) {
	log_print("mandar_script");
	ssize_t len;
	int fd;
	int bytes_enviados = 0;
	char file_size[256];
	struct stat file_stat;
	int offset;
	int data_restante;
	fd = open(ruta, O_RDONLY);
	if (fd == -1) {
		log_report("No se pudo abrir el script de transformacion");
	}
	if (fstat(fd, &file_stat) < 0) {
		log_report("No se pudieron obtener los datos del script:%s", ruta);
	}
	len = send(master.worker_socket, file_size, sizeof(file_size), 0);
	if (len < 0) {
		log_report("No se pudo enviar el tamanio del script");
	}

	offset = 0;
	data_restante = file_stat.st_size;
//	while (((bytes_enviados = sendfile(master.worker_socket, fd, &offset, BUFSIZ))> 0) && (data_restante > 0)) {
//		                data_restante -= bytes_enviados;
//	}
	if(((bytes_enviados = sendfile(master.worker_socket, fd, &offset, BUFSIZ))< 0)){
		log_report("No se pudo mandar todo el archivo");
	}
}


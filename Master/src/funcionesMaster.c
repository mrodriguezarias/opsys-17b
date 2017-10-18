#include "funcionesMaster.h"

void cargar_scripts(char* path_transf, char* path_reduc) {

	 script.fd_transf = file_open(path_transf);
	 script.script_transf = file_map(script.fd_transf);

	 script.fd_reduc = file_open(path_reduc);
	 script.script_reduc = file_map(script.fd_transf);

}

void liberar_scripts() {

	file_unmap(script.fd_transf, script.script_transf);
	file_unmap(script.fd_reduc, script.script_reduc);

}

inline time_t get_current_time(){
	return time(NULL);
}

const char *datetime(time_t time){
	struct tm lt;
	localtime_r(&time, &lt);
	return string_from_format("%d-%02d-%02d %02d:%02d:%02d", lt.tm_year + 1900, lt.tm_mon + 1,
			lt.tm_mday, lt.tm_hour, lt.tm_min, lt.tm_sec);
}

const char *timediff(time_t t1, time_t t2){
	unsigned duration = abs((int) difftime(t1, t2));
	unsigned seconds = duration % 60;
	unsigned minutes = duration / 60;
	unsigned hours = duration / 60;
	return string_from_format("%02u:%02u:%02u", hours, minutes, seconds);
}

const char *timeprom(time_t t1, time_t t2){
	unsigned duration = (abs((int) difftime(t1, t2))) / 3;
	unsigned seconds = duration % 60;
	unsigned minutes = duration / 60;
	unsigned hours = duration / 60;
	return string_from_format("%02u:%02u:%02u", hours, minutes, seconds);
}

void verificarParalelismo(){
	bool statusEtapaTransformacion(t_hilos* hilo){
		return (hilo->etapa == TRANSFORMACION && hilo->active);
	}
	bool statusEtapaReduccionLocal(t_hilos* hilo){
		return (hilo->etapa == REDUCCION_LOCAL && hilo->active);
	}
	int totalEtapas = mlist_count(hilos, statusEtapaTransformacion) + mlist_count(hilos, statusEtapaReduccionLocal);
	if(mlist_count(hilos, statusEtapaTransformacion) > 0 &&
			mlist_count(hilos, statusEtapaReduccionLocal) > 0 &&
			totalEtapas > tareasParalelo.total){
		tareasParalelo.total = totalEtapas;
		tareasParalelo.transf = mlist_count(hilos, statusEtapaTransformacion);
		tareasParalelo.reducc = mlist_count(hilos, statusEtapaReduccionLocal);
	}
}
void calcular_metricas(){

	printf("Tiempo total de ejecución del Job: %s\n", timediff(job_init, job_end));
	printf("Tiempo promedio de ejecución de cada etapa principal: %s\n", timeprom(job_init, job_end));
	printf("Cantidad de etapas ejecutadas en paralelo: %d\n"
			"Transformaciones: %d\n"
			"Reducciones Locales: %d\n",
			tareasParalelo.total,
			tareasParalelo.transf,
			tareasParalelo.reducc);
	bool getTransformacion(t_hilos* hilo){
		return (hilo->etapa == TRANSFORMACION);
	}
	bool getReduccionLocal(t_hilos* hilo){
		return (hilo->etapa == REDUCCION_LOCAL);
	}
	bool getReduccionTotal(t_hilos* hilo){
		return (hilo->etapa == REDUCCION_GLOBAL || hilo->etapa == REDUCCION_LOCAL);
	}
	printf("Cantidad total de tareas realizadas:\n"
			"Transformaciones: %d\n"
			"Reducciones: %d\n"
			"Reducciones Locales: %d\n",
			mlist_count(hilos, getTransformacion),
			mlist_count(hilos, getReduccionTotal),
			mlist_count(hilos, getReduccionLocal));
	bool getFallos(t_hilos* hilo){
		return (hilo->result == -1);
	}
	printf("Cantidad de fallos del job: %d", mlist_count(hilos, getFallos));
}

void terminate() {
	thread_term();
	liberar_scripts();
	socket_close(yama_socket);
	calcular_metricas();
	void hilo_destroy(t_hilos *self){
		free(self->hilo);
		free(self);
	}
	mlist_destroy(hilos, hilo_destroy);
}

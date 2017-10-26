#include "struct.h"

/*
 * **************************************INICIO FUNCIONES
 * *************************************ETAPA TRANSFORMACION
 *
 * */
tEtapaTransformacion* new_etapa_transformacion(const char*nodo,const char*ip,const char*puerto,int bloque,int bytes_ocuapdos,const char * archivo_etapa){
	tEtapaTransformacion *et = malloc(sizeof(tEtapaTransformacion));
	et->bloque = bloque;
	et->bytes_ocupados = bytes_ocuapdos;
	et->nodo = string_duplicate((char*)nodo);
	et->ip = string_duplicate((char*)ip);
	et->puerto = string_duplicate((char*)puerto);
	et->archivo_etapa = string_duplicate((char*)archivo_etapa);

	return et;
}

t_serial *list_transformacion_pack(mlist_t* list) {
	t_serial *serial = serial_create(NULL, 0);
	serial_add(serial, "i", mlist_length(list));

	void routine(tEtapaTransformacion* transformacion) {
		serial_add(serial, "sssiis",
				transformacion->nodo,
				transformacion->ip,
				transformacion->puerto,
				transformacion->bloque,
				transformacion->bytes_ocupados,
				transformacion->archivo_etapa);
	}
	mlist_traverse(list, routine);
	return serial;
}

mlist_t *list_transformacion_unpack(t_serial *serial) {
	mlist_t *list = mlist_create();

	int numblocks;
	serial_remove(serial, "i", &numblocks);

	while(numblocks--) {
		tEtapaTransformacion *et = malloc(sizeof(tEtapaTransformacion));
		serial_remove(serial,"sssiis",&et->nodo,
				&et->ip,
				&et->puerto,
				&et->bloque,
				&et->bytes_ocupados,
				&et->archivo_etapa);
		mlist_append(list, et);
	}

	return list;
}


void mandar_etapa_transformacion(mlist_t* list,t_socket sock){
	t_serial *serial = list_transformacion_pack(list);
	t_packet paquete = protocol_packet(OP_INICIAR_TRANSFORMACION, serial);
	protocol_send_packet(paquete, sock);
	serial_destroy(serial);
}

/*
 * **************************************************************************************FIN FUNCIONES
 * **************************************************************************************ETAPA TRANSFORMACION
 * */

/*
 * **************************************INICIO FUNCIONES
 * *************************************ETAPA REDUCCION LOCAL
 *
 * */

tEtapaReduccionLocal *new_etapa_rl(const char* nodo,const char* ip, const char* puerto, mlist_t* archivos_temporales, const char* archivo_etapa){
	tEtapaReduccionLocal *erl = malloc(sizeof(tEtapaReduccionLocal));
	erl->nodo = string_duplicate((char*)nodo);
	erl->ip = string_duplicate((char*)ip);
	erl->puerto = string_duplicate((char*)puerto);
	erl->archivo_etapa = string_duplicate((char*)archivo_etapa);
	erl->archivos_temporales_de_transformacion = mlist_copy(archivos_temporales);
	return erl;
}

t_serial *etapa_rl_pack(tEtapaReduccionLocal* rl){
	char* arch = string_new();
	char* archivos = string_new();

	for(int i = 0; i< mlist_length(rl->archivos_temporales_de_transformacion) ;i++){
		archivos = mlist_get(rl->archivos_temporales_de_transformacion,i);
		string_append(&arch, archivos);
		string_append(&arch, ":");
	}

	return serial_pack("sssss",
			rl->nodo,
			rl->ip,
			rl->puerto,
			arch,
			rl->archivo_etapa);

}

tEtapaReduccionLocal* etapa_rl_unpack(t_serial *serial){
	tEtapaReduccionLocal *rl = malloc(sizeof(tEtapaReduccionLocal));
	char* archivos = string_new();

	serial_unpack(serial,"sssss",
			&rl->nodo,
			&rl->ip,
			&rl->puerto,
			&archivos,
			&rl->archivo_etapa);

	rl->archivos_temporales_de_transformacion = mlist_create();
	int i = 0;
	char** arch = string_split(archivos, ":");
	while(arch[i] != NULL){
		mlist_append(rl->archivos_temporales_de_transformacion, arch[i]);
		i++;
	}

	return rl;
}

void mandar_etapa_rl(tEtapaReduccionLocal* rl,t_socket sock){
	t_serial *serial = etapa_rl_pack(rl);
	t_packet paquete = protocol_packet(OP_INICIAR_REDUCCION_LOCAL, serial);
	protocol_send_packet(paquete, sock);
	serial_destroy(serial);
}

/*
 * **************************************************************************************FIN FUNCIONES
 * **************************************************************************************ETAPA REDUCCION LOCAL
 * */

/*
 * **************************************INICIO FUNCIONES
 * *************************************ETAPA REDUCCION GLOBAL
 *
 * */

tEtapaReduccionGlobal* new_etapa_rg(const char*nodo,const char*ip,const char*puerto,char*archivo_temporal_rl,const char * archivo_etapa,char * encargado){
	tEtapaReduccionGlobal* rg = malloc(sizeof(tEtapaReduccionGlobal));

	rg->nodo = string_duplicate((char*)nodo);
	rg->ip = string_duplicate((char*)ip);
	rg->puerto = string_duplicate((char*)puerto);
	rg->archivo_temporal_de_rl = string_duplicate(archivo_temporal_rl);
	rg->archivo_etapa = string_duplicate((char*)archivo_etapa);
	rg->encargado = string_duplicate(encargado);

	return rg;
}

t_serial *list_reduccionGlobal_pack(mlist_t* list) {
	t_serial *serial = serial_create(NULL, 0);
	serial_add(serial, "i", mlist_length(list));

	void routine(tEtapaReduccionGlobal* rg) {
		serial_add(serial, "ssssss",
				rg->nodo,
				rg->ip,
				rg->puerto,
				rg->archivo_temporal_de_rl,
				rg->archivo_etapa,
				rg->encargado);
	}
	mlist_traverse(list, routine);
	return serial;
}



tEtapaReduccionGlobal* etapa_rg_unpack(t_serial *serial){
	tEtapaReduccionGlobal* rg = malloc(sizeof(tEtapaReduccionGlobal));
	serial_unpack(serial,"ssssss",
			&rg->nodo,
			&rg->ip,
			&rg->puerto,
			&rg->archivo_temporal_de_rl,
			&rg->archivo_etapa,
			&rg->encargado);
	return rg;
}

mlist_t *list_reduccionGlobal_unpack(t_serial *serial) {
	mlist_t *list = mlist_create();

	int numblocks;
	serial_remove(serial, "i", &numblocks);

	while(numblocks--) {
		tEtapaReduccionGlobal *rg = malloc(sizeof(tEtapaReduccionGlobal));
		serial_remove(serial, "ssssss",
				&rg->nodo,
				&rg->ip,
				&rg->puerto,
				&rg->archivo_temporal_de_rl,
				&rg->archivo_etapa,
				&rg->encargado);
		mlist_append(list, rg);
	}

	return list;
}

void mandar_etapa_rg(mlist_t* list,t_socket sock){
	t_serial *serial = list_reduccionGlobal_pack(list);
	t_packet paquete = protocol_packet(OP_INICIAR_REDUCCION_GLOBAL, serial);
	protocol_send_packet(paquete, sock);
	serial_destroy(serial);
}

/*
 * **************************************************************************************FIN FUNCIONES
 * **************************************************************************************ETAPA REDUCCION GLOBAL
 * */

/*
 * **************************************INICIO FUNCIONES
 * *************************************ETAPA ALMACENAMIENTO FINAL
 *
 * */

tAlmacenadoFinal* new_etapa_af(const char* nodo, const char* ip, const char* puerto, const char* archivo_etapa){
	tAlmacenadoFinal* af = malloc(sizeof(tAlmacenadoFinal));

	af->nodo = string_duplicate((char*)nodo);
	af->ip = string_duplicate((char*)ip);
	af->puerto = string_duplicate((char*)puerto);
	af->archivo_etapa = string_duplicate((char*)archivo_etapa);

	return af;
}

t_serial *etapa_af_pack(tAlmacenadoFinal* af){
	return serial_pack("ssss",
			af->nodo,
			af->ip,
			af->puerto,
			af->archivo_etapa);
}

tAlmacenadoFinal* etapa_af_unpack(t_serial *serial){
	tAlmacenadoFinal* af = malloc(sizeof(tAlmacenadoFinal));
	serial_unpack(serial,"ssss",
			&af->nodo,
			&af->ip,
			&af->puerto,
			&af->archivo_etapa);
	return af;
}

void mandar_etapa_af(tAlmacenadoFinal* af,t_socket sock){
	t_serial *serial = etapa_af_pack(af);
	t_packet paquete = protocol_packet(OP_INICIAR_ALMACENAMIENTO, serial);
	protocol_send_packet(paquete, sock);
	serial_destroy(serial);
}

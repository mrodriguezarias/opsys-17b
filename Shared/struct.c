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

t_serial *etapa_transformacion_pack(tEtapaTransformacion* transformacion){
	return serial_pack("sssiis",
			transformacion->nodo,
			transformacion->ip,
			transformacion->puerto,
			transformacion->bloque,
			transformacion->bytes_ocupados,
			transformacion->archivo_etapa);
}

t_serial* list_transformacion_pack(mlist_t* list){
	t_serial* serial = serial_pack("i", mlist_length(list));
	mlist_t* listSerial = mlist_map(list,etapa_transformacion_pack);
	char buff[SERIAL_MAX];
	t_serial* ser = serial_create(buff, SERIAL_MAX);
	memcpy(ser->data, serial->data, serial->size);
	ser->size = serial->size;

	for(int i = 0; i < mlist_length(list); i++){
		t_serial* aux = mlist_get(listSerial, i);
		memcpy(ser->data+ser->size, aux->data, aux->size);
		ser->size += aux->size;
	}

	return ser;
}

tEtapaTransformacion* etapa_transformacion_unpack(t_serial *serial){
	tEtapaTransformacion* et = malloc(sizeof(tEtapaTransformacion));
	serial_unpack(serial,"sssiis",&et->nodo,
			&et->ip,
			&et->puerto,
			&et->bloque,
			&et->bytes_ocupados,
			&et->archivo_etapa);
	return et;
}

mlist_t* list_transformacion_unpack(t_serial* serial){
	int tam = 0, size = 0;
	mlist_t* list = mlist_create();
	serial_unpack(serial,"i",&tam);
	size += 4;

	for(int i = 0; i < tam; i++){
		t_serial* aux = serial_create(serial->data+size, serial->size);
		tEtapaTransformacion* element = etapa_transformacion_unpack(aux);
		mlist_append(list, element);
		size += 8;
		size += string_length(element->nodo)+1;
		size += string_length(element->ip)+1;
		size += string_length(element->puerto)+1;
		size += string_length(element->archivo_etapa)+1;
	}

	return list;
}

void mandar_etapa_transformacion(mlist_t* list,t_socket sock){
	t_serial *serial = list_transformacion_pack(list);
	t_packet paquete = protocol_packet(OP_INICIAR_TRANSFORMACION, serial);
	protocol_send_packet(paquete, sock);
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

t_serial *etapa_rg_pack(tEtapaReduccionGlobal* rg){
	return serial_pack("ssssss",
			rg->nodo,
			rg->ip,
			rg->puerto,
			rg->archivo_temporal_de_rl,
			rg->archivo_etapa,
			rg->encargado);
}

t_serial* list_reduccionGlobal_pack(mlist_t* list){
	t_serial* serial = serial_pack("i", mlist_length(list));
	mlist_t* listSerial = mlist_map(list,etapa_rg_pack);
	char buff[SERIAL_MAX];
	t_serial* ser = serial_create(buff, SERIAL_MAX);
	memcpy(ser->data, serial->data, serial->size);
	ser->size = serial->size;

	for(int i = 0; i < mlist_length(list); i++){
		t_serial* aux = mlist_get(listSerial, i);
		memcpy(ser->data+ser->size, aux->data, aux->size);
		ser->size += aux->size;
	}

	return ser;
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

mlist_t* list_reduccionGlobal_unpack(t_serial* serial){
	int tam = 0, size = 0;
	mlist_t* list = mlist_create();
	serial_unpack(serial,"i",&tam);
	size += 4;

	for(int i = 0; i < tam; i++){
		t_serial* aux = serial_create(serial->data+size, serial->size);
		tEtapaReduccionGlobal* element = etapa_rg_unpack(aux);
		mlist_append(list, element);
		size += string_length(element->nodo)+1;
		size += string_length(element->ip)+1;
		size += string_length(element->puerto)+1;
		size += string_length(element->archivo_temporal_de_rl)+1;
		size += string_length(element->archivo_etapa)+1;
		size += string_length(element->encargado)+1;
	}

	return list;
}

void mandar_etapa_rg(mlist_t* list,t_socket sock){
	t_serial *serial = list_reduccionGlobal_pack(list);
	t_packet paquete = protocol_packet(OP_INICIAR_REDUCCION_GLOBAL, serial);
	protocol_send_packet(paquete, sock);
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
}

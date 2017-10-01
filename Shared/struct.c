#include "struct.h"

/*
 * **************************************INICIO FUNCIONES
 * *************************************ETAPA TRANSFORMACION
 *
 * */
tEtapaTransformacion new_etapa_transformacion(const char*nodo,const char*ip,const char*puerto,int bloque,int bytes_ocuapdos,const char * archivo_etapa){
	tEtapaTransformacion et = {
			.bloque = bloque,
			.bytes_ocupados = bytes_ocuapdos
	};
	et.nodo = malloc(20);
	et.ip = malloc(20);
	et.puerto = malloc(20);
	et.archivo_etapa = malloc(20);
	strcpy(et.nodo,nodo);
	strcpy(et.ip,ip);
	strcpy(et.puerto,puerto);
	strcpy(et.archivo_etapa,archivo_etapa);


	return et;
}

t_serial etapa_transformacion_pack(tEtapaTransformacion transformacion){
	return serial_pack("sssiis",
			transformacion.nodo,
			transformacion.ip,
			transformacion.puerto,
			transformacion.bloque,
			transformacion.bytes_ocupados,
			transformacion.archivo_etapa);
}

tEtapaTransformacion etapa_transformacion_unpack(t_serial serial){
	tEtapaTransformacion et;
	serial_unpack(serial,"sssiis",&et.nodo,
			&et.ip,
			&et.puerto,
			&et.bloque,
			&et.bytes_ocupados,
			&et.archivo_etapa);
	return et;
}


void mandar_etapa_transformacion(tEtapaTransformacion et,t_socket sock){
	printf("archivo etapa: %s\n"
						"ip worker: %s \n"
						"puerto worker: %s \n",
						et.archivo_etapa,
						et.ip,
						et.puerto);
	t_serial serial = etapa_transformacion_pack(et);
	free(et.nodo);free(et.ip);free(et.puerto);free(et.archivo_etapa);
	t_packet paquete = protocol_packet(OP_INICIAR_TRANSFORMACION, serial);
	protocol_send(paquete, sock);
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

tEtapaReduccionLocal new_etapa_rl(const char*nodo,const char*ip,const char*puerto,t_list*archivos_temporales,const char * archivo_etapa){
	tEtapaReduccionLocal rl;


	strcpy(rl.nodo,nodo);
	strcpy(rl.ip,ip);
	strcpy(rl.puerto,puerto);
	strcpy(rl.archivo_etapa,archivo_etapa);

	for(int i = 0;i < list_size(archivos_temporales);i++){
		list_add(rl.archivos_temporales_de_transformacion,list_get(archivos_temporales,i));
	}
	return rl;
}
t_serial etapa_rl_pack(tEtapaReduccionLocal rl){
	int cantidad_archivos = list_size(rl.archivos_temporales_de_transformacion);
	int cantidad_letras_directorio_archivo = strlen(rl.archivo_etapa);
	char * archivos[cantidad_archivos];
	char  arch[cantidad_archivos * cantidad_letras_directorio_archivo];
	for(int i = 0;i<list_size(rl.archivos_temporales_de_transformacion);i++){
		archivos[i] =list_get(rl.archivos_temporales_de_transformacion,i);
		strcat(arch,archivos[i]);
	}
	return serial_pack("sssss",
			rl.nodo,
			rl.ip,
			rl.puerto,
			arch,
			rl.archivo_etapa);

}
tEtapaReduccionLocal etapa_rl_unpack(t_serial serial){
	tEtapaReduccionLocal rl;
	char archivos[serial.size];
	serial_unpack(serial,"sssss",
			rl.nodo,
			rl.ip,
			rl.puerto,
			archivos,
			rl.archivo_etapa);
	rl.archivos_temporales_de_transformacion = list_create();
	return rl;
}

char * obtener_archivo_temporal(char * archivos, char * archivo_a_obtener){
	if(strstr(archivos,archivo_a_obtener) != NULL)
		return archivo_a_obtener;
	else{
		printf("El archivo %s no existe o no encuentra en esta etapa",archivo_a_obtener);
		return "No existe el archivo que se requiere";
	}
}

void mandar_etapa_rl(tEtapaReduccionLocal rl,t_socket sock){
	t_serial serial = etapa_rl_pack(rl);
	t_packet paquete = protocol_packet(OP_INICIAR_REDUCCION_LOCAL, serial);
	protocol_send(paquete, sock);
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


tEtapaReduccionGlobal new_etapa_rg(const char*nodo,const char*ip,const char*puerto,char*archivo_temporal_rl,const char * archivo_etapa,char * encargado){
	tEtapaReduccionGlobal rg;

	strcpy(rg.nodo,nodo);
	strcpy(rg.ip,ip);
	strcpy(rg.puerto,puerto);
	strcpy(rg.archivo_temporal_de_rl,archivo_temporal_rl);
	strcpy(rg.archivo_etapa,archivo_etapa);
	strcpy(rg.encargado,encargado);

	return rg;
	;
}
t_serial etapa_rg_pack(tEtapaReduccionGlobal rg){
	return serial_pack("ssssss",
			rg.nodo,
			rg.ip,
			rg.puerto,
			rg.archivo_temporal_de_rl,
			rg.archivo_etapa,
			rg.encargado);
}
tEtapaReduccionGlobal etapa_rg_unpack(t_serial serial){
	tEtapaReduccionGlobal rg;
	serial_unpack(serial,"ssssss",
			rg.nodo,
			rg.ip,
			rg.puerto,
			rg.archivo_temporal_de_rl,
			rg.archivo_etapa,
			rg.encargado);
	return rg;
}

void mandar_etapa_rg(tEtapaReduccionGlobal rg,t_socket sock){
	t_serial serial = etapa_rg_pack(rg);
	t_packet paquete = protocol_packet(OP_INICIAR_REDUCCION_GLOBAL, serial);
	protocol_send(paquete, sock);
}

/*
 * **************************************************************************************FIN FUNCIONES
 * **************************************************************************************ETAPA REDUCCION GLOBAL
 * */





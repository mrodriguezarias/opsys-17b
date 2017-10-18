#include "struct.h"


tinformacionArchivo* informacionArchivo_unpack(t_serial *serial){
	tinformacionArchivo* infoArch = malloc(sizeof(tinformacionArchivo));
	serial_unpack(serial,"ii",
			&infoArch->bloque,

			&infoArch->bytesOcupados);

	return infoArch;
}


mlist_t* list_informacionArchivo_unpack(t_serial* serial){
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

#include "struct.h"


mlist_t *nodelist_unpack(t_serial *serial) {
	mlist_t *list = mlist_create();

	int tamanioLista;
	serial_remove(serial, "i", &tamanioLista);

	while(tamanioLista--) {
		t_infoNodo *infonodo = malloc(sizeof(t_infoNodo));
		serial_remove(serial,"sii",&infonodo->nodo,
				&infonodo->ip,
				&infonodo->puerto
				);
		mlist_append(list, infonodo);
	}
	serial_destroy(serial);
	return list;
}

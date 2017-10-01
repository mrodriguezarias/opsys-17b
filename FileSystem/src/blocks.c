#include "blocks.h"

#include <path.h>

static void divide_in_blocks_txt(const char *path);
static void divide_in_blocks_bin(const char *path);

// ========== Funciones públicas ==========

// Recibirá una ruta completa, el nombre del archivo, el tipo (texto o binario) y los datos correspondientes.
// Responderá con un mensaje confirmando el resultado de la operación. Es responsabilidad del proceso Filesystem:
// Cortar el archivo en registros completos hasta 1MB en el caso de los archivos de texto o en bloques de 1 MB
// en el caso de los binarios.

void divide_in_blocks(const char *path) {
	if(path_istext(path)) {
		divide_in_blocks_txt(path);
	} else {
		divide_in_blocks_bin(path);
	}
}

// ========== Funciones privadas ==========

static void divide_in_blocks_txt(const char *path) {

}

static void divide_in_blocks_bin(const char *path) {

}

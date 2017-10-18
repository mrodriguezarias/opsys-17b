#ifndef YFILE_H_
#define YFILE_H_

#include <file.h>
#include <serial.h>

typedef struct yfile {
	char *path;
	size_t size;
	t_ftype type;
	mlist_t *blocks;
} t_yfile;

typedef struct {
	int blockno;
	char *node;
} t_block_copy;

typedef struct {
	int index;
	size_t size;
	t_block_copy copies[2];
} t_block;

/**
 * Crea un archivo del sistema de archivos yamafs.
 * @param path Ruta real del archivo.
 * @param type Tipo de archivo (texto o binario).
 * @return Archivo yamafs.
 */
t_yfile *yfile_create(const char *path, t_ftype type);

/**
 * Crea la ruta real correspondiente a una ruta YAMA.
 * @param path Ruta YAMA.
 * @return Ruta real.
 */
char *yfile_path(const char *path);

/**
 * Agrega un bloque de datos a un archivo yamafs.
 * @param file Archivo yamafs.
 * @param block Bloque de datos.
 */
void yfile_addblock(t_yfile *file, t_block *block);

/**
 * Imprime información administrativa de un archivo yamafs.
 * @param file Archivo yamafs.
 */
void yfile_print(t_yfile *file);

/**
 * Serializa un archivo.
 * @param file Archivo yamafs.
 * @return Contenedor serial.
 */
t_serial *yfile_pack(t_yfile *file);

/**
 * Deserializa un archivo.
 * @param serial Contenedor serial.
 * @return Archivo yamafs.
 */
t_yfile *yfile_unpack(t_serial *serial);

/**
 * Destruye un archivo yamafs.
 * @param file Archivo yamafs.
 */
void yfile_destroy(t_yfile *file);

#endif /* YFILE_H_ */

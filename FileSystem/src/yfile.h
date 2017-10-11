#ifndef YFILE_H_
#define YFILE_H_

#include <file.h>

typedef struct yfile {
	char *path;
	size_t size;
	t_ftype type;
	mlist_t *blocks;
} t_yfile;

typedef struct {
	char *node;
	int blockno;
} t_block_copy;

typedef struct {
	int index;
	size_t size;
	t_block_copy copies[2];
} t_block;

/**
 * Crea un archivo del sistema de archivos yamafs.
 * @param path Ruta del archivo yamafs.
 * @param type Tipo de archivo (texto o binario).
 * @return Archivo yamafs.
 */
t_yfile *yfile_create(const char *path, t_ftype type);

/**
 * Crea la ruta real correspondiente a una ruta YAMA.
 * @param yama_path Ruta YAMA.
 * @return Ruta real.
 */
char *yfile_path(const char *yama_path);

/**
 * Agrega un bloque de datos a un archivo yamafs.
 * @param file Archivo yamafs.
 * @param block Bloque de datos.
 */
void yfile_addblock(t_yfile *file, t_block *block);

/**
 * Crea un archivo del sistema de archivos yamafs copiando el
 * contenido de un archivo local.
 * @param path Ruta al archivo local que se quiere copiar.
 * @param dir Ruta de yamafs donde se guardará el archivo.
 * @return Archivo yamafs.
 */
t_yfile *yfile_cpfrom(const char *path, const char *dir);

/**
 * Imprime información administrativa de un archivo yamafs.
 * @param file Archivo yamafs.
 */
void yfile_print(t_yfile *file);

/**
 * Destruye un archivo yamafs.
 * @param file Archivo yamafs.
 */
void yfile_destroy(t_yfile *file);

#endif /* YFILE_H_ */

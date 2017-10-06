#ifndef YFILE_H_
#define YFILE_H_

#include <file.h>

struct t_node;

typedef struct yfile t_yfile;

typedef struct {
	struct t_node *node;
	int blockno;
} t_block_copy;

typedef struct {
	t_block_copy copies[2];
	size_t size;
} t_block;

/**
 * Crea un archivo del sistema de archivos yamafs.
 * @param path Ruta del archivo yamafs.
 * @param type Tipo de archivo (texto o binario).
 * @return Archivo yamafs.
 */
t_yfile *yfile_create(const char *path, t_ftype type);

/**
 * Crea un archivo del sistema de archivos yamafs copiando el
 * contenido de un archivo local.
 * @param path Ruta al archivo local que se quiere copiar.
 * @param dir Ruta de yamafs donde se guardará el archivo.
 * @return Archivo yamafs.
 */
t_yfile *yfile_cpfrom(const char *path, const char *dir);

#endif /* YFILE_H_ */

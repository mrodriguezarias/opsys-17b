#ifndef YFILE_H_
#define YFILE_H_

#include <file.h>

typedef struct yfile t_yfile;

/**
 * Crea un archivo del sistema de archivos yamafs.
 * @param path Ruta del archivo yamafs.
 * @param type Tipo de archivo (texto o binario).
 * @return Archivo yamafs.
 */
t_yfile *yfile_create(const char *path, t_ftype type);

#endif /* YFILE_H_ */

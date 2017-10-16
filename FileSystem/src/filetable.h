#ifndef FILETABLE_H_
#define FILETABLE_H_

#include "yfile.h"

/**
 * Inicializa la tabla de archivos.
 */
void filetable_init(void);

/**
 * Devuelve la cantidad de archivos en la tabla.
 * @return Total de archivos.
 */
int filetable_size(void);

/**
 * Agrega un archivo a la tabla de archivos.
 * @param file Archivo yama a agregar.
 */
void filetable_add(t_yfile *file);

/**
 * Encuentra un archivo por su ruta.
 * @param path Ruta al archivo.
 * @return Archivo yamafs.
 */
t_yfile *filetable_find(const char *path);

/**
 * Determina si existe un archivo yama en el sistema.
 * @param path Ruta al archivo.
 * @return Valor lógico indicando si el archivo existe.
 */
bool filetable_contains(const char *path);

/**
 * Renombra un archivo.
 * @param path Ruta al archivo a renombrar.
 * @param new_name Nuevo nombre.
 */
void filetable_rename(const char *path, const char *new_name);

/**
 * Remueve un archivo de la tabla.
 * @param Ruta al archivo.
 */
void filetable_remove(const char *path);

/**
 * Elimina todos los archivos de la tabla.
 */
void filetable_clear();

/**
 * Imprime la tabla de archivos.
 */
void filetable_print(void);

/**
 * Devuelve la cantidad de archivos en una directorio.
 * @param Ruta al directorio yamafs.
 * @return Cantidad de archivos en el directorio.
 */
size_t filetable_count(const char *path);

/**
 * Imprime los archivos que pertenezcan a un determinado directorio.
 * @param path Ruta al directorio yamafs.
 */
void filetable_ls(const char *path);

/**
 * Crea un archivo del sistema de archivos yamafs copiando el
 * contenido de un archivo local.
 * @param path Ruta al archivo local que se quiere copiar.
 * @param dir Ruta de yamafs donde se guardará el archivo.
 */
void filetable_cpfrom(const char *path, const char *dir);

#endif /* FILETABLE_H_ */

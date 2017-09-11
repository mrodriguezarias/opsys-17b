#ifndef FILE_H_
#define FILE_H_

#include <stdbool.h>
#include <stddef.h>

/**
 * Verifica si un archivo existe en el sistema.
 * @param path Ruta al archivo a verificar.
 * @return Valor lógico indicando si existe el archivo.
 */
bool file_exists(const char *path);

/**
 * Verifica si un archivo es un directorio.
 * @param path Ruta al archivo a verificar.
 * @return Valor lógico indicando si es un directorio.
 */
bool file_isdir(const char *path);

/**
 * Crea un directorio en el sistema.
 * También crea los directorios intermedios, si no existían.
 * @param path Ruta del directorio a crear.
 */
void file_mkdir(const char *path);

/**
 * Devuelve el tamaño de un archivo.
 * @param path Ruta al archivo.
 * @return Tamaño del archivo en bytes.
 */
size_t file_size(const char *path);

/**
 * Devuelve la ruta al directorio donde se encuentra un archivo.
 * La cadena devuelta debe ser liberada con free().
 * @param Ruta al archivo.
 * @return Directorio del archivo.
 */
char *file_dir(const char *path);

/**
 * Devuelve el nombre de un archivo.
 * @param path Ruta al archivo.
 * @return Nombre del archivo.
 */
const char *file_name(const char *path);

/**
 * Copia un archivo de un ruta (source) a otra (target).
 * Si el archivo destino ya existía, lo reemplaza.
 * @param source Ruta del archivo original.
 * @param target Ruta de la copia.
 */
void file_copy(const char *source, const char *target);

#endif /* FILE_H_ */

#ifndef PATH_H_
#define PATH_H_

#include <stdbool.h>
#include <stddef.h>
#include <mlist.h>

/*
 * Para todas estas funciones, si se les pasa por parámetro una ruta relativa,
 * se la considera relativa al directorio del usuario (~/yatpos).
 */

/**
 * Verifica si una ruta apunta a un archivo existente.
 * @param path Ruta a verificar.
 * @return Valor lógico indicando si existe el archivo.
 */
bool path_exists(const char *path);

/**
 * Verifica si una ruta apunta a un directorio.
 * @param path Ruta a verificar.
 * @return Valor lógico indicando si es un directorio.
 */
bool path_isdir(const char *path);

/**
 * Verifica si una ruta apunta a un archivo regular.
 * @param path Ruta a verificar.
 * @return Valor lógico indicando si es un archivo.
 */
bool path_isfile(const char *path);

/**
 * Determina si una ruta apunta a un archivo de texto.
 * @param path Ruta al archivo a verificar.
 * @return Valor lógico indicando si es de texto.
 */
bool path_istext(const char *path);

/**
 * Determina si una ruta apunta a un archivo binario.
 * @param path Ruta al archivo a verificar.
 * @return Valor lógico indicando si es binario.
 */
bool path_isbin(const char *path);

/**
 * Crea un directorio en el sistema.
 * También crea los directorios intermedios, si no existían.
 * @param path Ruta del directorio a crear.
 */
void path_mkdir(const char *path);

/**
 * Devuelve el tamaño de un archivo o un directorio.
 * @param path Ruta al archivo.
 * @return Tamaño del archivo en bytes.
 */
size_t path_size(const char *path);

/**
 * Devuelve el tamaño de un archivo con prefijo binario (KiB, MiB, etc.).
 * La cadena devuelta debe ser liberada con free().
 * @param path Ruta al archivo.
 * @return Cadena con el tamaño del archivo.
 */
char *path_sizep(const char *path);

/**
 * Devuelve la ruta al directorio donde se encuentra un archivo.
 * La cadena devuelta debe ser liberada con free().
 * @param Ruta al archivo.
 * @return Directorio del archivo.
 */
char *path_dir(const char *path);

/**
 * Devuelve el nombre de un archivo.
 * @param path Ruta al archivo.
 * @return Nombre del archivo.
 */
const char *path_name(const char *path);

/**
 * Crea una ruta aleatoria para un nuevo archivo temporal.
 * @return Ruta al archivo temporal (debe liberarse con free()).
 */
char *path_temp(void);

/**
 * Crea un archivo si no existía.
 * @param path Ruta al archivo a crear.
 */
void path_create(const char *path);

/**
 * Copia un archivo de una ruta (source) a otra (target).
 * Si el archivo destino ya existía, lo reemplaza.
 * @param source Ruta del archivo original.
 * @param target Ruta de la copia.
 */
void path_copy(const char *source, const char *target);

/**
 * Mueve un archivo de una ruta (source) a otra (target).
 * Si el archivo destino ya existía, lo reemplaza.
 * @param source Ruta del archivo original.
 * @param target Ruta de la copia.
 */
void path_move(const char *source, const char *target);

/**
 * Elimina un archivo o un directorio y su contenido.
 * @param path Ruta al archivo a eliminar.
 */
void path_remove(const char *path);

/**
 * Calcula el hash MD5 de un archivo.
 * @param path Ruta al archivo.
 * @return MD5 del archivo (debe ser liberado).
 */
char *path_md5(const char *path);

/**
 * Crea un archivo con un determinado tamaño, rellenándolo con ceros.
 * Si ya existía y era de menor tamaño, lo agranda.
 * Si era de mayor tamaño, lo achica.
 * @param path Ruta al archivo.
 * @param size Tamaño deseado del archivo.
 */
void path_truncate(const char *path, size_t size);

/**
 * Ordena alfabéticamente las líneas de un archivo de texto.
 * @param path Ruta al archivo.
 */
void path_sort(const char *path);

/**
 * Aparea un conjunto de archivos, juntándolos en uno solo.
 * Precondición: los archivos fuente deben estar ordenados.
 * @param sources Lista con los archivos a aparear.
 * @param target Ruta al archivo a crear con el resultado del apareo.
 */
void path_merge(mlist_t *sources, const char *target);

#endif /* PATH_H_ */

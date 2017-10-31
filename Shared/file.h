#ifndef FILE_H_
#define FILE_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <mlist.h>
#include <limits.h>

typedef enum {FTYPE_TXT, FTYPE_BIN} t_ftype;
typedef struct file t_file;

/**
 * Abre un archivo para escritura y lectura. Si ya existía lo vacía.
 * La ruta raíz por defecto es el directorio del usuario (~/yatpos).
 * @param path Ruta al archivo.
 * @return Descriptor del archivo.
 */
t_file *file_create(const char *path);

/**
 * Abre un archivo para lectura y escritura. Si no existía lo crea.
 * La ruta raíz por defecto es el directorio del usuario (~/yatpos).
 * @param path Ruta al archivo.
 * @return Descriptor del archivo.
 */
t_file *file_open(const char *path);

/**
 * Devuelve la ruta de un archivo abierto.
 * @param file Archivo abierto.
 * @return Ruta al archivo.
 */
const char *file_path(t_file *file);

/**
 * Devuelve el tipo de un archivo abierto (si es texto o binario).
 * @param file Archivo abierto.
 * @return Tipo del archivo.
 */
t_ftype file_type(t_file *file);

/**
 * Devuelve el tamaño de un archivo abierto.
 * @param file Archivo abierto.
 * @return Tamaño del archivo.
 */
size_t file_size(t_file *file);

/**
 * Devuelve un puntero al descriptor de un archivo (FILE).
 * @param file Archivo abierto.
 * @return Puntero al descriptor interno del archivo.
 */
FILE *file_pointer(t_file *file);

/**
 * Mueve la posición actual del archivo al inicio del mismo.
 * @param file Archivo abierto.
 */
void file_rewind(t_file *file);

/**
 * Lee la siguiente línea de un archivo de texto.
 * La cadena devuelta debe ser liberada con free().
 * @param file Archivo a leer.
 * @return Línea del archivo.
 */
char *file_readline(t_file *file);

/**
 * Escribe la siguiente línea de un archivo de texto.
 * @param file Archivo a escribir.
 * @param line Línea a escribir.
 */
void file_writeline(t_file *file, const char *line);

/**
 * Itera sobre cada línea de un archivo de texto.
 * @param file Archivo de texto a recorrer.
 * @param routine Rutina a ejecutar para cada línea.
 */
void file_ltraverse(t_file *file, bool (*routine)(const char *line));

/**
 * Itera sobre bloques alineados a memoria de un archivo binario.
 * @param file Archivo binario a recorrer.
 * @param routine Rutina a ejecutar para cada bloque de datos.
 */
void file_btraverse(t_file *file, bool (*routine)(const void *block, size_t size));

/**
 * Elimina el contenido de un archivo.
 * @param file Archivo abierto.
 */
void file_clear(t_file *file);

/**
 * Cierra y elimina un archivo.
 * @param file Archivo abierto.
 */
void file_delete(t_file *file);

/**
 * Mapea a memoria un archivo abierto.
 * @param file Archivo a mapear.
 * @return Puntero al mapeo (debe ser liberado con file_unmap).
 */
void *file_map(t_file *file);

/**
 * Sincroniza un mapeo de memoria hecho por file_map().
 * @param file Archivo sobre el que se hizo el mapeo.
 * @param map Mapeo de memoria.
 */
void file_sync(t_file *file, void *map);

/**
 * Libera un mapeo de memoria hecho por file_map().
 * @param file Archivo sobre el que se hizo el mapeo.
 * @param map Mapeo de memoria.
 */
void file_unmap(t_file *file, void *map);

/**
 * Cierra un archivo.
 * @param file Archivo a cerrar.
 */
void file_close(t_file *file);

#endif /* FILE_H_ */

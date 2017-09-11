#ifndef FILE_H_
#define FILE_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

typedef FILE t_file;

/*
 * Para todas estas funciones, si se les pasa por parámetro una ruta relativa,
 * se la considera relativa al directorio del usuario (~/yatpos).
 */

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
 * Devuelve el tamaño de un archivo con prefijo binario (KiB, MiB, etc.).
 * La cadena devuelta debe ser liberada con free().
 * @param path Ruta al archivo.
 * @return Cadena con el tamaño del archivo.
 */
char *file_sizep(const char *path);

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

/**
 * Abre un archivo para lectura o escritura.
 * La ruta raíz por defecto es el directorio del usuario (~/yatpos).
 * @param path Ruta al archivo.
 * @return Descriptor del archivo.
 */
t_file *file_open(const char *path);

/**
 * Devuelve la ruta un archivo abierto.
 * La cadena devuelta debe ser liberada con free().
 * @param file Archivo abierto.
 * @return Ruta al archivo.
 */
char *file_path(t_file *file);

/**
 * Lee la siguiente línea de un archivo de texto.
 * La cadena devuelta debe ser liberada con free().
 * @param file Archivo a leer.
 * @return Línea del archivo.
 */
char *file_readline(t_file *file);

/**
 * Lee todas las líneas de un archivo de texto.
 * La cadena devuelta debe ser liberada con free().
 * @param file Archivo a leer.
 * @return Líneas del archivo.
 */
char *file_readlines(t_file *file);

/**
 * Cierra un archivo.
 * @param file Archivo a cerrar.
 */
void file_close(t_file *file);

#endif /* FILE_H_ */

#ifndef FILE_H_
#define FILE_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <mlist.h>

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
 * Crea un archivo si no existía.
 * @param path Ruta al archivo a crear.
 */
void file_create(const char *path);

/**
 * Crea un archivo si no existía. Si existía lo elimina y lo vuelve a crear.
 * @param path Ruta al archivo a recrear.
 */
void file_recreate(const char *path);

/**
 * Copia un archivo de un ruta (source) a otra (target).
 * Si el archivo destino ya existía, lo reemplaza.
 * @param source Ruta del archivo original.
 * @param target Ruta de la copia.
 */
void file_copy(const char *source, const char *target);

/**
 * Elimina un archivo, un directorio o un enlace simbólico.
 * @param path Ruta al archivo a eliminar.
 */
void file_delete(const char *path);

/**
 * Crea un archivo para lectura y escritura.
 * La ruta raíz por defecto es el directorio del usuario (~/yatpos).
 * @param path Ruta al archivo.
 * @return Descriptor del archivo.
 */
t_file *file_new(const char *path);

/**
 * Abre un archivo para lectura y escritura.
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
 * Escribe una línea en un archivo de texto.
 * Inserta un '\n' al final si no estaba.
 * @param line Línea a escribir.
 * @param file Archivo a escribir.
 */
void file_writeline(const char *line, t_file *file);

/**
 * Crea un archivo con un determinado tamaño, rellenándolo con ceros.
 * Si ya existía y era de menor tamaño, lo agranda. Si era más grande,
 * lo achica.
 * @param path Ruta al archivo.
 * @param size Tamaño deseado del archivo.
 */
void file_truncate(const char *path, size_t size);

/**
 * Aparea un conjunto de archivos, juntándolos en un solo.
 * @param sources Lista con los archivos a aparear.
 * @param target Ruta al archivo a crear con el resultado del apareo.
 */
void file_merge(mlist_t *sources, const char *target);

/**
 * Cierra un archivo.
 * @param file Archivo a cerrar.
 */
void file_close(t_file *file);

#endif /* FILE_H_ */

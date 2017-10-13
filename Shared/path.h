#ifndef PATH_H_
#define PATH_H_

#include <stdbool.h>
#include <stddef.h>
#include <mlist.h>
#include <mstring.h>

typedef enum {PTYPE_YATPOS, PTYPE_YAMA, PTYPE_USER} t_ptype;

/*
 * Para todas estas funciones, si se les pasa por parámetro una ruta relativa,
 * se la considera relativa al directorio del usuario (~/yatpos).
 */

/**
 * Crea una ruta uniendo todas las cadenas pasadas por parámetro.
 * El tipo de ruta especifica la localidad de la ruta:
 * - PTYPE_YATPOS: local a yatpos (~/yatpos/{ruta})
 * - PTYPE_YAMA: local a yamafs (yamafs:/{ruta})
 * - PTYPE_USER: local al usuario ({cwd}/{ruta})
 * Si la ruta es absoluta (empieza con /), el tipo de ruta es ignorado.
 * @param type Tipo de ruta.
 * @return Ruta creada (debe ser liberada).
 */
#define path_create(type, ...) _path_create(type, #__VA_ARGS__, ##__VA_ARGS__)
char *_path_create(t_ptype type, const char *scount, ...);

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
 * Determina si un archivo está vacío.
 * @param path Ruta al archivo o directorio.
 * @return Valor lógico indicando si está vacío.
 */
bool path_isempty(const char *path);

/**
 * Compara dos rutas para ver si son iguales.
 * @param path1 Ruta 1.
 * @param path2 Ruta 2.
 * @return Valor lógico indicando si son iguales.
 */
bool path_equal(const char *path1, const char *path2);

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
void path_mkfile(const char *path);

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
 * Itera sobre los archivos de un directorio.
 * @param dpath Ruta al directorio.
 * @param routine Rutina de iteración.
 */
void path_files(const char *dpath, void (*routine)(const char *path));

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

/**
 * Aplica un script de transformación sobre un bloque de un archivo.
 * Guarda el resultado en un archivo temporal.
 * @param block Bloque de datos a transformar.
 * @param script Ruta al script de tranformación (relativa al usuario).
 * @param output Ruta al archivo temporal resultante (relativa a ~/yatpos).
 */
void path_apply(void *block, const char *script, const char *output);

#endif /* PATH_H_ */

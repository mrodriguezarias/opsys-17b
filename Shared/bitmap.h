#ifndef BITMAP_H_
#define BITMAP_H_

#include <stdbool.h>
#include <stdlib.h>

typedef struct bitmap t_bitmap;

/**
 * Crea un mapa de bits de un determinado tamaño.
 * @param size Tamaño del mapa de bits (en bits).
 * @return Mapa de bits.
 */
t_bitmap *bitmap_create(size_t size);

/**
 * Crea un mapa de bits cargándolo desde un archivo.
 * Lo mantiene sincronizado con el archivo.
 * @param size Tamaño del mapa de bits (en bits).
 * @param path Ruta al archivo.
 * @return Mapa de bits.
 */
t_bitmap *bitmap_load(size_t size, const char *path);

/**
 * Pone en 1 un bit determinado del mapa de bits.
 * @param bitmap Mapa de bits.
 * @param bit Bit a poner en 1.
 */
void bitmap_set(t_bitmap *bitmap, off_t bit);

/**
 * Pone en 0 un bit determinado del mapa de bits.
 * @param bitmap Mapa de bits.
 * @param bit Bit a poner en 0.
 */
void bitmap_unset(t_bitmap *bitmap, off_t bit);

/**
 * Verifica el valor de un bit del mapa de bits.
 * @param bitmap Mapa de bits.
 * @param bitmap Bit a verificar.
 * @return Valor del bit.
 */
bool bitmap_test(t_bitmap *bitmap, off_t bit);

/**
 * Devuelve la posición del primer 0 dentro del mapa.
 * @param bitmap Mapa de bits.
 * @return Posición del primer 0.
 */
off_t bitmap_firstzero(t_bitmap *bitmap);

/**
 * Devuelve la posición del primer 1 dentro del mapa.
 * @param bitmap Mapa de bits.
 * @return Posición del primer 1.
 */
off_t bitmap_firstone(t_bitmap *bitmap);

/**
 * Pone en 0 todos los bits del mapa de bits.
 * @param bitmap Mapa de bits.
 */
void bitmap_clear(t_bitmap *bitmap);

/**
 * Destruye el mapa de bits.
 * @param bitmap Mapa de bits.
 */
void bitmap_destroy(t_bitmap *bitmap);

#endif /* BITMAP_H_ */

#ifndef NODELIST_H_
#define NODELIST_H_

#include <socket.h>
#include <stdbool.h>
#include <thread.h>
#include <bitmap.h>
#include "yfile.h"

typedef struct t_node {
	char *name;
	int total_blocks;
	int free_blocks;
	bool available;
	thread_t *handler;
	t_socket socket;
	t_bitmap *bitmap;
} t_node;

/**
 * Esta estructura se mantiene sincronizada y almacenada en un archivo
 * ubicado en la ruta ~/yatpos/metadata/nodos.bin.
 */

/**
 * Inicializa la lista de nodos, cargando un estado anterior si existía.
 */
void nodelist_init(void);

/**
 * Devuelve la cantidad de nodos en la lista de nodos.
 * @return Cantidad de nodos.
 */
int nodelist_size();

/**
 * Crea un nuevo nodo y lo agrega a la lista de nodos.
 * @param name Nombre del nodo a crear.
 * @param blocks Cantidad de bloques del nodo.
 * @param handler Hilo manejador del nodo.
 * @return Puntero al nodo creado.
 */
t_node *nodelist_add(const char *name, int blocks, thread_t *handler);

/**
 * Busca un nodo en la lista de nodos según su ID.
 * @param name Nobre del nodo.
 * @return Puntero al nodo.
 */
t_node *nodelist_find(const char *name);

/**
 * Agrega un bloque a nodos de la lista.
 * @param block Bloque a agregar.
 */
void nodelist_addblock(t_block *block);

/**
 * Elimina un nodo de la lista de nodos.
 * @param name Nombre del nodo a eliminar.
 */
void nodelist_remove(const char *name);

/**
 * Elimina todos los nodos de la lista de nodos.
 */
void nodelist_clear();

/**
 * Imprime por pantalla la lista de nodos.
 */
void nodelist_print();

/**
 * Cierra la lista de nodos.
 */
void nodelist_term();

#endif /* NODELIST_H_ */

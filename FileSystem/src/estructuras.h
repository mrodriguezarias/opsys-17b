#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include <commons/collections/list.h>
#include <commons/config.h>
#include <stdbool.h>
#include <serial.h>

struct t_directory {
	int index;
	char nombre[255];
	int padre;
};

typedef struct {
	char* nombreNodo;
	int total;
	int libre;
} Nodo;

typedef struct{
	int tamanio;
	int libre;
	t_list* nodos;
}t_tablaNodos;

typedef struct{
	long tamanio;
	char* tipo;
	t_list* bloques;
}t_Archivo;

typedef struct {
	char* copia0[2]; //primer posicion para el numero de nodo, segunda posicion numero de bloque dentro del nodo
	char* copia1[2];
	long bytes;
}bloque;

typedef struct{
	char* nombreNodo;
	int bloquesTotales;
	int bloquesLibres;
}nodo_unpack;

struct t_directory tablaDirectorio[100];
t_config* archivoDirectorio;
t_config* archivoNodos;
t_list* listaArchivosYamaFS;
t_tablaNodos tablaNodos;
t_list* listaBitmaps;
char* busquedaNodo_GLOBAL;
t_list* listaNodosConectados;
bool estadoAnteriorexistente;

Nodo* infoNodo_unpack(t_serial);
void inicializarEstructurasFilesystem();
void guardarEstructuras();
char *config_file(char*,char*);
void crearDirectorio(int,char*);
void inicializarTablaArchivos();
void inicializarTablaDirectorio();
int encontrarIndexDelArchivo();
void inicializarTablaNodos();
void inicializarBitmap();
void inicializarNodo(int);
void registrarNodo(Nodo*,int);
bool encontreNodo(void*);
char* generarBitmap(int);
char* pasarlistaDenodosAChar(t_list*);

#endif /* ESTRUCTURAS_H_ */

#ifndef SYSTEM_H_
#define SYSTEM_H_

/**
 * Devuelve el directorio home del usuario.
 * (En la VM de la cátedra, va a ser siempre /home/utnso)
 * @return Directorio home.
 */
const char *system_homedir(void);

/**
 * Devuelve el directorio base del sistema (tp-2017-2c-YATPOS).
 * @return Directorio del sistema.
 */
const char *system_basedir(void);

/**
 * Devuelve el directorio de usuario del sistema (/home/utnso/yatpos).
 * @return Directorio de usuario.
 */
const char *system_userdir(void);

/**
 * Devuelve la ruta al directorio de recursos (Shared/rsc).
 * @return Directorio de recursos.
 */
const char *system_rscdir(void);

/**
 * Hace una ruta relativa al directorio de usuario.
 * La cadena devuelta debe ser liberada con free().
 * @param path Ruta de un archivo.
 * @return Ruta relativa al directorio de usuario.
 */
char *system_upath(const char *path);

/**
 * Hace una ruta relativa al directorio del ejecutable.
 * La cadena devuelta debe ser liberada con free().
 * @param path Ruta de un archivo.
 * @return Ruta relativa al directorio del ejecutable.
 */
char *system_lpath(const char *path);

/**
 * Crea todos los directorios del sistema.
 * Solo debería llamarse por process_init().
 */
void system_init(void);

/**
 * Devuelve la ruta absoluta del ejecutable.
 * @return Ruta del ejecutable.
 */
const char *system_proc(void);

/**
 * Devuelve el directorio actual donde se ejecutó el proceso.
 * @return Ruta al directorio actual.
 */
const char *system_cwd(void);

/**
 * Devuelve un número aleatorio.
 * @return Número entero aleatorio entre 0 y RAND_MAX.
 */
int system_rand(void);

/**
 * Termina la ejecución del proceso corriendo.
 * Opcionalmente imprime por pantalla un mensaje de error.
 * @param error Mensaje de error (puede ser NULL o vacío).
 */
void system_exit(const char *error, ...);

#endif /* SYSTEM_H_ */

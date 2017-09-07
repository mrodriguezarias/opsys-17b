#ifndef LOG_H_
#define LOG_H_

// Combinaciones posibles:
// logXY(...)
// X = [i: info; e: error]
// Y = [f: file; p: print]

/**
 * Escribe un mensaje en el archivo de log del proceso.
 * El mensaje se lo considera un mensaje informativo.
 * El mensaje no se imprime por pantalla.
 * @param format Formato del mensaje.
 */
void logif(const char *format, ...);

/**
 * Escribe un mensaje en el archivo de log del proceso.
 * El mensaje se lo considera un mensaje de error.
 * El mensaje no se imprime por pantalla.
 * @param format Formato del mensaje.
 */
void logef(const char *format, ...);

/**
 * Escribe un mensaje en el archivo de log del proceso.
 * El mensaje se lo considera un mensaje informativo.
 * El mensaje además se imprime por pantalla.
 * @param format Formato del mensaje.
 */
void logip(const char *format, ...);

/**
 * Escribe un mensaje en el archivo de log del proceso.
 * El mensaje se lo considera un mensaje de error.
 * El mensaje además se imprime por pantalla.
 * @param format Formato del mensaje.
 */
void logep(const char *format, ...);

#endif /* LOG_H_ */

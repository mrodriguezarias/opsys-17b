#ifndef LOG_H_
#define LOG_H_

/**
 * Escribe un mensaje en el archivo de log del proceso.
 * El mensaje se lo considera un mensaje informativo.
 * El mensaje no se imprime por pantalla.
 * @param format Formato del mensaje.
 */
void log_inform(const char *format, ...);

/**
 * Escribe un mensaje en el archivo de log del proceso.
 * El mensaje se lo considera un mensaje informativo.
 * El mensaje además se imprime por pantalla.
 * @param format Formato del mensaje.
 */
void log_print(const char *format, ...);

/**
 * Escribe un mensaje en el archivo de log del proceso.
 * El mensaje se lo considera un mensaje de error.
 * El mensaje además se imprime por pantalla.
 * @param format Formato del mensaje.
 */
void log_report(const char *format, ...);

#endif /* LOG_H_ */

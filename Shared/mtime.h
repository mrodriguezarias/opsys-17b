#ifndef MTIME_H_
#define MTIME_H_

#define MTIME_DATE 0x1 // Agrega la fecha
#define MTIME_PRECISE 0x2 // Agrega los milisegundos
#define MTIME_DIFF 0x4 // Para diferencias de tiempo

typedef unsigned long long mtime_t;

/**
 * Devuelve el instante de tiempo actual.
 * @return Instante de tiempo actual.
 */
mtime_t mtime_now(void);

/**
 * Calcula la diferencia entre dos instantes de tiempo.
 * @param t1 Instante de tiempo 1.
 * @param t2 Instante de tiempo 2.
 * @return Diferencia de tiempo.
 */
mtime_t mtime_diff(mtime_t t1, mtime_t t2);

/**
 * Crea una cadena con la fecha de un instante de tiempo.
 * Formato: YYYY-MM-DD
 * @param time Instante de tiempo.
 * @return Cadena con la fecha (a liberar).
 */
char *mtime_date(mtime_t time);

/**
 * Crea una cadena con la hora de un instante de tiempo.
 * Formato: hh:mm:ss
 * @param time Instante de tiempo.
 * @return Cadena con la hora (a liberar).
 */
char *mtime_time(mtime_t time);

/**
 * Crea una cadena con la fecha y hora de un instante de tiempo.
 * Formato: YYYY-MM-DD hh:mm:ss
 * @param time Instante de tiempo.
 * @return Cadena con la fecha y hora (a liberar).
 */
char *mtime_datetime(mtime_t time);

/**
 * Devuelve una cadena con un instante de tiempo formateado.
 * Formato: [YYYY-MM-DD ]hh:mm:ss[.SSS]
 * @param time Instante de tiempo.
 * @param mode Máscara de bits para definir el formato.
 * @return Cadena con el tiempo formateado (a liberar).
 */
char *mtime_formatted(mtime_t time, int mode);

/**
 * Imprime un instante de tiempo.
 * Formato: [YYYY-MM-DD ]hh:mm:ss[.SSS]
 * @param time Instante de tiempo.
 * @param mode Máscara de bits para definir el formato.
 */
void mtime_print(mtime_t time, int mode);

#endif /* MTIME_H_ */

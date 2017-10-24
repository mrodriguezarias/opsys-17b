#ifndef NUMBER_H_
#define NUMBER_H_

typedef double t_number;

#include <stdbool.h>

/**
 * Redondea un número hacia el entero superior más cercano.
 * @param x Número a redondear hacia arriba.
 * @return Número redondeado.
 */
t_number number_ceiling(t_number x);

/**
 * Determina si un número es igual a otro.
 * @param x Un número.
 * @param y Otro número.
 * @return Valor lógico indicando si los números son iguales.
 */
bool number_equals(t_number x, t_number y);

#endif /* NUMBER_H_ */

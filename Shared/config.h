#ifndef MCONFIG_H_
#define MCONFIG_H_

#include <commons/config.h>

/**
 * Carga la configuración del proceso en ejecución.
 * @return Estructura de configuración.
 */
void config_load(void);

/**
 * Obtiene una propiedad de la configuración del proceso actual.
 * @param property Propiedad a leer.
 * @return Valor de la propiedad.
 */
const char *config_get(const char *property);

#endif /* MCONFIG_H_ */

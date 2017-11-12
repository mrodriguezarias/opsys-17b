# tp-2017-2c-YATPOS

Sistema de archivos distribuido **YAMA** (**Y**et **A**nother **M**r. **A**dministrator) desarrollado por el Equipo **YATPOS** (**Y**et **A**gain **T**rying to **P**ass **O**perating **S**ystems).

## Especificación

La especificación detallada del sistema se puede consultar en el [enunciado](https://sisoputnfrba.gitbooks.io/yama-tp-2c2017/) del trabajo práctico.

## Dependencias

### libcommons

Biblioteca _commons_ de la cátedra.

Para instalar, clonar el [repositorio](https://github.com/sisoputnfrba/so-commons-library) y ejecutar `sudo make install`. Para enlazar, agregar `-lcommons` a los parámetros de compilación.

### libreadline

Biblioteca _readline_ de GNU, usada para la consola del **FileSystem**.

Para instalar, ejecutar `sudo apt install libreadline6 libreadline6-dev`. Para enlazar, agregar `-lreadline` a los parámetros de compilación.

### libssl

Biblioteca _ssl_ de OpenSSL, usada para calcular el MD5 de los archivos del **FileSystem**.

Para instalar, ejecutar `sudo apt install libssl-dev`. Para enlazar, agregar `-lssl -lcrypto` a los parámetros de compilación.

## Instalación

Ejecutar la siguiente línea en una terminal:

`curl -sL bit.ly/2hzgEGg | bash -s {argumento}`

El argumento puede ser alguno de los siguientes, según el caso:

- **all**: instala todo
- **node**: instala solo DataNode y Worker
- **fs**: instala solo FileSystem
- **yama**: instala solo YAMA
- **master**: instala solo Master
- **dnode**: instala solo DataNode
- **worker**: instala solo Worker
- **uninstall**: desinstala todo

Luego de instalar, ejecutar `fs`/`yama`/`master`/`dnode`/`worker` para correr cada proceso.

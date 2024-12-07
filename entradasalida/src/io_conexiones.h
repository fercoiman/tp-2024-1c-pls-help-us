#ifndef IO_CONEXIONES_H_
#define IO_CONEXIONES_H_
#include <stdlib.h>
#include <utils/conexiones.h>

extern t_log* IO_logger;
extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;
extern char* IP_KERNEL;
extern char* PUERTO_KERNEL;
int socket_kernel;
int socket_memoria;

int conectar_io_kernel();
int conectar_io_memoria();


#endif
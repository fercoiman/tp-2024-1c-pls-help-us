#ifndef CONEXIONES_MEMORIA_H_
#define CONEXIONES_MEMORIA_H_
#include <pthread.h>
#include <utils/estructuras.h>
#include <utils/conexiones.h>
#include <commons/string.h>
extern char* PUERTO_ESCUCHA;
extern t_log* memoria_logger;
extern t_dictionary* lista_interfaz;
//las genericas no se conectan a memoria por eso no tiene t_list* generica
extern t_list* lista_stdin;
extern t_list* lista_stdout;
extern t_list* lista_dialfs;
int cliente_cpu;
int cliente_kernel;

void esperar_conexiones();
int esperar_cpu();
int esperar_kernel();


#endif
#ifndef KERNEL_CONEXIONES_H_
#define KERNEL_CONEXIONES_H_

#include <stdlib.h>
#include <utils/conexiones.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <pthread.h>

 
extern char* PUERTO_ESCUCHA;
extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;
extern char* IP_CPU;
extern char* PUERTO_CPU_DISPATCH;
extern char* PUERTO_CPU_INTERRUPT;
extern char* ALGORITMO_PLANIFICACION;
extern int QUANTUM;
extern int GRADO_MULTIPROGRAMACION;
extern char** RECURSOS;
extern char** INSTANCIAS_RECURSOS;

extern t_log* kernel_logger;
extern t_config* kernel_config;
extern t_dictionary* lista_interfaz;
t_list* lista;
//conexiones
int conectar_memoria();
int conectar_cpu_dispatch();
int conectar_cpu_interrupt();
void* esperar_io(void* args);
void manejar_peticiones_io(int);
void selector(int ,t_log*);
char* registrar_interfaz(int,conexion_interfaz*);
int crear_conexion(char *ip, char* puerto);

#endif

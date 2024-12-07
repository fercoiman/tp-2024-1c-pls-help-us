#ifndef INICIALIZAR_KERNEL_H_
#define INICIALIZAR_KERNEL_H_
#include <stdlib.h>
#include <stdio.h>
#include <utils/conexiones.h>
#include <commons/txt.h>
#include <commons/log.h>
#include <commons/config.h>

/* Variables globales*/
t_log* kernel_logger;
t_config* kernel_config;

char* PUERTO_ESCUCHA;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* IP_CPU;
char* PUERTO_CPU_DISPATCH;
char* PUERTO_CPU_INTERRUPT;
char* ALGORITMO_PLANIFICACION;
int QUANTUM;
int GRADO_MULTIPROGRAMACION;
char** RECURSOS;
char** INSTANCIAS_RECURSOS;
void inicializar_kernel();
void inicializar_loggers_kernel();
void inicializar_configs_kernel();


#endif

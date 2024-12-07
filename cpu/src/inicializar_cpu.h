#ifndef INICIALIZAR_CPU_H_
#define INICIALIZAR_CPU_H_
#include "cpu.h"
char *IP_MEMORIA;
char *PUERTO_MEMORIA;
char *PUERTO_CPU_DISPATCH;
char *PUERTO_CPU_INTERRUPT;
int CANTIDAD_ENTRADAS_TLB;
char *ALGORITMO_TLB;

t_log* cpu_logger;
t_config* cpu_config;
t_list* lista;
extern int conexion_memoria;
void inicializa_cpu();
void inicializar_loggers_cpu();
void inicializar_configs_cpu();

#endif
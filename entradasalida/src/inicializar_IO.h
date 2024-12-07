#ifndef INICIALIZAR_IO_H_
#define INICIALIZAR_IO_H_

#include <stdlib.h>
#include <stdio.h>
#include <commons/txt.h>
#include <commons/log.h>
#include <commons/config.h>

t_log* IO_logger;
t_config* IO_config;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* IP_KERNEL;
char* PUERTO_KERNEL;

void inicializa_IO();
void inicializar_loggers_IO();
void inicializar_configs_IO();

#endif

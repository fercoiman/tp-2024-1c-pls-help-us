#ifndef INICIALIZAR_MEMORIA_H_
#define INICIALIZAR_MEMORIA_H_

#include <stdlib.h>
#include <stdio.h>
#include <utils/conexiones.h>
#include <commons/txt.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
extern t_log* memoria_logger;
extern t_config* memoria_config;
extern int TAM_MEMORIA;
extern int TAM_PAGINA;
extern int RETARDO_RESPUESTA;
extern char* PUERTO_ESCUCHA;
extern char* PATH_SCRIPTS;
extern t_dictionary* tabla_paginas;
int cantidad_marcos;
char* bitmap_marcos;
void *memoria;
void inicializa_memoria();
void inicializar_loggers_memoria();
void inicializar_configs_memoria();
void paginar_memoria();
void marcar_marco_ocupado(int);
void marcar_marco_libre(int);
int esta_marco_ocupado(int);
int asignar_marco(uint32_t,uint32_t);
// void imprimirPrueba();

#endif

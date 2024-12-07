#ifndef CPU_H_
#define CPU_H_

#include <utils/estructuras.h>
#include <stdlib.h>
#include <stdio.h>
#include <utils/conexiones.h>
#include <commons/txt.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/dictionary.h>
#include <pthread.h>
#include "cpu_operaciones_instrucciones.h"
#include "cpu_ciclo_instruccion.h"

t_instruccion* recibir_instruccion_memoria(uint32_t, uint32_t);
 void *atender_interrupciones(void *);
 void atender_peticiones_instrucciones();
 void iterator(char*);
 int conectar_cpu_memoria();
 extern t_log* cpu_logger;
 extern t_config* cpu_config;
 extern t_list* lista;
 extern char *IP_MEMORIA;
 extern char *PUERTO_MEMORIA;
 extern char *PUERTO_CPU_DISPATCH;
 extern char *PUERTO_CPU_INTERRUPT;
 extern int conexion_memoria;
 extern bool continuar_ciclo_instruccion;
 void inicializa_cpu();
//void manejar_peticiones_instruccion();
//definida aca
//void recibir_interrupcion(int socket_kernel);
//void devolver_a_kernel(t_contexto_ejec* contexto, op_code code, int socket_cliente);
void agregar_a_paquete(t_paquete* , void* , int);
int enviar_paquete(t_paquete* , int );
t_contexto_ejec* recibir_contexto(int);

#endif

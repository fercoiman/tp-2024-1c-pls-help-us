#ifndef _KERNEL_PLANIFICADOR_LARGO_PLAZO_H
#define _KERNEL_PLANIFICADOR_LARGO_PLAZO_H

//#include "kernel_consola.h"
#include <commons/collections/list.h>
#include <semaphore.h>
#include <utils/estructuras.h>
#include <commons/log.h>
#include <commons/config.h>
#include <pthread.h>
#include <commons/string.h>
#include <pthread.h>


extern t_log* kernel_logger;
extern t_config* kernel_config;
extern int QUANTUM;
extern int GRADO_MULTIPROGRAMACION;
extern sem_t inicializar_planificador;
extern sem_t mutex_lista_new;
extern sem_t mutex_lista_ready;
extern sem_t mutex_lista_exit;
extern sem_t mutex_lista_exec;
extern sem_t mutex_lista_blocked;
extern sem_t grado_multiprogramacion;
extern sem_t proceso_ejecutando;
extern sem_t proceso_finalizado;
extern sem_t hay_proceso_ready;


t_recurso *recurso_new(char *);

 
void inicializar_listas();
void planificar();
void pasar_new_a_ready();
void pasar_new_a_exit();
void pasar_ready_a_exit();
void pasar_ready_a_execute();
void pasar_blocked_a_exit();
void pasar_blocked_a_ready();
void pasar_execute_a_blocked();
void pasar_execute_a_ready();
void pasar_execute_a_exit();
void mover_procesos(t_list* lista_origen, t_list* lista_destino, sem_t* sem_origen, sem_t* sem_destino, t_estado nuevo_estado);
void* planificar_procesos();

void agregar_a_cola(t_pcb *pcb,t_list* lista,sem_t* sem);
void agregar_a_new(t_pcb *pcb);
void agregar_a_ready(t_pcb *pcb);
void agregar_a_exit(t_pcb *pcb);
void finalizar_proceso(int);
void finalizar_proceso_execute();
void buscar_en_lista_y_finalizar(char* key, void* lista);

void imprimir_recurso(t_recurso* );
#endif

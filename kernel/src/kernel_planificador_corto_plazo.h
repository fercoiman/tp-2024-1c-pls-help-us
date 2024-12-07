#ifndef _KERNEL_PLANIFICADOR_CORTO_PLAZO_H
#define _KERNEL_PLANIFICADOR_CORTO_PLAZO_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <string.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <utils/estructuras.h>
#include <utils/conexiones.h>
#include <time.h>

extern int socket_conexion_cpu_dispatch;
extern int socket_conexion_cpu_interrupt;
extern int GRADO_MULTIPROGRAMACION;
extern t_log * kernel_logger;
extern t_config* kernel_config;
extern char* ALGORITMO_PLANIFICACION;
extern int QUANTUM;

extern t_list* lista_ready; 
extern t_list* lista_exec;
extern t_list* lista_blocked;
extern t_list* lista_exit;
extern t_list* lista_new;

sem_t iniciar_planificador;
sem_t sem_hay_proceso_en_new;
sem_t sem_ready;
sem_t sem_fin_proceso;
sem_t sem_block;
sem_t sem_hay_proceso_esperando_ready;

extern sem_t proceso_ejecutando;
extern sem_t mutex_lista_new;
extern sem_t mutex_lista_ready;
extern sem_t mutex_lista_exit;
extern sem_t mutex_lista_exec;
extern sem_t mutex_lista_blocked;
extern sem_t grado_multiprogramacion;
extern sem_t inicializar_planificador;
extern sem_t hay_proceso_ready;
extern sem_t quantum;
extern sem_t proceso_finalizado;
extern sem_t continuar_planificacion;
extern sem_t proceso_interrumpido;
extern sem_t proceso_blocked;
extern sem_t empezar_tiempo_cpu;
extern sem_t terminar_tiempo_cpu;
extern sem_t iniciar_quantum;
extern sem_t actualizar_contexto_sem;

// Definiciones Hilos
pthread_t hilo_fifo;
pthread_t hilo_round_robin;
pthread_t hilo_virtual_round_robin;
pthread_t hilo_quantum;
pthread_t hilo_verificacion;
pthread_t hilo_tiempo_cpu;

void mostrar_procesos();

// u_int32_t running_process_id;

void crear_contexto_y_enviar_a_CPU(t_pcb* );
void poner_a_ejecutar(t_pcb* );
void iniciar_planificacion();
void agregar_a_ready();
void agregar_a_new(t_pcb*);
void agregar_a_blocked();
void agregar_a_running();
void pasar_ready_a_execute();
void pasar_execute_a_ready();
void pasar_execute_a_blocked();
void pasar_blocked_a_ready();
void mover_procesos(t_list* lista_origen, t_list* lista_destino, sem_t* sem_origen, sem_t* sem_destino, t_estado nuevo_estado);
t_pcb* obtener_siguiente_en_ready();
void* planificar_round_robin();
void* planificar_virtual_round_robin();
void inicializar_hilos_planificador();
int interrumpir(t_pcb*); //supongo que interrumpe , en el .c necesitamos definir que hace
int agregar_estado_ready(t_pcb*);//supongo que agrega a ready , en el .c necesitamos definir que hace
                                //creo que esta operacion cambia solo el campo estado del PCB en
                                //la funcion pasar_a_ready es la que mueve entre colas  VERRRRRR!!!!
void* planificar_fifo();
t_pcb* obtener_siguiente_en_ready();
void enviar_contexto_de_ejecucion_a(t_contexto_ejec* , op_code , int );
uint32_t obtener_pid(void);
t_pcb* crear_pcb(void);
void solicitar_creacion_memoria(t_pcb*);
void enviar_proceso_cpu (t_pcb*);
int enviar_paquete(t_paquete* paquete, int socket_destino);
t_paquete* crear_paquete(op_code codigo_operacion);
void eliminar_paquete(t_paquete* paquete);
void crear_hilo_planificador_fifo();
void crear_hilo_planificador_rr();
void crear_hilo_planificador_vrr();
void crear_hilo_quantum();
void finalizar_hilo_quantum();
void* esperar_quantum();
void finalizar_proceso_quantum();

void crear_hilo_verificacion();
void finalizar_hilo_verificacion();
void* verificacion_fin_proceso();
void* tiempo_en_cpu();
void crear_hilo_tiempo_cpu();
void finalizar_hilo_tiempo_cpu();

void actualizar_contexto_execute(uint32_t , t_registros_cpu* , uint32_t );
#endif

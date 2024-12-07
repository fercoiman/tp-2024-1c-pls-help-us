#ifndef KERNEL_CONSOLA_H_
#define KERNEL_CONSOLA_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <semaphore.h>
#include <utils/estructuras.h>
#include <math.h>
#include <utils/conexiones.h>
//solo para mostrar
void mostrar_encabezado();
 
// Variables globales externas
extern t_log* kernel_logger;
extern t_config* kernel_config;
extern int QUANTUM;
extern t_list* lista_new;
extern int socket_conexion_memoria;
extern int GRADO_MULTIPROGRAMACION;
extern char* ALGORITMO_PLANIFICACION;
// Semáforos
sem_t mutex_lista_new;
sem_t mutex_lista_ready;
sem_t mutex_lista_exit;
sem_t mutex_lista_exec;
sem_t mutex_lista_blocked;
sem_t proceso_ejecutando;
sem_t inicializar_planificador;
sem_t actualizar_contexto_sem;
sem_t hay_proceso_ready;
sem_t quantum;
sem_t proceso_finalizado;
sem_t continuar_planificacion;
sem_t proceso_interrumpido;
sem_t proceso_blocked;
sem_t empezar_tiempo_cpu;
sem_t terminar_tiempo_cpu;
sem_t iniciar_quantum;


extern t_dictionary* tabla_procesos;
extern t_list* lista_new;
extern t_list* lista_ready;
extern t_list* lista_exec;
extern t_list* lista_blocked;
extern t_list* lista_exit;

int init_consola(void);
void listar_procesos_por_estado();
void iterator (char* key,void* elemento);
t_comando* armar_comando(char* linea);
t_instruccion* armar_instruccion(char *linea);
void iniciar_proceso(char *path);
void finalizar_proceso(uint32_t pid);
void detener_planificacion(void);
void actualizar_grado_multiprogramacion(int);
void agregar_a_new(t_pcb *pcb);
void destruir_comando(t_comando *);
void inicializar_semaforos(void);
void inicializar_pid_global(void);
void ejecutar_script(char*);
uint32_t obtener_pid(void);
t_pcb* crear_pcb(void);
void solicitar_creacion_memoria(t_pcb*);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
int enviar_paquete(t_paquete* paquete, int socket_destino);
t_paquete* crear_paquete(op_code codigo_operacion);
void eliminar_paquete(t_paquete* paquete);
void mostrar_procesos();
void pasar_new_a_ready();
void planificar_fifo();
void planificar();
void* planificar_procesos();
void inicializar_hilos_planificador();
void iniciar_planificador_largo_plazo();

#endif 

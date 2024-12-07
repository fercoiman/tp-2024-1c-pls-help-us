#ifndef KERNEL_H_
#define KERNEL_H_
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <utils/estructuras.h>
#include <utils/conexiones.h>
//#include "kernel_planificador_largo_plazo.h"
//#include "kernel_consola.h"

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
extern t_list* lista;
//DICCIONARIOS
extern t_dictionary* tabla_procesos;
extern t_dictionary* matriz_recursos_asignados;
extern t_dictionary* matriz_recursos_pendientes;
extern t_dictionary* matriz_recursos_asignados_x_pid;
extern t_dictionary* lista_interfaz;
//LISTAS
extern t_list* lista_new;
extern t_list* lista_ready;
extern t_list* lista_exec;
extern t_list* lista_blocked;
extern t_list* lista_exit;
extern t_list* lista_generica;
extern t_list* lista_stdin;
extern t_list* lista_stdout;
extern t_list* lista_dialfs;
extern t_list *recursos_totales;

extern int servidor_kernel;
int socket_conexion_memoria;
int socket_conexion_cpu_dispatch;
int socket_conexion_cpu_interrupt;
int socket_conexion_io;

void inicializar_kernel();
void planificar_fifo();
int iniciar_kernel_servidor();
void iniciar_proceso(char* path);
void poner_a_ejecutar(t_pcb* );
void inicializar_listas();
void inicializar_semaforos();
void inicializar_hilos_planificador();
void init_consola();
void* esperar_io(void* args);
void* manejar_peticiones_cpu_dispatch(void*);
void* manejar_peticiones_cpu_interrupt(void*);
void* manejar_peticiones_memoria(void*);
int conectar_cpu_interrupt();
int conectar_memoria();
int conectar_cpu_dispatch();

//Liberar memoria
void t_instruccion_destroyer(void* instruccion);
void pcb_destroyer(void* element);
void list_destroyer(void* list);
void dictionary_destroy_and_destroy_list_elements(t_dictionary* dictionary);
#endif

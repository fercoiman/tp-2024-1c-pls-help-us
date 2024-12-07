#ifndef _OPERACIONES_H
#define _OPERACIONES_H
#include <utils/estructuras.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <commons/string.h>
#include <utils/conexiones.h>
#include <semaphore.h>
#include <pthread.h>
extern char* ALGORITMO_PLANIFICACION;
extern t_log * kernel_logger;
extern t_list* recursos_totales;
extern t_dictionary* matriz_recursos_asignados;
extern t_dictionary* matriz_recursos_pendientes;
extern t_dictionary* matriz_recursos_asignados_x_pid;
extern t_dictionary* tabla_procesos;
//IOS
extern t_dictionary* lista_interfaz;
extern t_list* lista_generica;
extern t_list* lista_stdin;
extern t_list* lista_stdout;
extern t_list* lista_dialfs;
//
extern int socket_conexion_memoria;
extern int socket_conexion_cpu_interrupt;
extern int socket_io;
extern sem_t actualizar_contexto_sem;
extern sem_t proceso_ejecutando;
extern sem_t hay_proceso_ready;
extern sem_t continuar_planificacion;
extern sem_t proceso_finalizado;
extern sem_t actualizar_contexto_sem;

//Manejador de recursos
t_recurso* obtener_recurso(char* );
t_recurso *recurso_new(char *);
bool verificar_existencia_recurso(char*);
void  recibir_recurso_solicitado(int);
void asignar_recurso(uint32_t , char*);
void desalojar_recurso(uint32_t , char*);
void imprimir_recurso(t_recurso* );
//
void actualizar_contexto(t_contexto_ejec*);
void manejar_peticiones_io(int );    
void* manejar_peticiones_cpu_dispatch(void*);
void* manejar_peticiones_cpu_interrupt(void*);
void* manejar_peticiones_memoria(void*);
void finalizar_proceso_execute();


t_contexto_ejec* recibir_contexto(int);
void buscar_en_lista_y_actualizar(char* , void* );
void pasar_execute_a_ready();
void pasar_execute_a_blocked();
void pasar_execute_a_exit();
void pasar_blocked_a_exit();
void pasar_blocked_a_ready();
char* registrar_interfaz(int,conexion_interfaz*);
void mostrar_procesos();
//IOS
void recibir_pedido_IO(int);
void solicitar_a_io(t_contexto_ejec*,op_code);
bool verificar_existencia_io(char* ,char*);
void* esperar_respuesta_io(void*);
//
void actualizar_contexto_execute(uint32_t , t_registros_cpu*, uint32_t);
void recibir_y_actualizar(int);

void finalizar_hilo_quantum();
#endif

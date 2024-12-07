#ifndef MEMORIA_H
#define MEMORIA_H
#include <pthread.h>
#include <utils/estructuras.h>
#include <utils/conexiones.h>
#include <commons/string.h>
extern int cliente_cpu;
extern int cliente_kernel;
extern int cliente_io;
extern int cantidad_marcos;
extern char* bitmap_marcos;
extern void* memoria;
t_log* memoria_logger;
t_config* memoria_config;
t_list* lista;
t_dictionary* lista_instrucciones_PID;
char* path_instrucciones;
char* PUERTO_ESCUCHA;
int TAM_MEMORIA;
int TAM_PAGINA;
int RETARDO_RESPUESTA;
char* PATH_SCRIPTS;

extern void* memoria;
void* esperar_io(void*);
char* registrar_interfaz(int);
void enviar_instruccion_a_cpu(int);
void retraso_respuesta(int);
void marcar_marco_ocupado(int );
//void enviar_instruccion(t_instruccion* , int );
void mostrar_estado_marcos();
int asignar_marco(uint32_t,uint32_t);
void agregar_a_lista_x_pid(t_list* lista_instrucciones, int pid);
void mostrar_lista_instrucciones_x_pid();
void interpretar_pseudocodigo(char*,uint32_t);
void esperar_conexiones();
void selector(int,t_log*);
void *atender_clientes(void *);
void iterator(char*,void*);
uint32_t recibir_pid(int);
void manejar_solicitud_creacion_proceso(int);
t_pcb* recibir_pcb(t_paquete*);
void acceso_pagina(int);
void retardo_respuesta(int ) ;
void enviar_respuesta(int , uint32_t );
void recibir_pagina(int , uint32_t* , uint32_t* ) ;
void crear_tabla_paginas(uint32_t);
void eliminar_paquete(t_paquete* );
void buffer_read(void*, t_buffer* , uint32_t );
void agregar_a_paquete(t_paquete*, void* , int );
t_paquete* crear_paquete(op_code );
// Función auxiliar de selección de operación
uint32_t recibir_pid(int);
uint32_t recibir_pc(int);
int recibir_operacion(int );
char* recibir_mensaje(int );
void eliminar_paquete(t_paquete* );
void inicializa_memoria(void);
void recibir_programcounter(int , uint32_t *,uint32_t* );

void escribir_en_memoria(int);
void leer_de_memoria(int);
void resize_memory(int);

//LIBERAR MEMORIA
void handle_kernel_disconnection();
void dictionary_destroy_and_destroy_list_elements(t_dictionary* dictionary); 
void list_destroyer(void* list);  
void t_instruccion_destroyer(void* instruccion);
void liberar_lista_instrucciones(char* key, void* element);
void liberar_memoria_instrucciones();
#endif
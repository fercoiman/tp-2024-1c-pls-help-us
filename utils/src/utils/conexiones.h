#ifndef _CONEXIONES_H_
#define _CONEXIONES_H_

#include <sys/types.h>
#include<assert.h>
#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include <readline/readline.h>
#include<commons/log.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include <utils/estructuras.h>


typedef struct {
    uint32_t size; // Tamaño del payload
    uint32_t offset; // Desplazamiento dentro del payload
    void* stream; // Payload
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;
void recibir_handshake(int);
//PROTOTIPOS PARA CLIENTE
void* serializar_paquete(t_paquete* , int );
int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char*, int , op_code );
void paquete(int);
//t_paquete* crear_paquete(op_code codigo);
t_buffer* buffer_create(uint32_t);
void buffer_destroy(t_buffer*);
void buffer_add(t_buffer* , void* , uint32_t);
//void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
//void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
//void eliminar_paquete(t_paquete* paquete);
void terminar_programa(t_log* logger, t_config* config);

//PROTOTIPOS PARA SERVIDOR
t_instruccion* deserializar_instruccion_en(void* , int*);
t_instruccion* recibir_instruccion(int);
void recibir_program_counter(int , uint32_t *,uint32_t* );
void* recibir_buffer(int*, int);
int iniciar_servidor(char*, t_log*,char*);
int esperar_cliente(int, t_log*,char*);
t_paquete* recibir_paquete(int);
//void buffer_read(void*, t_buffer*, uint32_t );
char* recibir_mensaje(int);
void recibir_path_y_pid(int , char **, int *);
int recibir_operacion(int);
void liberar_instruccion(t_instruccion*);
void liberar_contexto(t_contexto_ejec*);
void agregar_a_paquete(t_paquete* , void* , int );
int enviar_paquete(t_paquete*, int);
t_paquete* crear_paquete(op_code );
void eliminar_paquete(t_paquete*);
void buffer_read(void*, t_buffer*, uint32_t);

#endif

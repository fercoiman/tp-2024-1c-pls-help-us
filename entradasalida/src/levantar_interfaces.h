#ifndef LEVANTAR_INTERFACES_H_
#define LEVANTAR_INTERFACES_H_
#include <utils/estructuras.h>
#include <commons/log.h>
#include <commons/config.h>

extern t_log* IO_logger;
t_config* archivo_config;
extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;
extern char* IP_KERNEL;
extern char* PUERTO_KERNEL;
char* TIPO_INTERFAZ;
int TIEMPO_UNIDAD_TRABAJO;
char* PATH_BASE_DIALFS;
int BLOCK_SIZE;
int BLOCK_COUNT;
int RETRASO_COMPACTACION;
//lee el archivo config y el nombre , y se la envia a la funcion correspondiente para crearla (depende del tipo de interfaz que haya leido del config)
void iniciar_interfaz(char* nombre, char* nombre_archivo);
//funcion mostrar_interfaces no es necesaria pero sirve para ver cuales se iniciaron correctamente
void conectar_interfaz_kernel(char* , char* );
void conectar_interfaz_memoria(char* , char* );
//funciones para crear una nueva interfaz segun su tipo
interfaz_generica* inicializar_generica(char*);
interfaz_stdin* inicializar_stdin(char*);
interfaz_stdout* inicializar_stdout(char*);
interfaz_dialfs* inicializar_dialfs(char*);
//funcion para insertar la interfaz creada previamente a la lista
//funcion para liberar memoria de las interfaces
void iniciar_listas_io();
void mostrar_interfaces();
#endif
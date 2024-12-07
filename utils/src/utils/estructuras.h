#ifndef _ESTRUCTURAS_H
#define _ESTRUCTURAS_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include<commons/collections/list.h>
typedef enum
{
	MENSAJE,
	HANDSHAKE,
	HANDSHAKE_TAM_MEMORIA,
	PAQUETE,
	// CPU
	CREAR_PROCESO,
	TERMINAR_PROCESO, //Libera la pcb, avisa a memoria y a consola
	BLOQUEAR_PROCESO,
	PETICION_KERNEL,
	APROPIAR_RECURSOS,
	ASIGNAR_RECURSOS,
	DESALOJAR_RECURSOS,
	DESALOJAR_PROCESO,
	SLEEP,
	FIN_SLEEP,
	PROCESAR_INSTRUCCION,
	INTERRUPCION,
	PETICION_CPU,
	EJECUTAR,
	// memoria
	ACCESO_A_PAGINA,
	PAGE_FAULT,
	FINALIZAR_PROCESO_MEMORIA,
	READ_MEMORY,
	READ_MEMORY_RESPUESTA,
	WRITE_MEMORY,
	WRITE_MEMORY_RESPUESTA,
	INSTRUCCION,
	RESIZE_MEMORY,
	RESIZE_MEMORY_RESPUESTA,
	TAMANO_PAGINA,
	MARCO, //se usa? Si
	
	NUEVO_PROCESO,
	FINALIZAR_PROCESO,
	LEER_CONTENIDO_PAGINA,
	ESCRIBIR_CONTENIDO_PAGINA,
	ABRIR_ARCHIVO,
	ABRIR_ARCHIVO_RESPUESTA,
	CERRAR_ARCHIVO,
	APUNTAR_ARCHIVO,
	TRUNCAR_ARCHIVO,
	TRUNCAR_ARCHIVO_RESPUESTA,
	LEER_ARCHIVO,
	LEER_ARCHIVO_RESPUESTA,
	ESCRIBIR_ARCHIVO,
	ESCRIBIR_ARCHIVO_RESPUESTA,
	CREAR_ARCHIVO,
	CREAR_ARCHIVO_RESPUESTA,
	ELIMINAR_ARCHIVO,
	ELIMINAR_ARCHIVO_RESPUESTA,
	//consola kernel (FINALIZAR PROCESO REUTILIZA EL DE ARRIBA)
	INICIAR_PROCESO,
	DETENER_PLANIFICACION,
	INICIAR_PLANIFICACION,
	MULTIPROGRAMACION,
	PROCESO_ESTADO,
	INTERRUPCION_IO,
	ACTUALIZAR_CONTEXTO,
	MOSTRAR,
	//IO
	NUEVA_INTERFAZ,
}op_code;
typedef struct{
	uint32_t pagina;
	uint32_t marco;
	uint32_t offset;
}t_pagina;
typedef struct { 
	int codigo_operacion_tamanio; //
	char* codigo_operacion;//OPERACION SUM ETC
	int parametro1_tamanio;
	int parametro2_tamanio;
	int parametro3_tamanio;
	char* parametros[3];
}t_instruccion;
typedef struct {
	char* comando[3];
}t_comando;

typedef struct  {
	char* nombre_recurso;
	int tamanio_nombre_recurso;
	int instancias_en_posesion;
}t_recurso;
typedef struct registros_cpu{ 
    uint8_t regAX;
    uint8_t regBX;
    uint8_t regCX;
    uint8_t regDX;

    uint32_t regEAX;
    uint32_t regEBX;
    uint32_t regECX;
    uint32_t regEDX;
    
    uint32_t regSI;
    uint32_t regDI;
	//uint32_t regPC;
} t_registros_cpu;
typedef struct
{
	uint32_t pid;
	t_instruccion* instruccion;
	uint32_t program_counter;
	t_registros_cpu* registros_CPU;

} t_contexto_ejec;

//estructuras del kernel
typedef struct {
    int conexion;
    void (*funcion)(int);
} ThreadArgs;

enum estado{
    NEW,
    READY,
    EXEC,
    BLOCKED,
    EXIT
};typedef enum estado t_estado;
struct pcb{
    uint32_t pid;
    t_estado estado; // Estado del proceso (listo, ejecutando, bloqueado, etc.)
    uint32_t quantum;
    uint32_t program_counter;
    t_registros_cpu* registros_cpu; //Registros de la CPU del proceso
	char* path; // Path del script a ejecutar
	uint32_t tamanio_path; // Tamaño del path
    //int prioridad;
    t_instruccion* instruccion;
};

typedef struct pcb t_pcb;

struct fcb{
	char* nombre;
	int tamanio;
	int bloque_inicial;

};

typedef struct fcb t_fcb;
/* -------------------------------- */

/*Estructuras IO*/
struct io_generica{
	char* nombre;
    char* tipo_interfaz;
    int tiempo_unidad_trabajo;
	char* ip_kernel;
	int puerto_kernel;
	int estado;
};
typedef struct io_generica interfaz_generica;
//DE ACA PARA ABAJO SON PARA LA 3ER ENTREGA , PERO LO DEJO MODELADO 
struct io_stdin{
	char* nombre;
	char* tipo_interfaz;
	char* ip_kernel;
	int puerto_kernel;
	char* ip_memoria;
	int puerto_memoria;
	int estado;
};
typedef struct io_stdin interfaz_stdin;

struct io_stdout{
	char* nombre;
	char* tipo_interfaz;
	char* ip_kernel;
	int puerto_kernel;
	char* ip_memoria;
	int puerto_memoria;
	int tiempo_unidad_trabajo;
	int estado;
};
typedef struct io_stdout interfaz_stdout;

struct dialfs {
    char* nombre;
    char* tipo_interfaz;
    int tiempo_unidad_trabajo;
    char* ip_kernel;
    int puerto_kernel;
    char* ip_memoria;
    int puerto_memoria;
    char* path_base_dialfs;
    int block_size;
    int block_count;
    int retraso_compactacion;
	int estado;
};
typedef struct dialfs interfaz_dialfs;


typedef struct lista_interfaces lista_io;

enum tipos_io{
	GENERICA,
	STDIN,
	STDOUT,
	DIALFS
};

struct conexion_interfaz{
	int socket;
	char* nombre;
};
typedef struct conexion_interfaz conexion_interfaz;

// reemplazado por una lista
// struct io{
// 	interfaz_generica* io_generica[MAX_INTERFACES];
// 	interfaz_stdin* io_stdin[MAX_INTERFACES];
// 	interfaz_stdout* io_stdout[MAX_INTERFACES];
// 	interfaz_dialfs* io_dialfs[MAX_INTERFACES];
// };
// typedef struct io lista_io;

/*---------------------------------*/
void parametros_lenght(t_instruccion*);
#endif
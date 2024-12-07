#ifndef CPU_CICLO_INSTRUCCION_H_
#define CPU_CICLO_INSTRUCCION_H_

#include "cpu.h"
#include "cpu_operaciones_instrucciones.h"
#include <utils/conexiones.h>
#include "cpu_funciones_intermodulo.h"
extern int socket_kernel_interrupt;
extern int socket_memoria;
//IOS
// void operacion_io_stdin_read(t_contexto_ejec*,int);
// void operacion_io_stdout_write(t_contexto_ejec**, t_instruccion*);
// void operacion_fs_create(t_contexto_ejec*,int);
// void operacion_fs_delete(t_contexto_ejec*,int);
// void operacion_fs_truncate(t_contexto_ejec*,int);
// void operacion_fs_write(t_contexto_ejec* ,int);
// void operacion_fs_read(t_contexto_ejec*,int);
//
void ciclo_instruccion(int);
void t_instruccion_destroyer(void* );
void t_contexto_destroyer(void*);
// t_instruccion* recibir_instruccion(int);
// t_instruccion *deserializar_instruccion_en(t_paquete* paquete);
// void recibir_y_mostrar_instruccion(int cliente_fd);
void operacion_set(t_contexto_ejec**, t_instruccion*);
t_instruccion* solicitar_instruccion_memoria(uint32_t , uint32_t );
void recibir_interrupcion(int );
#endif

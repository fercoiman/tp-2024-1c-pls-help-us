#ifndef CPU_OPERACIONES_INSTRUCCIONES_H
#define CPU_OPERACIONES_INSTRUCCIONES_H

#include "cpu.h"
extern int conexion_memoria;
void operacion_set(t_contexto_ejec** ,t_instruccion* );
void setter_registro(char*,t_contexto_ejec**, uint32_t);
void operacion_mov_in(t_contexto_ejec** ,t_instruccion*);
void operacion_mov_out(t_contexto_ejec** ,t_instruccion*);
void operacion_sum(t_contexto_ejec**,t_instruccion*);
void operacion_sub(t_contexto_ejec**,t_instruccion*);
 uint32_t obtener_valor_registro(char*,t_contexto_ejec**);
void operacion_jnz(t_contexto_ejec**,t_instruccion*);
void operacion_resize(t_contexto_ejec**,t_instruccion* instruccion);
void operacion_copy_string(t_contexto_ejec**, t_instruccion*);
void operacion_wait(t_contexto_ejec* ,int );
void operacion_signal(t_contexto_ejec*,int );
void operacion_exit(t_contexto_ejec*,int);
// //OPERACIONES IO
void operacion_io_gen_sleep(t_contexto_ejec*,int);
void lanzar_interrupcion(op_code);

// //FUNCIONES PARA KERNEL
// void pasar_a_kernel(t_contexto_ejec*, op_code, int);

uint32_t leer_valor_de_direccion(uint32_t,uint32_t);
void guardar_en_direccion(uint32_t , int ,uint32_t );
#endif



#include "cpu_funciones_intermodulo.h"

//PASO DE DATOS

void pasar_a_kernel(t_contexto_ejec* un_contexto, op_code codigo,int cliente_kernel){
    log_info(cpu_logger,"Se envia el contexto a KERNEL");
    t_paquete* paquete_del_contexto = crear_paquete(codigo);
    agregar_a_paquete(paquete_del_contexto,&un_contexto->pid,sizeof(uint32_t));
    agregar_a_paquete(paquete_del_contexto,&un_contexto->program_counter,sizeof(uint32_t));
    agregar_a_paquete(paquete_del_contexto,&un_contexto->registros_CPU->regAX,sizeof(uint8_t));
    agregar_a_paquete(paquete_del_contexto,&un_contexto->registros_CPU->regBX,sizeof(uint8_t));
    agregar_a_paquete(paquete_del_contexto,&un_contexto->registros_CPU->regCX,sizeof(uint8_t));
    agregar_a_paquete(paquete_del_contexto,&un_contexto->registros_CPU->regDX,sizeof(uint8_t));
    agregar_a_paquete(paquete_del_contexto,&un_contexto->registros_CPU->regEAX,sizeof(uint32_t));
    agregar_a_paquete(paquete_del_contexto,&un_contexto->registros_CPU->regEBX,sizeof(uint32_t));
    agregar_a_paquete(paquete_del_contexto,&un_contexto->registros_CPU->regECX,sizeof(uint32_t));
    agregar_a_paquete(paquete_del_contexto,&un_contexto->registros_CPU->regEDX,sizeof(uint32_t));
    agregar_a_paquete(paquete_del_contexto,&un_contexto->registros_CPU->regSI,sizeof(uint32_t));
    agregar_a_paquete(paquete_del_contexto,&un_contexto->registros_CPU->regDI,sizeof(uint32_t));
    int tamanio = strlen(un_contexto->instruccion->codigo_operacion)+1;
    agregar_a_paquete(paquete_del_contexto,&tamanio,sizeof(int));
    agregar_a_paquete(paquete_del_contexto,un_contexto->instruccion->codigo_operacion,sizeof(char)* tamanio);
    int tamanio_param_1 = strlen(un_contexto->instruccion->parametros[0])+1;
    int tamanio_param_2 = strlen(un_contexto->instruccion->parametros[1])+1;
    int tamanio_param_3 = strlen(un_contexto->instruccion->parametros[2])+1;
    if(tamanio_param_1 == 1){
        tamanio_param_1 = 0;
        agregar_a_paquete(paquete_del_contexto,&tamanio_param_1,sizeof(int));
        agregar_a_paquete(paquete_del_contexto,un_contexto->instruccion->parametros[0],sizeof(char)* tamanio_param_1);
    }else{
        agregar_a_paquete(paquete_del_contexto,&tamanio_param_1,sizeof(int));
        agregar_a_paquete(paquete_del_contexto,un_contexto->instruccion->parametros[0],sizeof(char)* tamanio_param_1);
    }
    if (tamanio_param_2 == 1){
        tamanio_param_2 = 0;
        agregar_a_paquete(paquete_del_contexto,&tamanio_param_2,sizeof(int));
        agregar_a_paquete(paquete_del_contexto,un_contexto->instruccion->parametros[1],sizeof(char)* tamanio_param_2);
    }else{   
        agregar_a_paquete(paquete_del_contexto,&tamanio_param_2,sizeof(int));
        agregar_a_paquete(paquete_del_contexto,un_contexto->instruccion->parametros[1],sizeof(char)* tamanio_param_2);
    }
    if (tamanio_param_3 == 1){
        tamanio_param_3 = 0;
        agregar_a_paquete(paquete_del_contexto,&tamanio_param_3,sizeof(int));
        agregar_a_paquete(paquete_del_contexto,un_contexto->instruccion->parametros[2],sizeof(char)* tamanio_param_3);
    }else{
        agregar_a_paquete(paquete_del_contexto,&tamanio_param_3,sizeof(int));
        agregar_a_paquete(paquete_del_contexto,un_contexto->instruccion->parametros[2],sizeof(char)* tamanio_param_3);
    }
    enviar_paquete(paquete_del_contexto,cliente_kernel);
    eliminar_paquete(paquete_del_contexto);
    return;
}

//INSTRUCCION SOLICITADA A MEMORIA

// void instruccion_de_memoria(int PC, int PID, int conexion_memoria){
//     t_paquete* paquete_PC = crear_paquete();

//     op_code codigo = recibir_operacion(conexion_memoria);

//     if(codigo != INSTRUCCION){
//         log_error(cpu_logger,"No se pudo recibir la INSTRUCCION de MEMORIA");
//         return NULL;
//     }
//     //HAY QUE RECIBIR LA INSTRUCCION DE MEMORIA
//     t_instruccion* instruccion = recibir_instruccion(conexion_memoria);

//     eliminar_paquete(paquete_PC);
//     return instruccion;


// }


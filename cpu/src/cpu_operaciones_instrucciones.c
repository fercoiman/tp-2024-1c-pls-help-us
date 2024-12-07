#include "cpu_operaciones_instrucciones.h"
#include "cpu_funciones_intermodulo.h"

int tamanio_pagina;

//SET PROBADO Y FUNCionando
void operacion_set(t_contexto_ejec** un_contexto,t_instruccion* instruccion){
    char* registro = strdup(instruccion->parametros[0]);
    uint32_t un_valor = atoi(instruccion->parametros[1]);
    setter_registro(registro,un_contexto,un_valor);
    log_info(cpu_logger,"SET: %s = %d",registro,un_valor);
    free(registro);
}

//Prueba de seteo
//Asignar un valor al registro de la CPU
void setter_registro(char* registro,t_contexto_ejec** un_contexto, uint32_t un_valor){
    if(strcmp(registro,"AX") == 0){
        (*un_contexto)->registros_CPU->regAX = un_valor;
    }else if(strcmp(registro,"BX") == 0){
        (*un_contexto)->registros_CPU->regBX = un_valor;
    }else if(strcmp(registro,"CX") == 0){
        (*un_contexto)->registros_CPU->regCX = un_valor;
    }else if(strcmp(registro,"DX") == 0){
        (*un_contexto)->registros_CPU->regDX= un_valor;
    }else if(strcmp(registro,"EAX") == 0){
        (*un_contexto)->registros_CPU->regEAX= un_valor;
    }else if(strcmp(registro,"EBX") == 0){
        (*un_contexto)->registros_CPU->regEBX= un_valor;
    }else if(strcmp(registro,"ECX") == 0){
        (*un_contexto)->registros_CPU->regECX= un_valor;
    }else if(strcmp(registro,"EDX") == 0){
        (*un_contexto)->registros_CPU->regEDX= un_valor;
    }else if(strcmp(registro,"SI") == 0){
        (*un_contexto)->registros_CPU->regSI= un_valor;
    }else if(strcmp(registro,"DI") == 0){
        (*un_contexto)->registros_CPU->regDI= un_valor;
    }else if (strcmp(registro,"PC") == 0){
        (*un_contexto)->program_counter = un_valor;
    }else{ 
        log_info(cpu_logger,"Registro inexistente %s:",registro);
    }

}

//MOV_IN
 void operacion_mov_in(t_contexto_ejec** un_contexto,t_instruccion* instruccion){
    char* registro = string_duplicate(instruccion->parametros[0]);
    uint32_t valor_registro = obtener_valor_registro(instruccion->parametros[1],un_contexto);
    uint32_t valor = leer_valor_de_direccion(valor_registro,(*un_contexto)->pid);
    if(valor == -1){
        return;
    }
    setter_registro(registro,un_contexto,valor);
    log_trace(cpu_logger,"MOV_IN: %s = %d",registro,valor);
    free(registro);
    return;
}

uint32_t leer_valor_de_direccion(uint32_t direccion ,uint32_t pid){
    t_paquete* paquete = crear_paquete(READ_MEMORY);
    uint32_t bytes_a_leer = sizeof(uint32_t);
    agregar_a_paquete(paquete,&pid,sizeof(uint32_t));
    agregar_a_paquete(paquete,&direccion,sizeof(uint32_t));
    agregar_a_paquete(paquete,&bytes_a_leer,sizeof(uint32_t));
    enviar_paquete(paquete,socket_memoria);
    int codigo = recibir_operacion(socket_memoria);
    if(codigo != READ_MEMORY_RESPUESTA){
        log_error(cpu_logger, "Se recibió otro codigo de operacion: %d", codigo);
        eliminar_paquete(paquete);
        return -1;
    }
    
    uint32_t valor = 0;
    int size = 0;
    void* buffer = recibir_buffer(&size, socket_memoria);

    if (size != sizeof(uint32_t)) {
        log_error(cpu_logger, "Tamaño del buffer recibido no coincide con el tamaño esperado");
        free(buffer);
        return -1; // O algún valor de error específico
    }
    memcpy(&valor, buffer, sizeof(uint32_t));
    free(buffer);
    log_info(cpu_logger, "PID: %d - Leer Dirección Física: %d - Valor: %d", pid,direccion ,valor);
    eliminar_paquete(paquete);
    return valor;
}

//MOV_OUT
 void operacion_mov_out(t_contexto_ejec** un_contexto, t_instruccion*  instruccion) {
    //"dir logica"
    int direccion_logica = atoi(instruccion->parametros[0]);
    char* registro = instruccion->parametros[1];

    uint32_t valor = obtener_valor_registro(registro,un_contexto);

    if(valor == -1){
        log_error(cpu_logger,"Error al obtener valor del registro en MOV_OUT");
        return;
    }

    guardar_en_direccion(valor,direccion_logica,(*un_contexto)->pid);
    return;
}

void guardar_en_direccion(uint32_t valor, int direccion,uint32_t pid){

    t_paquete* paquete = crear_paquete(WRITE_MEMORY);
    agregar_a_paquete(paquete,&pid,sizeof(uint32_t));
    agregar_a_paquete(paquete,&direccion,sizeof(int));
    agregar_a_paquete(paquete,&valor,sizeof(uint32_t));  
    enviar_paquete(paquete,socket_memoria);
    eliminar_paquete(paquete);
    int codigo = recibir_operacion(socket_memoria);
    
    //esta_en_tlb();

    if(codigo == WRITE_MEMORY_RESPUESTA){
        char* mensaje = recibir_mensaje(socket_memoria);

        if(strcmp(mensaje,"OK") == 0){
			log_trace(cpu_logger,"PID: %d - Se escribio en la dirección física: %d - Valor: %d“", pid, direccion, valor);
            free(mensaje);
            return;
        } else {
			log_error(cpu_logger,"PID: %d - no se pudo escribir en memoria: %s", pid, mensaje);
            free(mensaje);
		}

        

    }else{
        log_error(cpu_logger,"Se recibió codigo en MOV: %d, esto ESTA MAL", codigo);
        return;
    }
}

//SUM PROBADO Y FUNCIONANDO
void operacion_sum(t_contexto_ejec** un_contexto,t_instruccion* instruccion){
    char* registro_origen = strdup(instruccion->parametros[0]);
    char* registro_destino = strdup(instruccion->parametros[1]);

    uint32_t valor1 = obtener_valor_registro(registro_destino,un_contexto);
    uint32_t valor2 = obtener_valor_registro(registro_origen,un_contexto);

    valor1 = valor1 + valor2;
    
    setter_registro(registro_destino,un_contexto,valor1);

    free(registro_destino);
    free(registro_origen);

}

//SUB PROBADO Y FUNCIONANDO
void operacion_sub(t_contexto_ejec** un_contexto,t_instruccion* instruccion){
    char* registro_destino = strdup(instruccion->parametros[1]);
    char* registro_origen = strdup(instruccion->parametros[0]);

    uint32_t valor1 = obtener_valor_registro(registro_destino,un_contexto);
    uint32_t valor2 = obtener_valor_registro(registro_origen,un_contexto);

    valor1 = valor1 - valor2;
    
    setter_registro(registro_destino,un_contexto,valor1);

    free(registro_destino);
    free(registro_origen);

}

uint32_t obtener_valor_registro(char* registro,t_contexto_ejec** un_contexto){
    
    uint32_t valor;

    if(strcmp(registro,"AX") == 0){
        valor = (*un_contexto)->registros_CPU->regAX;
    }else if(strcmp(registro,"BX") == 0){
        valor = (*un_contexto)->registros_CPU->regBX;
    }else if(strcmp(registro,"CX") == 0){
        valor = (*un_contexto)->registros_CPU->regCX;
    }else if(strcmp(registro,"DX") == 0){
        valor = (*un_contexto)->registros_CPU->regDX;   
    }else if(strcmp(registro,"EAX") == 0){
        valor = (*un_contexto)->registros_CPU->regEAX;
    }else if(strcmp(registro,"EBX") == 0){
        valor = (*un_contexto)->registros_CPU->regEBX;
    }else if(strcmp(registro,"ECX") == 0){
        valor = (*un_contexto)->registros_CPU->regECX;
    }else if(strcmp(registro,"EDX") == 0){
        valor = (*un_contexto)->registros_CPU->regEDX;
    }else if(strcmp(registro,"SI") == 0){
        valor = (*un_contexto)->registros_CPU->regSI;
    }else if(strcmp(registro,"DI") == 0){
        valor = (*un_contexto)->registros_CPU->regDI;
    }else{
        valor = -1; //Si no esta el registro
        log_info(cpu_logger,"Registro inexistente %s:",registro);
    }

    return valor;

}

//JNZ PROBADO Y FUNCIONANDO
void operacion_jnz(t_contexto_ejec** un_contexto,t_instruccion* instruccion){
    char* registro = strdup(instruccion->parametros[0]);
    int n_instruccion = atoi(instruccion->parametros[1]); //Indice de la funcion a la que se salta
    
    uint32_t valor = obtener_valor_registro(registro,un_contexto);
    
    if(valor!= 0){ //Si no es cero
        (*un_contexto)->program_counter = n_instruccion; //Se actualiza PC para que apunte al numero de instruccion especificado
    }

    free(registro);
}

//RESIZE
void operacion_resize(t_contexto_ejec** un_contexto, t_instruccion* instruccion){ 
    uint32_t size = atoi(instruccion->parametros[0]);
    t_paquete* paquete = crear_paquete(RESIZE_MEMORY);
    agregar_a_paquete(paquete,&(*un_contexto)->pid,sizeof(uint32_t));
    agregar_a_paquete(paquete,&size,sizeof(uint32_t));
    enviar_paquete(paquete,socket_memoria);
    eliminar_paquete(paquete);
    int codigo_operacion = recibir_operacion(socket_memoria);
    if (codigo_operacion == RESIZE_MEMORY_RESPUESTA){
        char* mensaje = recibir_mensaje(socket_memoria);
        log_trace(cpu_logger, "RESIZE: %s", mensaje);
        free(mensaje);
    }else{
        log_error(cpu_logger, "Error al redimensionar memoria");
    }
 }

// //COPY_STRING
void operacion_copy_string(t_contexto_ejec** un_contexto, t_instruccion* instruccion){
    int size = atoi(instruccion->parametros[2]);
    
    uint32_t* SI = (uint32_t*)(uintptr_t)(*un_contexto)->registros_CPU->regSI;
    uint32_t* DI = (uint32_t*)(uintptr_t)(*un_contexto)->registros_CPU->regDI;

    if (SI == NULL || DI == NULL ) {
        log_error(cpu_logger, "Error: Punteros SI o DI son NULL");
        return;
    }

    // Copiar los bytes desde SI a DI
    memcpy(DI,SI,size);

    // Registrar la operación
    //log_info(cpu_logger, "COPY_STRING: Copiados %d bytes desde %p a %p", size, SI, DI);

}

//WAIT PROBADO Y FUNCIONANDO
void operacion_wait(t_contexto_ejec* un_contexto,int cliente_kernel){
    pasar_a_kernel(un_contexto, ASIGNAR_RECURSOS, cliente_kernel);
}
//SIGNAL PROBADO Y FUNCIONANDO
void operacion_signal(t_contexto_ejec* un_contexto,int cliente_kernel){
    pasar_a_kernel(un_contexto, DESALOJAR_RECURSOS, cliente_kernel);
}
//IO_GEN_SLEEP

void operacion_io_gen_sleep(t_contexto_ejec* un_contexto,int cliente_kernel){
    pasar_a_kernel(un_contexto,SLEEP,cliente_kernel);
}

// void lanzar_interrupcion(op_code operacion){
    
// }
//IO_STDIN_READ
// void operacion_io_stdin_read(t_contexto_ejec* un_contexto,int cliente_kernel){
    
// }

//IO_STDOUT_WRITE
// void operacion_io_stdout_write(t_contexto_ejec** un_contexto, t_instruccion* instruccion) {
//     if(instruccion->parametro1_tamanio != 0 || instruccion->parametro2_tamanio != 0 || instruccion->parametro3_tamanio == 0) {
//         log_error(cpu_logger, "No hay ningun tamanio");
//         return;
//     }
    
//     char* interfaz = strdup(instruccion->parametros[0]);
//     uint32_t* direccion = (uint32_t*)(uintptr_t)(*un_contexto)->registros_CPU->regDI;
//     int tamanio = atoi(instruccion->parametros[2]);
    
//     // Verifica si la dirección es válida (no es NULL)
//     if(direccion == NULL) {
//         log_error(cpu_logger, "Error: Puntero de dirección es NULL");
//         free(interfaz);
//         return;
//     }

//    //HACER LA ESCRITURA
    
//     // Registra la operación
//     //log_info(cpu_logger, "IO_STDOUT_WRITE: Datos en la dirección %p impresos por STDOUT", (void*)(uintptr_t)(*direccion));
    
//     free(interfaz);
// }

//IO_FS_CREATE

//IO_FS_DELETE

//IO_FS_TRUNCATE

//IO_FS_WRITE
// void operacion_fs_write(t_contexto_ejec* un_contexto,int cliente_kernel){
//     bool pagefault;//HACER
//     if(!pagefault){
//         un_contexto->program_counter++;
//         un_contexto->instruccion->parametros[2]=string_itoa(tamanio_pagina);
//         un_contexto->instruccion->parametro3_tamanio= strlen(un_contexto->instruccion->parametros[2]) + 1;
//         pasar_a_kernel(un_contexto,ESCRIBIR_ARCHIVO,cliente_kernel);
//     } 
// }

//IO_FS_READ
// void operacion_fs_read(t_contexto_ejec* un_contexto,int cliente_kernel){
//     bool pagefault;//HACER
//     if(!pagefault){
//         un_contexto->program_counter++;
//         un_contexto->instruccion->parametros[2]=string_itoa(tamanio_pagina);
//         un_contexto->instruccion->parametro3_tamanio= strlen(un_contexto->instruccion->parametros[2]) + 1;
//         pasar_a_kernel(un_contexto,LEER_ARCHIVO,cliente_kernel);
//     } 
// }

//EXIT
void operacion_exit(t_contexto_ejec* un_contexto,int cliente_kernel){
    pasar_a_kernel(un_contexto, FINALIZAR_PROCESO, cliente_kernel);
    log_info(cpu_logger, "Finalizo el proceso %d", un_contexto->pid);
    t_contexto_destroyer(un_contexto);
}

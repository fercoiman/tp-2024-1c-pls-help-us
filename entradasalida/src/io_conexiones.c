#include "io_conexiones.h"

int conectar_io_kernel(){
   
    int conexion_kernel = crear_conexion(IP_KERNEL,PUERTO_KERNEL);
    if(conexion_kernel == -1){
        log_info(IO_logger,"No fue posible conectar IO a servidor de KERNEL");
        exit(EXIT_FAILURE);
    }
    enviar_mensaje("OK",conexion_kernel,HANDSHAKE);
    log_info(IO_logger,"Handshake enviado a kernel esperando respuesta");
    op_code codigo_operacion = recibir_operacion(conexion_kernel);
    log_info(IO_logger,"Handshake recibido de kernel");
    if(codigo_operacion != HANDSHAKE){
        log_info(IO_logger,"Error al hacer el handshake con kernel");
    }
    int size;
    char* buffer = recibir_buffer(&size,conexion_kernel);
    free(buffer);
    return conexion_kernel;
}

int conectar_io_memoria(){
    int conexion_memoria = crear_conexion(IP_MEMORIA,PUERTO_MEMORIA);
    if(conexion_memoria == -1){
        log_info(IO_logger,"No fue posible conectar IO a servidor de MEMORIA");
        exit(EXIT_FAILURE);
    }
    enviar_mensaje("OK",conexion_memoria,HANDSHAKE);
    log_info(IO_logger,"Handshake enviado a memoria esperando respuesta");
    op_code codigo_operacion = recibir_operacion(conexion_memoria);
    log_info(IO_logger,"Handshake recibido de memoria");
    if(codigo_operacion != HANDSHAKE){
        log_info(IO_logger,"Error al hacer el handshake con memoria");
    }
    int size;
    char* buffer = recibir_buffer(&size,conexion_memoria);
    free(buffer);
    return conexion_memoria;
}

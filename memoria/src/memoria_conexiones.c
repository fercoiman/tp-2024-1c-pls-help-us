#include "memoria_conexiones.h"
int servidor_memoria;
void esperar_conexiones(){
    servidor_memoria = iniciar_servidor(PUERTO_ESCUCHA,memoria_logger,"Memoria");
    if(servidor_memoria == -1){
        log_error(memoria_logger,"Error al iniciar el servidor de memoria");
        exit(-1);
    }
    cliente_cpu = esperar_cpu(servidor_memoria);
    cliente_kernel = esperar_kernel(servidor_memoria);
    return;
}
int esperar_cpu(int servidor_memoria){
    int cliente_cpu = esperar_cliente(servidor_memoria, memoria_logger, "CPU");
    if(cliente_cpu == -1){
        log_error(memoria_logger,"Error al conectarse con cpu");
        exit(-1);
    }
    log_info(memoria_logger,"se conecto CPU");
    return cliente_cpu;
}
int esperar_kernel(int servidor_memoria){
    log_info(memoria_logger,"esperando conexion con kernel");
    int cliente_kernel = esperar_cliente(servidor_memoria, memoria_logger, "KERNEL");
    if(cliente_kernel == -1){
        log_error(memoria_logger,"Error al conectarse con kernel");
        exit(-1);
    }
    log_info(memoria_logger,"se conecto Kernel");
    return cliente_kernel;
}


// void* esperar_io(void* args){
//     int cliente_fd = *((int*) args);
//     free(args);  // Libera la memoria asignada para el argumento
//     while (1) {
//         int cod_op = recibir_operacion(cliente_fd);
//         switch (cod_op) {
//             case HANDSHAKE:
//             recibir_handshake(cliente_fd);
//             break;
//             case WRITE_MEMORY:
//             //IMPLEMENTAR LOGICA
//             break;
//             case READ_MEMORY:
//             //IMPLEMENTAR LOGICA
//             break;
//             default:
//             log_error(memoria_logger, "Operacion desconocida");
//             sleep(50);
//             break;
//         }
    
//     }
// }
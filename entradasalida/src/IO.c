#include "IO.h"
int conexion_kernel;
int conexion_memoria;

int main(int argc, char* argv[]) {
    //Iniciar IO
    if (argc != 3) {
        fprintf(stderr, "Uso: %s [nombre] [archivo.config]\n", argv[0]);
        return EXIT_FAILURE;
    }
     char* nombre = argv[1];
     char* archivo_config = argv[2];
    inicializa_IO();
    iniciar_interfaz(nombre,archivo_config);
    manejar_solicitudes(conexion_kernel);

    

    return 0;
}
void manejar_solicitudes(int cliente_fd){
    while (1) {
        log_info(IO_logger,"esperando opcode");
        int cod_op = recibir_operacion(cliente_fd);
        log_info(IO_logger,"El codigo de operacion recibide es %d",cod_op);
        switch (cod_op) {
            case SLEEP:
            log_trace(IO_logger,"ME LLEGÓ LA SLEEP");
            recibir_sleep(cliente_fd);
            break;
            case READ_MEMORY:
            //IMPLEMENTAR LOGICA
            break;
            case -1:
            log_error(IO_logger,"Operacion no reconocida");
            sleep(20);
            break;
        }
    
    }
} 


void conectar_interfaz_kernel(char* nombre,char* tipo){
    log_info(IO_logger,"Conectando interfaz ");
    conexion_kernel = conectar_io_kernel();
    printf("conexion_kernel %d\n",conexion_kernel);
    t_paquete* paquete = crear_paquete(NUEVA_INTERFAZ);
    int size = strlen(nombre)+1;
    int size1 = strlen(tipo)+1;
    log_info(IO_logger,"tamanio nombre %d",size);
    log_info(IO_logger,"tamanio tipo %d",size1);
    agregar_a_paquete(paquete,&size,sizeof(int));
    agregar_a_paquete(paquete,nombre,sizeof(char) * size);
    agregar_a_paquete(paquete,&size1,sizeof(int));
    agregar_a_paquete(paquete,tipo,sizeof(char) * size1);
    
    enviar_paquete(paquete,conexion_kernel);  
}
void conectar_interfaz_memoria(char* nombre,char* tipo){
    log_info(IO_logger,"Conectando interfaz ");
    conexion_memoria = conectar_io_memoria();
    printf("conexion_memoria %d\n",conexion_memoria);
    t_paquete* paquete = crear_paquete(NUEVA_INTERFAZ);
    int size = strlen(nombre)+1;
    int size1 = strlen(tipo)+1;
    log_info(IO_logger,"a memoria tamanio nombre %d",size);
    log_info(IO_logger,"a memoria tamanio tipo %d",size1);
    agregar_a_paquete(paquete,&size,sizeof(int));
    agregar_a_paquete(paquete,nombre,sizeof(char) * size);
    agregar_a_paquete(paquete,&size1,sizeof(int));
    agregar_a_paquete(paquete,tipo,sizeof(char) * size1);
    enviar_paquete(paquete,conexion_memoria);  
}
t_comando* armar_comando(char *linea) {
    t_comando *comando = malloc(sizeof(t_comando));
    if (comando == NULL) {
        return NULL;
    }

    char *linea_dup = strdup(linea);
    if (linea_dup == NULL) {
        free(comando);
        return NULL;
    }

    // Inicializar todos los elementos del array a NULL
    for (int i = 0; i < 3; i++) {
        comando->comando[i] = NULL;
    }

    char *token = strtok(linea_dup, " ");
    if (token == NULL) {
        free(comando);
        free(linea_dup);
        return NULL;
    }

    comando->comando[0] = strdup(token);
    if (comando->comando[0] == NULL) {
        free(comando);
        free(linea_dup);
        return NULL;
    }

    token = strtok(NULL, " ");
    if (token != NULL) {
        comando->comando[1] = strdup(token);
        if (comando->comando[1] == NULL) {
            free(comando->comando[0]);
            free(comando);
            free(linea_dup);
            return NULL;
        }
    }

    token = strtok(NULL, " ");
    if (token != NULL) {
        comando->comando[2] = strdup(token);
        if (comando->comando[2] == NULL) {
            free(comando->comando[0]);
            free(comando->comando[1]);
            free(comando);
            free(linea_dup);
            return NULL;
        }
    }

    free(linea_dup);
    return comando;
}
//PARA MOSTAR QUE SE AGREGAN CORRECTAMENTE A LA LISTA
// void mostrar_interfaces(lista_io* lista_interfaces) {
//         log_trace(IO_logger, "Interfaces GENERICAS:\n");
//         interfaz_generica* interfaz_gen = lista_interfaces->io_generica;
//         while (interfaz_gen != NULL) {
//         log_info(IO_logger, "Nombre: %s", interfaz_gen->nombre);
//         log_info(IO_logger, "Tipo de Interfaz: %s", interfaz_gen->tipo_interfaz);
//         log_info(IO_logger, "Tiempo de Unidad de Trabajo: %d", interfaz_gen->tiempo_unidad_trabajo);
//         log_info(IO_logger, "IP del Kernel: %s", interfaz_gen->ip_kernel);
//         log_info(IO_logger, "Puerto del Kernel: %d\n", interfaz_gen->puerto_kernel);
//         interfaz_gen = interfaz_gen->siguiente;
//     }

//     interfaz_stdin* interfaz_stdin = lista_interfaces->io_stdin;
//     log_trace(IO_logger, "Interfaces STDIN:\n");
//     while (interfaz_stdin != NULL) {
//         log_info(IO_logger, "Nombre: %s", interfaz_stdin->nombre);
//         log_info(IO_logger, "IP del Kernel: %s", interfaz_stdin->ip_kernel);
//         log_info(IO_logger, "Puerto del Kernel: %d", interfaz_stdin->puerto_kernel);
//         log_info(IO_logger, "IP de Memoria: %s", interfaz_stdin->ip_memoria);
//         log_info(IO_logger, "Puerto de Memoria: %d\n", interfaz_stdin->puerto_memoria);
//         interfaz_stdin = interfaz_stdin->siguiente;
//     }
//     interfaz_stdout* interfaz_stdout = lista_interfaces->io_stdout;
//     log_trace(IO_logger, "Interfaces STDOUT:\n");
//     while (interfaz_stdout != NULL) {
//         log_info(IO_logger, "Nombre: %s", interfaz_stdout->nombre);
//         log_info(IO_logger, "IP del Kernel: %s", interfaz_stdout->ip_kernel);
//         log_info(IO_logger, "Puerto del Kernel: %d", interfaz_stdout->puerto_kernel);
//         log_info(IO_logger, "IP de Memoria: %s", interfaz_stdout->ip_memoria);
//         log_info(IO_logger, "Puerto de Memoria: %d", interfaz_stdout->puerto_memoria);
//         log_info(IO_logger, "Tiempo de Unidad de Trabajo: %d\n", interfaz_stdout->tiempo_unidad_trabajo);
//         interfaz_stdout = interfaz_stdout->siguiente;
//     }
//     interfaz_dialfs* interfaz_dialfs = lista_interfaces->io_dialfs;

//     log_trace(IO_logger, "Interfaces DIALFS:\n");
//     while (interfaz_dialfs != NULL) {
//         log_info(IO_logger, "Nombre: %s", interfaz_dialfs->nombre);
//         log_info(IO_logger, "Tipo de Interfaz: %s", interfaz_dialfs->tipo_interfaz);
//         log_info(IO_logger, "Tiempo de Unidad de Trabajo: %d", interfaz_dialfs->tiempo_unidad_trabajo);
//         log_info(IO_logger, "IP del Kernel: %s", interfaz_dialfs->ip_kernel);
//         log_info(IO_logger, "Puerto del Kernel: %d", interfaz_dialfs->puerto_kernel);
//         log_info(IO_logger, "IP de Memoria: %s", interfaz_dialfs->ip_memoria);
//         log_info(IO_logger, "Puerto de Memoria: %d", interfaz_dialfs->puerto_memoria);
//         log_info(IO_logger, "Path Base DIALFS: %s", interfaz_dialfs->path_base_dialfs);
//         log_info(IO_logger, "Block Size: %d", interfaz_dialfs->block_size);
//         log_info(IO_logger, "Block Count: %d", interfaz_dialfs->block_count);
//         log_info(IO_logger, "Retraso de Compactación: %d\n", interfaz_dialfs->retraso_compactacion);
//         interfaz_dialfs = interfaz_dialfs->siguiente;
//     }

void recibir_sleep(int socket){
    t_paquete* paquete = recibir_paquete(socket);
    paquete->buffer->offset = 0; //mirar
    char* valor=malloc(sizeof(char)*100);
    int tamanio;
    buffer_read(&tamanio,paquete->buffer,sizeof(int));
    buffer_read(valor,paquete->buffer,sizeof(char)*tamanio);
    eliminar_paquete(paquete);
    
    int valor_int = atoi(valor);
    free(valor);
    log_info(IO_logger,"haciendo sleep por el valor de %d",valor_int);
    usar_interfaz_generica(valor_int);
    enviar_mensaje("OK",socket,FIN_SLEEP);
}
void usar_interfaz_generica(int cantidad_unidades_trabajo) {

    log_info(IO_logger, "Interfaz genérica: Esperando %d unidades de trabajo...", cantidad_unidades_trabajo);
    usleep(cantidad_unidades_trabajo * TIEMPO_UNIDAD_TRABAJO);
    log_info(IO_logger, "Operación completada");
    return;
}
// interfaz_generica* buscar_interfaz_generica(char* nombre){
//    for(int i = 0; i < list_size(lista_generica); i++){
//        interfaz_generica* interfaz = list_get(lista_generica,i);
//        if(strcmp(interfaz->nombre,nombre) == 0){
//            return interfaz;
//        }else{
//            log_error(IO_logger,"No se encontro la interfaz generica");
//            return NULL;
//        }
//    }
//    return NULL;
// }
// t_instruccion* recibir_instruccion_io(t_paquete* paquete){
//     t_instruccion* instruccion = malloc(sizeof(t_instruccion));
//     int tamanio;
//     int tamanio1;
//     int tamanio2;
//     int tamanio3;
//     buffer_read(&tamanio,paquete->buffer,sizeof(int));
//     char* codigo = malloc(sizeof(char)*tamanio);
//     buffer_read(codigo,paquete->buffer,sizeof(char)*tamanio);
//     instruccion->codigo_operacion = codigo;
//     buffer_read(&tamanio1,paquete->buffer,sizeof(int));
//     char *parametro1 = malloc(sizeof(char)*tamanio1);
//     buffer_read(parametro1,paquete->buffer,sizeof(char)*tamanio1);
//     instruccion->parametros[0] = parametro1;
//     if(tamanio1 == 0){
//         instruccion->parametros[0] = NULL;
//     }
//     buffer_read(&tamanio2,paquete->buffer,sizeof(int));
//     char *parametro2 = malloc(sizeof(char)*tamanio2);
//     buffer_read(parametro2,paquete->buffer,sizeof(char)*tamanio2);
//     instruccion->parametros[1] = parametro2;
//     if(tamanio2 == 0){
//         instruccion->parametros[1] = NULL;
//     }
//     buffer_read(&tamanio3,paquete->buffer,sizeof(int));
//     char *parametro3 = malloc(sizeof(char)*tamanio3);
//     buffer_read(parametro3,paquete->buffer,sizeof(char)*tamanio3);
//     instruccion->parametros[2] = parametro3;
//     if(tamanio3 == 0){
//         instruccion->parametros[2] = NULL;
//     }
//     instruccion->codigo_operacion_tamanio = tamanio;    
//     instruccion->parametro1_tamanio = tamanio1;
//     instruccion->parametro2_tamanio = tamanio2;
//     instruccion->parametro3_tamanio = tamanio3;
//     return instruccion;//ver
// }
// }
//falta implementar , deberia solo recibir el nombre 

void destruir_comando(t_comando *comando) {
    if (comando == NULL) {
        return;
    }
    free(comando->comando[0]);
    free(comando->comando[1]);
}
    


#include "kernel_operaciones.h"

void manejar_peticiones_io(int cliente_io){
   while (1){
    int cod_op = recibir_operacion(cliente_io);
    switch(cod_op){
        case HANDSHAKE:
            log_info(kernel_logger,"Se conecto un proceso de IO");
            break;
        default:
            log_error(kernel_logger,"Mensaje no reconocido");
            break;
    }
   }
}
void* manejar_peticiones_cpu_dispatch(void* cliente_cpu_dispatch){
    int cliente_fd = *((int*) cliente_cpu_dispatch);
    while(1){
        int cod_op = recibir_operacion(cliente_fd);
        log_info(kernel_logger,"recibi el codigo de operacion en dispatch %d",cod_op);
        switch(cod_op){
            case ASIGNAR_RECURSOS:
            if (strcmp(ALGORITMO_PLANIFICACION,"RR") == 0){
                finalizar_hilo_quantum();
                }
                log_trace(kernel_logger,"Asignando recursos");
                recibir_recurso_solicitado(cliente_fd);
                if (strcmp(ALGORITMO_PLANIFICACION,"RR") == 0){
                    sem_post(&continuar_planificacion);
                }
                break;
            case DESALOJAR_RECURSOS:
            if (strcmp(ALGORITMO_PLANIFICACION,"RR") == 0){
                finalizar_hilo_quantum();
                }
                log_trace(kernel_logger,"Desalojando recursos");
                recibir_recurso_solicitado(cliente_fd);
                if (strcmp(ALGORITMO_PLANIFICACION,"RR") == 0){
                    sem_post(&continuar_planificacion);
                }
                break;
            case ACTUALIZAR_CONTEXTO:
                //recibir contexto y actualizar el pcb correspondiente
                break;
            case SLEEP:
            if (strcmp(ALGORITMO_PLANIFICACION,"RR") == 0){
                finalizar_hilo_quantum();
                }
                recibir_pedido_IO(cliente_fd);
                if (strcmp(ALGORITMO_PLANIFICACION,"RR") == 0){
                    sem_post(&continuar_planificacion);
                }
                break;
            case CREAR_ARCHIVO:
                recibir_pedido_IO(cliente_fd);
                break;
            case ELIMINAR_ARCHIVO:
                recibir_pedido_IO(cliente_fd);
                break;
            case TRUNCAR_ARCHIVO:
                recibir_pedido_IO(cliente_fd);
                break;
            case LEER_ARCHIVO:
                recibir_pedido_IO(cliente_fd);
                break;
            case ESCRIBIR_ARCHIVO:
                recibir_pedido_IO(cliente_fd);
                break;
            case INTERRUPCION:
                recibir_y_actualizar(cliente_fd);
                sem_post(&actualizar_contexto_sem);
                break;
            case FINALIZAR_PROCESO:
                log_info(kernel_logger,"Finalizando proceso");
                if (strcmp(ALGORITMO_PLANIFICACION,"RR") == 0){
                    finalizar_hilo_quantum();
                }
                recibir_y_actualizar(cliente_fd);
                finalizar_proceso_execute();
                //sem_post(&continuar_planificacion);
                sem_post(&proceso_finalizado);
                sem_post(&proceso_ejecutando);
                break;
            case -1:
                log_error(kernel_logger, "el cliente se desconecto. Terminando servidor");
                return NULL;
            default:
                log_error(kernel_logger,"Mensaje no reconocido cpu distpatch");
                break;
        }
    }
}
void recibir_y_actualizar(int cliente_fd){
    t_contexto_ejec* contexto = recibir_contexto(cliente_fd); //recibo el contexto de ejecucion
    uint32_t pid = contexto->pid;
    t_registros_cpu* registros_cpu = contexto->registros_CPU;
    uint32_t program_counter = contexto->program_counter;
    actualizar_contexto_execute(pid, registros_cpu, program_counter);   
}
void* manejar_peticiones_cpu_interrupt(void* cliente_cpu_interrupt){
    int cliente_fd = *((int*) cliente_cpu_interrupt);
    while(1){
        int cod_op = recibir_operacion(cliente_fd);
        log_info(kernel_logger,"recibi el codigo de operacion en interrupt %d",cod_op);
        switch(cod_op){
            
            case DESALOJAR_PROCESO:
                log_info(kernel_logger,"desalojando...");
                break;
            case INTERRUPCION:
                log_info(kernel_logger,"Interrupcionnnnnn");
                int size;
	            char* buffer = recibir_buffer(&size, cliente_fd);
	            free(buffer);
                break;
            case -1:
                log_error(kernel_logger, "el cliente se desconecto. Terminando servidor");
                return NULL;
            default:
                log_error(kernel_logger,"Mensaje no reconocido cpu interrupt");
                break;
        }
    }
}

void* manejar_peticiones_memoria(void* cliente_memoria){
    int cliente_fd = *((int*) cliente_memoria);
    while(1){
        int cod_op = recibir_operacion(cliente_fd);
        switch(cod_op){
            case HANDSHAKE:
                log_info(kernel_logger,"OK");
                break;
            case -1:
                log_error(kernel_logger, "el cliente se desconecto. Terminando servidor");
                return NULL;
            default:
                log_error(kernel_logger,"Mensaje no reconocido peticion memoria");
                break;
        }
    }

}
void recibir_recurso_solicitado(int cliente_fd){
    t_contexto_ejec* contexto_a_actualizar = recibir_contexto(cliente_fd);
    actualizar_contexto_execute(contexto_a_actualizar->pid,contexto_a_actualizar->registros_CPU,contexto_a_actualizar->program_counter);
    log_info(kernel_logger,"pid : %d, pc: %d",contexto_a_actualizar->pid,contexto_a_actualizar->program_counter);
    //log_info(kernel_logger,"regAX: %d, regBX: %d, regCX: %d, regDX: %d, regEAX: %d, regEBX: %d, regECX: %d, regEDX: %d, regSI: %d, regDI: %d",contexto_a_actualizar->registros_CPU->regAX,contexto_a_actualizar->registros_CPU->regBX,contexto_a_actualizar->registros_CPU->regCX,contexto_a_actualizar->registros_CPU->regDX,contexto_a_actualizar->registros_CPU->regEAX,contexto_a_actualizar->registros_CPU->regEBX,contexto_a_actualizar->registros_CPU->regECX,contexto_a_actualizar->registros_CPU->regEDX,contexto_a_actualizar->registros_CPU->regSI,contexto_a_actualizar->registros_CPU->regDI);
    log_info(kernel_logger,"Instruccion: %s",contexto_a_actualizar->instruccion->codigo_operacion);
    log_info(kernel_logger,"Parametro 1: %s",contexto_a_actualizar->instruccion->parametros[0]);
    bool existe = verificar_existencia_recurso(contexto_a_actualizar->instruccion->parametros[0]);
    if (!existe){
        log_error(kernel_logger,"El recurso solicitado : %s no existe",contexto_a_actualizar->instruccion->parametros[0]);
        pasar_execute_a_exit();
        sem_post(&proceso_ejecutando);
        log_trace(kernel_logger,"pasando proceso: %d a exit por recurso no encontrado",contexto_a_actualizar->pid);
        liberar_contexto(contexto_a_actualizar);
        return;
    }
    if ((strcmp(contexto_a_actualizar->instruccion->codigo_operacion,"WAIT") == 0) && existe){
        log_info(kernel_logger,"El proceso %d solicita (WAIT) el recurso %s",contexto_a_actualizar->pid,contexto_a_actualizar->instruccion->parametros[0]);
        asignar_recurso(contexto_a_actualizar->pid,contexto_a_actualizar->instruccion->parametros[0]);
        
    }else if((strcmp(contexto_a_actualizar->instruccion->codigo_operacion,"SIGNAL") == 0) && existe){
        log_info(kernel_logger,"El proceso %d solicita (SIGNAL) el recurso %s",contexto_a_actualizar->pid,contexto_a_actualizar->instruccion->parametros[0]);
        desalojar_recurso(contexto_a_actualizar->pid,contexto_a_actualizar->instruccion->parametros[0]);
    }else{
        log_error(kernel_logger,"El recurso solicitado no existe u operacion incorrecta");
    }
    liberar_contexto(contexto_a_actualizar);
}
void actualizar_contexto(t_contexto_ejec* contexto_a_actualizar){
    uint32_t pid_actualizado = contexto_a_actualizar->pid;
    t_pcb* pcb_actualizado = malloc(sizeof(t_pcb));
    log_info(kernel_logger,"Actualizando contexto del proceso %d",pid_actualizado);
    void buscar_en_lista_y_actualizar(char* key, void* lista) {
    t_list* lista_procesos = (t_list*) lista;
    for (int i = 0; i < list_size(lista_procesos); i++) {
        t_pcb* pcb = list_get(lista_procesos, i);
        if (pcb->pid == pid_actualizado) {
            sem_wait(&actualizar_contexto_sem);
            pcb_actualizado = list_remove(lista_procesos, i);
            pcb_actualizado->pid = pid_actualizado;
            pcb_actualizado->instruccion = NULL;
            pcb_actualizado->program_counter = contexto_a_actualizar->program_counter;
            pcb_actualizado->registros_cpu->regAX = contexto_a_actualizar->registros_CPU->regAX;
            pcb_actualizado->registros_cpu->regBX = contexto_a_actualizar->registros_CPU->regBX;
            pcb_actualizado->registros_cpu->regCX = contexto_a_actualizar->registros_CPU->regCX;
            pcb_actualizado->registros_cpu->regDX = contexto_a_actualizar->registros_CPU->regDX;
            pcb_actualizado->registros_cpu->regEAX = contexto_a_actualizar->registros_CPU->regEAX;
            pcb_actualizado->registros_cpu->regEBX = contexto_a_actualizar->registros_CPU->regEBX;
            pcb_actualizado->registros_cpu->regECX = contexto_a_actualizar->registros_CPU->regECX;
            pcb_actualizado->registros_cpu->regEDX = contexto_a_actualizar->registros_CPU->regEDX;
            pcb_actualizado->registros_cpu->regSI = contexto_a_actualizar->registros_CPU->regSI;
            pcb_actualizado->registros_cpu->regDI = contexto_a_actualizar->registros_CPU->regDI;
            list_add(lista_procesos, pcb_actualizado);
            sem_post(&actualizar_contexto_sem);
            
            log_info(kernel_logger,"Contexto actualizado correctamente");
            return;
        }
    }
    
}
dictionary_iterator(tabla_procesos, buscar_en_lista_y_actualizar);
}


void desalojar_recurso(uint32_t pid, char* nombre_recurso) {
    char* pid_str = string_itoa(pid);

    t_list* recursos_asignados = dictionary_get(matriz_recursos_asignados, pid_str);

    if (recursos_asignados == NULL) {
        log_error(kernel_logger, "El proceso %d no tiene el recurso %s asignado", pid, nombre_recurso);
        pasar_execute_a_exit();
        sem_post(&proceso_ejecutando);
        log_error(kernel_logger, "moviendo al proceso %d a exit porque hizo signal al recurso %s pero que no lo tiene asignado", pid, nombre_recurso);
        free(pid_str);
        return;
    }

    // Buscar y eliminar el recurso de la lista de asignados
    bool recurso_encontrado = false;
    for (int i = 0; i < list_size(recursos_asignados); i++) {
        t_recurso* recurso = list_get(recursos_asignados, i);
        if (strcmp(recurso->nombre_recurso, nombre_recurso) == 0) {
            recurso->instancias_en_posesion++;
            list_remove(recursos_asignados, i);
            recurso_encontrado = true;
            break;
        }
    }

    if (!recurso_encontrado) {
        log_error(kernel_logger, "El recurso %s no está asignado al proceso %d", nombre_recurso, pid);
    } else {
        log_trace(kernel_logger, "Recurso desalojado: %s", nombre_recurso);
        pasar_execute_a_ready();
        sem_post(&hay_proceso_ready);
        sem_post(&proceso_ejecutando);
    }

    // Si la lista de recursos asignados queda vacía, removemos el pid del diccionario
    if (list_is_empty(recursos_asignados)) {
        dictionary_remove(matriz_recursos_asignados, pid_str);
        list_destroy(recursos_asignados);
    }
    
    free(pid_str);
}


void asignar_recurso(uint32_t pid, char* nombre_recurso) {
    //mostrar_procesos();
    t_recurso* recurso = obtener_recurso(nombre_recurso);
    //el recurso no existe
    if (recurso == NULL) {
        log_error(kernel_logger, "Error al obtener el recurso: %s (NO deberia pasar)", nombre_recurso);
        return;
    }
    //el recurso existe y no está disponible (A BLOCKED)
    if (recurso->instancias_en_posesion <= 0) {
        log_trace(kernel_logger, "El recurso existe pero NO fue asignado:%d  al proceso:%s", pid, recurso->nombre_recurso);
        pasar_execute_a_blocked();
        sem_post(&proceso_ejecutando);
        //sem_post(&continuar_planificacion);
        log_info(kernel_logger,"El proceso %d se queda en blocked esperando que se libere el recurso",pid);
    } else {//el recurso existe y está disponible (A READY)
        char* pid_str = string_itoa(pid);
        t_list* recursos_asignados = dictionary_get(matriz_recursos_asignados, pid_str);
        // Si no existe la lista de recursos para este PID, la creamos
        if (recursos_asignados == NULL) {
            recursos_asignados = list_create();
            dictionary_put(matriz_recursos_asignados, pid_str, recursos_asignados);
        } else {
            free(pid_str);
        }
        // Añadimos el recurso a la lista de recursos asignados
        list_add(recursos_asignados, recurso);

        // Reducimos las instancias en posesión del recurso
        recurso->instancias_en_posesion--;

        log_trace(kernel_logger, "Recurso asignado:%d  al proceso:%s", pid, recurso->nombre_recurso);
        //log_info(kernel_logger, "Instancias restantes: %d", recurso->instancias_en_posesion);
        pasar_execute_a_ready();
        sem_post(&hay_proceso_ready);
        int valor;
        sem_getvalue(&hay_proceso_ready, &valor);
        log_info(kernel_logger,"valor semaforo ready: %d",valor);
        sem_post(&proceso_ejecutando);
        log_trace(kernel_logger,"pasando proceso: %d a ready porque se asignó el recurso: %s",pid,recurso->nombre_recurso);
    }
}
t_recurso* obtener_recurso(char* nombre_recurso) {
    t_recurso* recurso_encontrado = NULL;
    void buscar_recurso(t_recurso* recurso) {
        if (strcmp(recurso->nombre_recurso, nombre_recurso) == 0) {
            recurso_encontrado = recurso; 
        }
    }
    list_iterate(recursos_totales, (void*)buscar_recurso);
    if (recurso_encontrado == NULL) {
        log_error(kernel_logger, "Recurso no encontrado: %s", nombre_recurso);
    }
    return recurso_encontrado;  
}
bool verificar_existencia_recurso(char* nombre_recurso) {
 bool recurso_existe = false;
    void verificar_recurso(t_recurso* recurso){
        if (strcmp(recurso->nombre_recurso, nombre_recurso) == 0) {
            recurso_existe = true;
        }
    }

    list_iterate(recursos_totales, (void*) verificar_recurso);
    return recurso_existe;
}
t_recurso *recurso_new(char *nombre_recurso){
	t_recurso *recurso = malloc(sizeof(t_recurso));
	recurso->nombre_recurso = strdup(nombre_recurso);
	recurso->instancias_en_posesion = 0;
	return recurso;
}
char* registrar_interfaz(int cliente_fd,conexion_interfaz* conexion){
    log_info(kernel_logger,"Registrando interfaz");
    t_paquete* paquete = recibir_paquete(cliente_fd);
    paquete->buffer->offset = 0;
    int size;
    buffer_read(&size, paquete->buffer, sizeof(int));
    log_info(kernel_logger,"tamanio recibido: %d",size);
    char* nombre = malloc(sizeof(char) * size);
    buffer_read(nombre, paquete->buffer, sizeof(char) * size);
    log_info(kernel_logger,"nombre recibido: %s",nombre);
    int size1;
    buffer_read(&size1, paquete->buffer, sizeof(int));
    log_info(kernel_logger,"tamanio recibido: %d",size1);
    char* tipo_interfaz = malloc(sizeof(char) * size1);
    buffer_read(tipo_interfaz, paquete->buffer, sizeof(char) * size1);
    log_info(kernel_logger,"tipo de interfaz recibido: %s",tipo_interfaz);
    if (strcmp(tipo_interfaz,"STDIN") == 0){
        conexion->nombre = nombre;
        dictionary_put(lista_interfaz,nombre, conexion);
        list_add(lista_stdin, conexion);
        log_info(kernel_logger,"Interfaz stdin registrada: %s",nombre);
        return nombre;
    }else if (strcmp(tipo_interfaz,"STDOUT") == 0){
        conexion->nombre = nombre;
        dictionary_put(lista_interfaz,nombre, conexion);
        list_add(lista_stdout, conexion);
        log_info(kernel_logger,"Interfaz stdout registrada: %s",nombre);
        return nombre;
    }else if (strcmp(tipo_interfaz,"DIALFS") == 0){
        conexion->nombre = nombre;
        dictionary_put(lista_interfaz,nombre, conexion);
        list_add(lista_dialfs, conexion);
        log_info(kernel_logger,"Interfaz dialfs registrada: %s",nombre);
        return nombre;
    }else if (strcmp(tipo_interfaz,"GENERICA") == 0){
        conexion->nombre = nombre;
        list_add(lista_generica, conexion);
        dictionary_put(lista_interfaz,nombre, conexion);
        log_info(kernel_logger,"Interfaz generica registrada: %s",nombre);
        return nombre;
    }else{
        log_error(kernel_logger,"Tipo de interfaz no reconocido");
        eliminar_paquete(paquete);
        return NULL;
    }
    log_info(kernel_logger,"Interfaz registrada: %s",nombre);
    eliminar_paquete(paquete);
}
void recibir_pedido_IO(int cliente_fd){
    t_contexto_ejec* contexto = recibir_contexto(cliente_fd);
    //A MODO DE TESTEO
    log_info(kernel_logger,"pid : %d, pc: %d",contexto->pid,contexto->program_counter);
    log_info(kernel_logger,"regAX: %d, regBX: %d, regCX: %d, regDX: %d, regEAX: %d, regEBX: %d, regECX: %d, regEDX: %d, regSI: %d, regDI: %d",contexto->registros_CPU->regAX,contexto->registros_CPU->regBX,contexto->registros_CPU->regCX,contexto->registros_CPU->regDX,contexto->registros_CPU->regEAX,contexto->registros_CPU->regEBX,contexto->registros_CPU->regECX,contexto->registros_CPU->regEDX,contexto->registros_CPU->regSI,contexto->registros_CPU->regDI);
    log_info(kernel_logger,"Instruccion: %s",contexto->instruccion->codigo_operacion);
    log_info(kernel_logger,"Parametro 1: %s",contexto->instruccion->parametros[0]);
    log_info(kernel_logger,"Parametro 2: %s",contexto->instruccion->parametros[1]);
    log_info(kernel_logger,"Parametro 3: %s",contexto->instruccion->parametros[2]);
    
    //A MODO DE TESTEO
    //verifico que instruccion de IO me pide
    if (strcmp(contexto->instruccion->codigo_operacion,"IO_GEN_SLEEP") == 0){
            solicitar_a_io(contexto,SLEEP);
            actualizar_contexto_execute(contexto->pid,contexto->registros_CPU,contexto->program_counter);
            sem_post(&proceso_ejecutando);
            liberar_contexto(contexto);
            return;
        //LOGICA
    }else if (strcmp(contexto->instruccion->codigo_operacion,"IO_STDIN_READ") == 0){
        //LOGICA
    }else if (strcmp(contexto->instruccion->codigo_operacion,"IO_STDOUT_WRITE") == 0){
        //LOGICA
    }else if (strcmp(contexto->instruccion->codigo_operacion,"IO_FS_CREATE") == 0){
        //LOGICA
    }else if (strcmp(contexto->instruccion->codigo_operacion,"IO_FS_DELETE") == 0){
        //LOGICA
    }else if (strcmp(contexto->instruccion->codigo_operacion,"IO_FS_TRUNCATE") == 0){
        //LOGICA
    }else if (strcmp(contexto->instruccion->codigo_operacion,"IO_FS_WRITE") == 0){
        //LOGICA
    }else if (strcmp(contexto->instruccion->codigo_operacion,"IO_FS_READ") == 0){
        //LOGICA
    }   
}

void solicitar_a_io(t_contexto_ejec* contexto, op_code codigo_operacion){
    log_trace(kernel_logger,"Solicitando IO");
    t_paquete* paquete = crear_paquete(codigo_operacion);
    //agregar_a_paquete(paquete,&contexto->pid,sizeof(uint32_t));
    //int tamanio = strlen(contexto->instruccion->codigo_operacion)+1;
    //agregar_a_paquete(paquete,&tamanio,sizeof(int));
    //agregar_a_paquete(paquete,contexto->instruccion->codigo_operacion,sizeof(char)* tamanio);
    int tamanio_param_1 = contexto->instruccion->parametro1_tamanio;
    int tamanio_param_2 = contexto->instruccion->parametro2_tamanio;
    int tamanio_param_3 = contexto->instruccion->parametro3_tamanio;
    log_info(kernel_logger,"tamanio param 1: %d",tamanio_param_1);
    log_info(kernel_logger,"tamanio param 2: %d",tamanio_param_2);
    log_info(kernel_logger,"tamanio param 3: %d",tamanio_param_3);
    conexion_interfaz* conexion = dictionary_get(lista_interfaz,contexto->instruccion->parametros[0]);
    if (conexion == NULL){
        log_error(kernel_logger,"IO no encontrada");
        pasar_execute_a_exit();
        sem_post(&proceso_ejecutando);
        log_trace(kernel_logger,"pasando proceso: %d a exit por io no encontrada",contexto->pid);
        return;
    }
    socket_io = conexion->socket;
    
    if(codigo_operacion == SLEEP){
        agregar_a_paquete(paquete,&tamanio_param_2,sizeof(int));
        log_info(kernel_logger,"parametro del valor: %s",contexto->instruccion->parametros[1]);
        agregar_a_paquete(paquete,contexto->instruccion->parametros[1],sizeof(char)*tamanio_param_2);
        enviar_paquete(paquete,socket_io);
    }
    log_info(kernel_logger,"Enviando aa %s ",conexion->nombre);
    pthread_t hilo_io;
    int* socket_io_copia = malloc(sizeof(int));
    *socket_io_copia = socket_io;
    pthread_create(&hilo_io,NULL,esperar_respuesta_io,socket_io_copia);
    pthread_detach(hilo_io);
    
}
void* esperar_respuesta_io(void* socket_io){
    int cliente_fd = *((int*) socket_io);
    while (1){
        log_trace(kernel_logger,"Esperando respuesta de IO");
        int cod_op = recibir_operacion(cliente_fd);
        switch(cod_op){
           case FIN_SLEEP:
                log_info(kernel_logger,"me llego fin sleep");
                break;
            case -1:
                log_error(kernel_logger, "el cliente se desconecto. Terminando servidor");
                return NULL;
            default:
                log_error(kernel_logger,"Mensaje no reconocido");
                break;
        }
    }
}

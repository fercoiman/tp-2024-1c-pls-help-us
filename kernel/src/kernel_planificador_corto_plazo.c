#include "kernel_planificador_corto_plazo.h"

char* planificador_actual;
int quantum_actual;

void inicializar_hilos_planificador(){

    planificador_actual = ALGORITMO_PLANIFICACION;
    if(strcmp(planificador_actual,"FIFO") == 0){
        log_info(kernel_logger,"Planificador FIFO seleccionado");
        crear_hilo_planificador_fifo();
    }
    else if(strcmp(planificador_actual,"RR") == 0){
        log_info(kernel_logger,"Planificador Round Robin seleccionado");
        crear_hilo_planificador_rr();
    }else if(strcmp(planificador_actual,"VRR") == 0){
        log_info(kernel_logger,"Planificador Virtual Round Robin seleccionado");
        crear_hilo_planificador_vrr();
    }
    else{
        log_error(kernel_logger,"Planificador no reconocido");
    }
}

void crear_hilo_planificador_fifo(){
    pthread_create(&hilo_fifo,NULL,planificar_fifo,NULL);
    pthread_detach(hilo_fifo);
    log_info(kernel_logger,"Hilo de Planificador FIFO creado correctamente");
}
void crear_hilo_planificador_rr(){
    pthread_create(&hilo_round_robin,NULL,planificar_round_robin,NULL);
    pthread_detach(hilo_round_robin);
    log_info(kernel_logger,"Hilo de Planificador Round Robin creado correctamente");
}
void crear_hilo_planificador_vrr(){
    pthread_create(&hilo_virtual_round_robin,NULL,planificar_virtual_round_robin,NULL);
    pthread_detach(hilo_virtual_round_robin);
    log_info(kernel_logger,"Hilo de Planificador Virtual Round Robin creado correctamente");
}


void* planificar_fifo(){
    sem_wait(&inicializar_planificador);
    log_info(kernel_logger,"Planificador FIFO iniciado");
     while (!list_is_empty(lista_ready) || !list_is_empty(lista_new) || !list_is_empty(lista_exec) || !list_is_empty(lista_blocked)){
        sem_wait(&proceso_ejecutando);
        t_pcb* pcb = obtener_siguiente_en_ready();
        if (pcb == NULL)
        {
            log_info(kernel_logger,"No hay mas procesos para ejecutar");
            break;
        }
        enviar_proceso_cpu(pcb);
        pasar_ready_a_execute();  
    }
    log_info(kernel_logger,"No hay mas procesos para ejecutar");
    //finalizar
    return 0;
}
t_pcb* obtener_siguiente_en_ready(){

    t_pcb *pcb = NULL;
    if(!list_is_empty(lista_ready)){
        sem_wait(&mutex_lista_ready);
        pcb = list_get(lista_ready,0);
        sem_post(&mutex_lista_ready);
        return pcb;
    }
    else{
        log_error(kernel_logger,"NO DEBERIAS ESTAR ACA");
        //sem_wait(&esperar_proceso_a_ready);
        log_info(kernel_logger,"Proceso en ready, continuando");
        sem_wait(&mutex_lista_ready);
        if (!list_is_empty(lista_ready))
        {
        pcb = list_get(lista_ready,0);
        sem_post(&mutex_lista_ready);
        return pcb;
        }else
        return NULL;    
    }
    
}

void* planificar_round_robin() {
    sem_wait(&inicializar_planificador);
    crear_hilo_verificacion();
    log_info(kernel_logger, "Planificador Round Robin iniciado");

    while (1) {
            sem_wait(&hay_proceso_ready);
            t_pcb* pcb = obtener_siguiente_en_ready();
            //mostrar_procesos();
            if (pcb == NULL) {
                log_info(kernel_logger, "No deberia entrar aca");
                break;
            }

            log_trace(kernel_logger, "Proceso a enviar a CPU: %d", pcb->pid);
            crear_hilo_quantum(); // Crear hilo del quantum aquí para cada proceso
            sem_wait(&proceso_ejecutando); // Esperar a que se pueda ejecutar un proceso
            enviar_proceso_cpu(pcb);
            pasar_ready_a_execute();
            sem_post(&iniciar_quantum); // Iniciar el quantum para el proceso
            log_info(kernel_logger, "Esperando para continuar la planificación");
            sem_wait(&continuar_planificacion); // Esperar señal para continuar la planificación
            finalizar_hilo_quantum(); // Finalizar hilo del quantum para el proceso
    }

    log_info(kernel_logger, "No hay más procesos para ejecutar");
    return NULL;
}

void* verificacion_fin_proceso() { // Para el caso de que finalice un proceso antes de fin de quantum
    while (1) {
        log_info(kernel_logger, "Hilo verificacion creado");
        sem_wait(&proceso_finalizado); 
        log_info(kernel_logger, "Pasé wait proceso finalizado, finalizando hilo quantum"); // Si se habilita es porque desde CPU ya terminó el proceso y me lo devolvió
        sem_post(&proceso_ejecutando);
        sem_post(&continuar_planificacion);
    } 
    return 0;
}
void* esperar_quantum() {
    while (1) {
    quantum_actual = 0;
    sem_wait(&iniciar_quantum);
    log_info(kernel_logger, "Inicio de espera de quantum");
    quantum_actual = QUANTUM;
    usleep(quantum_actual * 1000);
    log_info(kernel_logger, "Fin de quantum");
    finalizar_proceso_quantum();
    sem_wait(&actualizar_contexto_sem);
    pasar_execute_a_ready();
    sem_post(&hay_proceso_ready);
    sem_post(&continuar_planificacion);
    sem_post(&proceso_ejecutando);
    }
    return 0;

}

void finalizar_proceso_quantum() {
    t_paquete* paquete = crear_paquete(INTERRUPCION);
    char* mensaje = "Fin de quantum";
    agregar_a_paquete(paquete, mensaje, strlen(mensaje) + 1);
    enviar_paquete(paquete, socket_conexion_cpu_interrupt);
    log_error(kernel_logger, "Interrupción por QUANTUM enviada a CPU");    
}

void proceso_en_blocked(){
    sem_wait(&proceso_blocked);
    pasar_blocked_a_ready();
}

//revisar esto, por ahora inutil
void interrupcion_fin_quantum(){ 
    //Enviar mensaje a CPU para que interrumpa el proceso
    pasar_execute_a_blocked();
    sem_wait(&proceso_interrumpido); //Semaforo que avisaria que hay un proceso en blocked
    // otro semaforo para avisarle al planificador que puede seguir???
}

void crear_hilo_verificacion() {
    pthread_create(&hilo_verificacion, NULL, verificacion_fin_proceso, NULL);
    pthread_detach(hilo_verificacion);
    log_info(kernel_logger, "Hilo de verificación de fin de proceso creado correctamente");
}

void finalizar_hilo_verificacion(){
    pthread_cancel(hilo_verificacion);
    log_info(kernel_logger,"Hilo de verificacion de fin de proceso finalizado correctamente");
}

void crear_hilo_quantum() {
    pthread_create(&hilo_quantum, NULL, esperar_quantum, NULL);
    pthread_detach(hilo_quantum);
    log_info(kernel_logger, "Hilo de Quantum creado correctamente");
}

void finalizar_hilo_quantum() {
    if (pthread_cancel(hilo_quantum) != 0) {
        return;
    }
    pthread_cancel(hilo_quantum);
    log_info(kernel_logger, "Hilo de Quantum finalizado correctamente");
}


void* planificar_virtual_round_robin() {
    sem_wait(&inicializar_planificador);
    crear_hilo_verificacion();
    while (!list_is_empty(lista_ready) || !list_is_empty(lista_new) || !list_is_empty(lista_exec) || !list_is_empty(lista_blocked)) {
        crear_hilo_quantum();
        crear_hilo_tiempo_cpu();
        sem_wait(&grado_multiprogramacion); //controla grado de multiprogramacion
        t_pcb* pcb = obtener_siguiente_en_ready();
        if (pcb == NULL)
        {
            log_info(kernel_logger,"No hay mas procesos para ejecutar");
            break;
        }
        sem_wait(&proceso_ejecutando); //desbloquea el semaforo en caso de que vuelva el PCB de CPU
        sem_post(&quantum);
        sem_post(&empezar_tiempo_cpu);
        enviar_proceso_cpu(pcb);
        pasar_ready_a_execute();   

        log_info(kernel_logger,"haciendo post a quantum");
        
        log_info(kernel_logger,"wait continuar planificacion");
        sem_wait(&continuar_planificacion);
        log_info(kernel_logger,"pase el wait continuar planificacion");


        // if(list_is_empty(lista_exec)){
        //     log_info(kernel_logger,"Proceso terminado, paso al siguiente");
        // }else{
        //     //interrupcion
        //     pasar_execute_a_blocked();
        //     log_info(kernel_logger,"Fin de quantum, proceso interrumpido y pasa a blocked");
        // }
        
        //pthread join para que espere la llegada de una respuesta? o un semaforo mutex para execute 
    }
    log_info(kernel_logger,"No hay mas procesos para ejecutar");
    finalizar_hilo_verificacion();
    //finalizar
    return NULL;
}
void actualizar_contexto_execute(uint32_t pid, t_registros_cpu* registros_cpu, uint32_t program_counter) {
    t_pcb *pcb = NULL;
    sem_wait(&mutex_lista_exec);
    pcb = list_remove(lista_exec, 0);
    sem_post(&mutex_lista_exec);
    pcb->program_counter = program_counter;
    pcb->registros_cpu = registros_cpu;
    // pcb->instruccion = contexto->instruccion; // Descarto por ahora
    pcb->pid = pid;
    sem_wait(&mutex_lista_exec);
    list_add(lista_exec, pcb);
    sem_post(&mutex_lista_exec);
    log_info(kernel_logger, "Contexto (EXECUTE) actualizado  correctamente");
}
void* tiempo_en_cpu(){
    int tiempo_total;
    struct timespec start, end;

    sem_wait(&empezar_tiempo_cpu);
    clock_gettime(CLOCK_MONOTONIC, &start);

    sem_wait(&terminar_tiempo_cpu);
    clock_gettime(CLOCK_MONOTONIC, &end);

    tiempo_total = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    //return tiempo_total;
    return NULL;
}

void crear_hilo_tiempo_cpu(){
    pthread_create(&hilo_tiempo_cpu,NULL,tiempo_en_cpu,NULL);
    pthread_detach(hilo_tiempo_cpu);
    log_info(kernel_logger,"Hilo de tiempo en CPU creado correctamente");
}

void finalziar_hilo_tiempo_cpu(){
    pthread_cancel(hilo_tiempo_cpu);
    log_info(kernel_logger,"Hilo de tiempo en CPU finalizado correctamente");
}

// ESTO PONE A EJECUTAR UN PROCESO A CPU , LA FUNCION RECIBE UN PCB PERO CREA UN CONTEXTO Y SE LO ENVIA A CPU YA QUE ESTA RECIBE CONTEXTOS PARA EJECUTAR
void enviar_proceso_cpu (t_pcb* pcb) {
    t_paquete* paquete = crear_paquete(PETICION_CPU);
    if (paquete == NULL)
    {
        log_error(kernel_logger, "No se pudo crear el paquete para la solicitud de creación de proceso");
    }

    agregar_a_paquete(paquete, &pcb->pid, sizeof(uint32_t));
    agregar_a_paquete(paquete, &pcb->program_counter, sizeof(uint32_t));
    agregar_a_paquete(paquete, pcb->registros_cpu, sizeof(t_registros_cpu));
    pcb->instruccion = malloc(sizeof(t_instruccion));
    pcb->instruccion->codigo_operacion_tamanio = 0;
    pcb->instruccion->codigo_operacion = NULL;
    pcb->instruccion->parametro1_tamanio = 0;
    pcb->instruccion->parametro2_tamanio = 0;
    pcb->instruccion->parametro3_tamanio = 0;
    pcb->instruccion->parametros[0] = NULL;
    pcb->instruccion->parametros[1] = NULL;
    pcb->instruccion->parametros[2] = NULL;
    agregar_a_paquete(paquete, &pcb->instruccion->codigo_operacion_tamanio, sizeof(int));
    agregar_a_paquete(paquete, &pcb->instruccion->codigo_operacion, sizeof(char) * pcb->instruccion->codigo_operacion_tamanio);
    agregar_a_paquete(paquete, &pcb->instruccion->parametro1_tamanio, sizeof(int));
    agregar_a_paquete(paquete, &pcb->instruccion->parametro2_tamanio, sizeof(int));
    agregar_a_paquete(paquete, &pcb->instruccion->parametro3_tamanio, sizeof(int));
    agregar_a_paquete(paquete, pcb->instruccion->parametros[0], sizeof(char) * pcb->instruccion->parametro1_tamanio);
    agregar_a_paquete(paquete, pcb->instruccion->parametros[1], sizeof(char) * pcb->instruccion->parametro2_tamanio);
    agregar_a_paquete(paquete, pcb->instruccion->parametros[2], sizeof(char) * pcb->instruccion->parametro3_tamanio);

    int result = enviar_paquete(paquete, socket_conexion_cpu_dispatch);
    if (result < 0) {
        log_error(kernel_logger, "Error al enviar el paquete a CPU");
    } else {
        log_info(kernel_logger, "proceso enviado a ejecutar correctamente a CPU");
    }
    // Liberar el paquete
    eliminar_paquete(paquete);
}

void iniciar_planificacion(){
}
void pasar_ready_a_execute() {
    mover_procesos(lista_ready, lista_exec, &mutex_lista_ready, &mutex_lista_exec, EXEC);
}
void pasar_execute_a_ready() {
    mover_procesos(lista_exec, lista_ready, &mutex_lista_exec, &mutex_lista_ready, READY);
}
void pasar_blocked_a_exit() {
    mover_procesos(lista_blocked, lista_exit, &mutex_lista_blocked, &mutex_lista_exit, EXIT);
}
void pasar_blocked_a_ready() {
    mover_procesos(lista_blocked, lista_ready, &mutex_lista_blocked, &mutex_lista_ready, READY);
}
void pasar_execute_a_exit() {
    mover_procesos(lista_exec, lista_exit, &mutex_lista_exec, &mutex_lista_exit, EXIT);
}
void pasar_execute_a_blocked() {
    mover_procesos(lista_exec, lista_blocked, &mutex_lista_exec, &mutex_lista_blocked, BLOCKED);
}





void agregar_a_blocked(){

}
void agregar_a_running(){

}
int interrumpir(t_pcb* pcb){
    log_info(kernel_logger,"tamo interrumpiendo"); //TODO: definir que hace
    return 0;
}
int addEstadoReady(t_pcb* pcb){
    log_info(kernel_logger,"tamo ready"); //TODO: definir que hace
    return 0;
}


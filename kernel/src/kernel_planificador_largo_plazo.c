#include "kernel_planificador_largo_plazo.h"

t_dictionary* tabla_procesos;
t_dictionary* matriz_recursos_asignados;
t_dictionary* matriz_recursos_pendientes;
t_dictionary* matriz_recursos_asignados_x_pid;
t_dictionary* lista_interfaz;
t_list* lista_new;
t_list* lista_ready;
t_list* lista_exec;
t_list* lista_blocked;
t_list* lista_exit;
t_list* lista_generica;
t_list* lista_stdin;
t_list* lista_stdout;
t_list* lista_dialfs;
t_list *recursos_totales;
extern char** RECURSOS;
extern char** INSTANCIAS_RECURSOS;
int* recursos_disponible;
uint32_t pid_final;

void iniciar_planificador_largo_plazo(){
    pthread_t hilo_planificador_largo_plazo;
    pthread_create(&hilo_planificador_largo_plazo,NULL,planificar_procesos,NULL);
    pthread_detach(hilo_planificador_largo_plazo);
    log_info(kernel_logger,"Soltado hilo planificador largo plazo");
    return;
}



void agregar_a_cola(t_pcb *pcb,t_list* lista,sem_t* sem){
    list_add(lista,pcb);
}
void agregar_a_new(t_pcb *pcb){
    agregar_a_cola(pcb,lista_new,&mutex_lista_new);
    log_info(kernel_logger,"Proceso con PID %d ingresado a la cola NEW", pcb->pid);
}
void agregar_a_ready(t_pcb *pcb){
    agregar_a_cola(pcb,lista_ready,&mutex_lista_ready);
    log_info(kernel_logger,"Proceso con PID %d ingresado a la cola READY", pcb->pid);
}
void agregar_a_exit(t_pcb *pcb){
    agregar_a_cola(pcb,lista_exit,&mutex_lista_exit);
    log_info(kernel_logger,"Proceso con PID %d ingresado a la cola EXIT", pcb->pid);
}

void* planificar_procesos(){
    while (!list_is_empty(lista_new)) {
        sem_wait(&grado_multiprogramacion);
        log_info(kernel_logger, "Iniciando planificador de largo plazo");
        pasar_new_a_ready();
        sem_post(&inicializar_planificador);
        sem_post(&hay_proceso_ready);
    }
    log_info(kernel_logger, "No hay procesos en la cola NEW");
    return NULL; 
}

//haria una creacion de hilos para los planificadores para que esten constantemente revisando sobre procesos nuevos

void inicializar_listas() {
    // Inicializar listas de procesos
    lista_new = list_create();
    lista_ready = list_create();
    lista_exec = list_create();
    lista_blocked = list_create();
    lista_exit = list_create();
    // Inicializar listas de interfaces
    lista_generica = list_create();
    lista_stdin = list_create();
    lista_stdout = list_create();
    lista_dialfs = list_create();
    //Inicializar diccionario de interfaces
    lista_interfaz = dictionary_create();
    
    // Inicializar diccionario de procesos
    tabla_procesos = dictionary_create();
    dictionary_put(tabla_procesos, "new", lista_new);
    dictionary_put(tabla_procesos, "ready", lista_ready);
    dictionary_put(tabla_procesos, "exec", lista_exec);
    dictionary_put(tabla_procesos, "blocked", lista_blocked);
    dictionary_put(tabla_procesos, "exit", lista_exit);

    // Inicializar diccionarios de recursos
    matriz_recursos_asignados = dictionary_create();
    matriz_recursos_pendientes = dictionary_create();
    matriz_recursos_asignados_x_pid = dictionary_create();
    
    // Inicializar recursos y sus instancias

    int cant_recursos = string_array_size(INSTANCIAS_RECURSOS);
    recursos_totales = list_create();
    int* recursos_disponibles = malloc(sizeof(int) * cant_recursos);
    if (recursos_disponibles == NULL) {
        perror("Error al asignar memoria para recursos_disponibles");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < cant_recursos; i++) {
        recursos_disponibles[i] = atoi(INSTANCIAS_RECURSOS[i]);
        t_recurso* recurso_n = recurso_new(RECURSOS[i]);
        recurso_n->instancias_en_posesion = recursos_disponibles[i];
        list_add(recursos_totales, recurso_n);
    }
    list_iterate(recursos_totales, (void*) imprimir_recurso);   
}
void imprimir_recurso(t_recurso* recurso) {
    log_info(kernel_logger, "Recurso %s con %d instancias", recurso->nombre_recurso, recurso->instancias_en_posesion);
}
void finalizar_proceso(int pid_a_finalizar){
    pid_final = pid_a_finalizar;
    dictionary_iterator(tabla_procesos, buscar_en_lista_y_finalizar);
    if (pid_final != 0) {
        log_error(kernel_logger, "No se encontró el proceso con PID %d", pid_final);
    }
    pid_final = 0;
}

void finalizar_proceso_execute(){
    pasar_execute_a_exit();
    sem_post(&grado_multiprogramacion);
    //sem_post(&proceso_ejecutando);
    //sem_post(&proceso_finalizado);
    log_info(kernel_logger,"El proceso  termino con exito");
}


void buscar_en_lista_y_finalizar(char* key, void* lista) {
    t_list* lista_procesos = (t_list*) lista;
    for (int i = 0; i < list_size(lista_procesos); i++) {
        t_pcb* pcb = list_get(lista_procesos, i);
        if (pcb->pid == pid_final) {
            t_pcb* pcb_a_exit = list_remove(lista_procesos, i);
            list_add(lista_exit, pcb_a_exit);
            log_info(kernel_logger, "Proceso con PID %d finalizado con éxito", pid_final);
            pid_final=0;
            return;
        }
    }
}

void mover_procesos(t_list* lista_origen, t_list* lista_destino, sem_t* sem_origen, sem_t* sem_destino, t_estado nuevo_estado) {
    if (!list_is_empty(lista_origen)) {
        sem_wait(sem_origen);
        t_pcb* pcb = list_remove(lista_origen, 0); 
        sem_post(sem_origen);
        sem_wait(sem_destino);
        pcb->estado = nuevo_estado;
        list_add(lista_destino, pcb);
        sem_post(sem_destino);
        if (nuevo_estado == NEW) {
            log_trace(kernel_logger, "Proceso con PID %d movido a la lista NEW", pcb->pid);
        }
        else if(nuevo_estado == READY){
            log_trace(kernel_logger, "Proceso con PID %d movido a la lista READY", pcb->pid);
        }
        else if(nuevo_estado == EXEC){
            log_trace(kernel_logger, "Proceso con PID %d movido a la lista EXEC", pcb->pid);
        }
        else if(nuevo_estado == BLOCKED){
            log_trace(kernel_logger, "Proceso con PID %d movido a la lista BLOCKED", pcb->pid);
        }
        else if(nuevo_estado == EXIT){
            log_trace(kernel_logger, "Proceso con PID %d movido a la lista EXIT", pcb->pid);
        }
    }else 
        log_info(kernel_logger, "No hay procesos en la lista origen");
}

//READY, EXEC y BLOCKED tienen que reservar el semaforo de grado_multiprogramacion, si ya estan en uno de esos tres no cambio nada
void pasar_new_a_ready() {
    log_info(kernel_logger, "Pasando procesos de NEW a READY");
    mover_procesos(lista_new, lista_ready, &mutex_lista_new, &mutex_lista_ready, READY); 
}
void pasar_ready_a_exit() { //se usara? ver finalizar proceso
    mover_procesos(lista_ready, lista_exit, &mutex_lista_ready, &mutex_lista_exit, EXIT);
}
void pasar_new_a_exit() { //se usara? ver finalizar proceso
    mover_procesos(lista_new, lista_exit, &mutex_lista_new, &mutex_lista_exit, EXIT);
}


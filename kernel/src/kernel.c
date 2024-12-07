 #include "kernel.h"
 #include <pthread.h>



 int main(int argc, char* argv[]) {
    //Iniciar kernel
    inicializar_kernel();
    servidor_kernel = iniciar_kernel_servidor(); //poner kernel en escucha (servidor_kernel es el socket)
    
    //conectarse a los otros modulos
    socket_conexion_memoria = conectar_memoria();
    sleep(1);
    socket_conexion_cpu_interrupt = conectar_cpu_interrupt();
    socket_conexion_cpu_dispatch = conectar_cpu_dispatch();
    pthread_t hilo_io;

    pthread_create(&hilo_io, NULL, esperar_io, NULL);
    pthread_detach(hilo_io);
    
    pthread_t hilo_interrupt,hilo_dispatch,hilo_memoria;
    int* interrupt_ptr = malloc(sizeof(int));
    int* dispatch_ptr = malloc(sizeof(int));
    int* memoria_ptr = malloc(sizeof(int));
    *interrupt_ptr = socket_conexion_cpu_interrupt;
    *dispatch_ptr = socket_conexion_cpu_dispatch;
    *memoria_ptr = socket_conexion_memoria;
	pthread_create(&hilo_dispatch, NULL, manejar_peticiones_cpu_dispatch,dispatch_ptr );
	pthread_create(&hilo_interrupt, NULL, manejar_peticiones_cpu_interrupt, interrupt_ptr);
	pthread_create(&hilo_memoria, NULL, manejar_peticiones_memoria, memoria_ptr);

	pthread_detach(hilo_dispatch);
	pthread_detach(hilo_interrupt);
	pthread_detach(hilo_memoria);
    inicializar_semaforos();
    inicializar_listas();
    init_consola();
    terminar_programa(kernel_logger,kernel_config);
    //dictionary_destroy_and_destroy_list_elements(tabla_procesos);
    return 0;
}

void t_instruccion_destroyer(void* instruccion){
    t_instruccion* instruccion_a_eliminar = (t_instruccion*) instruccion;
    free(instruccion_a_eliminar->codigo_operacion);
    for(int i = 0; i < 3; i++){
        free(instruccion_a_eliminar->parametros[i]);
    } 
    free(instruccion_a_eliminar);
}
void pcb_destroyer(void* element) {
    t_pcb* pcb_a_eliminar = (t_pcb*) element;
    if (pcb_a_eliminar->registros_cpu) {
        free(pcb_a_eliminar->registros_cpu);
    }
    if (pcb_a_eliminar->path) {
        free(pcb_a_eliminar->path);
    }
    if (pcb_a_eliminar->instruccion) {
        t_instruccion_destroyer(pcb_a_eliminar->instruccion);
    }
    free(pcb_a_eliminar);
}
void list_destroyer(void* list) {
    list_destroy_and_destroy_elements((t_list*)list, pcb_destroyer);
}
void dictionary_destroy_and_destroy_list_elements(t_dictionary* dictionary) {
    dictionary_destroy_and_destroy_elements(dictionary, list_destroyer);
}

#include "kernel_consola.h"
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <errno.h>
#define BUFFER_SIZE 256
t_log *logger_consola;
t_config *config_consola;
uint32_t pid_global= 1000;
pthread_mutex_t mutex_pid;
sem_t grado_multiprogramacion;
int grado_multiprogramacion_inicial;
void mostrar_encabezado() {
    
    const char* ORANGE_LIGHT_FG = "\033[38;5;208m"; 
    const char* LIGHT_PURPLE_FG = "\033[38;5;183m"; 
    const char* GREEN_FG = "\033[32m";              
    const char* RED_FG = "\033[31m";                 
    const char* BLUE_FG = "\033[34m";                
    const char* YELLOW_FG = "\033[33m";              
    const char* RESET = "\033[0m";                   
    const char* BOLD = "\033[1m";                    
    const char* UNDERLINE = "\033[4m";               

    int width = 60;
    int height = 11;

    printf("\n");

   
    printf("%s┌", GREEN_FG);
    for (int i = 0; i < width - 2; i++) printf("%s─", GREEN_FG); 
    printf("┐\n");

    
    for (int i = 0; i < height - 2; i++) {
        printf("%s│", GREEN_FG);

        if (i == 0) {
            int text_width = 11; 
            int padding = (width - text_width - 2) / 2; 
            printf("%*s", padding, ""); 
            printf("%s%s%sPLS HELP US%s", BOLD, ORANGE_LIGHT_FG, UNDERLINE, RESET);
            printf("%*s", padding + (width - text_width - 2) % 2, ""); 
        } else if (i == 2) {
            int text_width = 14; 
            int padding = (width - text_width - 2) / 2; 
            printf("%*s", padding, ""); 
            printf("%s%s%sCONSOLA KERNEL%s", BOLD, LIGHT_PURPLE_FG, UNDERLINE, RESET);
            printf("%*s", padding + (width - text_width - 2) % 2, ""); 
        } else if (i == 4) {
            char quantum_str[width - 3];
            snprintf(quantum_str, width - 3, "Quantum: %d", QUANTUM);
            int text_width = strlen(quantum_str);
            int padding = (width - text_width - 2) / 2; 
            printf("%*s", padding, ""); 
            printf("%s%s%s%s", RED_FG, BLUE_FG, quantum_str, RESET);
            printf("%*s", padding + (width - text_width - 2) % 2, ""); 
        } else if (i == 6) {
            char grado_str[width - 3];
            snprintf(grado_str, width - 3, "Grado de Multiprogramacion: %d", GRADO_MULTIPROGRAMACION);
            int text_width = strlen(grado_str);
            int padding = (width - text_width - 2) / 2; 
            printf("%*s", padding, ""); 
            printf("%s%s%s%s", YELLOW_FG, GREEN_FG, grado_str, RESET);
            printf("%*s", padding + (width - text_width - 2) % 2, ""); 
        } else if (i == 8) {
            char algoritmo_str[width - 3];
            snprintf(algoritmo_str, width - 3, "Algoritmo de Planificacion: %s", ALGORITMO_PLANIFICACION);
            int text_width = strlen(algoritmo_str);
            int padding = (width - text_width - 2) / 2; 
            printf("%*s", padding, ""); 
            printf("%s%s%s%s", BLUE_FG, RED_FG, algoritmo_str, RESET);
            printf("%*s", padding + (width - text_width - 2) % 2, "");
        } else {
            printf("%*s", width - 2, " ");
        }
        printf("%s│\n", GREEN_FG);
    }

    printf("%s└", GREEN_FG);
    for (int i = 0; i < width - 2; i++) printf("%s─", GREEN_FG); 
    printf("%s┘\n", GREEN_FG);
}


void inicializar_semaforos(){
	grado_multiprogramacion_inicial = config_get_int_value(kernel_config, "GRADO_MULTIPROGRAMACION");
	sem_init(&grado_multiprogramacion,0,grado_multiprogramacion_inicial);
    sem_init(&mutex_lista_new, 0, 1);
    sem_init(&mutex_lista_ready, 0, 1);
	sem_init(&mutex_lista_exit, 0, 1);
	sem_init(&mutex_lista_exec, 0 ,1);
	sem_init(&mutex_lista_blocked, 0 ,1);
    sem_init(&proceso_ejecutando, 0, 1);//
    sem_init(&inicializar_planificador, 0, 0);
    sem_init(&hay_proceso_ready, 0, 0);
    sem_init(&quantum, 0, 0);
    sem_init(&proceso_finalizado, 0, 0);
    sem_init(&continuar_planificacion, 0, 0);//
    sem_init(&proceso_interrumpido, 0, 0);
    sem_init(&proceso_blocked, 0, 0);
    sem_init(&empezar_tiempo_cpu, 0, 0);
    sem_init(&terminar_tiempo_cpu, 0, 0);
    sem_init(&iniciar_quantum,0,0);//
    sem_init(&actualizar_contexto_sem,1,0);//
}

void inicializar_pid_global(){
	pid_global = 1000;
	pthread_mutex_init(&mutex_pid,NULL);
}

uint32_t obtener_pid(){
    pthread_mutex_lock(&mutex_pid);
    uint32_t pid = pid_global++;
    pthread_mutex_unlock(&mutex_pid);
    return pid;
}


/*PID: Identificador del proceso (deberá ser un número entero, único en todo el sistema).
Program Counter: Número de la próxima instrucción a ejecutar.
Quantum: Unidad de tiempo utilizada por el algoritmo de planificación VRR.
Registros de la CPU: Estructura que contendrá los valores de los registros de uso general de la CPU.*/
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

    // Inicializar el segundo parámetro
    comando->comando[1] = NULL;

    if ((token = strtok(NULL, " ")) != NULL) {
        comando->comando[1] = strdup(token);
        if (comando->comando[1] == NULL) {
            free(comando->comando[0]);
            free(comando);
            free(linea_dup);
            return NULL;
        }
    }

    free(linea_dup);
    return comando;
}
int init_consola() {

    mostrar_encabezado();

    while (1) {
        char *linea = readline(">>>");
        if (linea) {
            add_history(linea);
        }
        t_comando * comando = armar_comando(linea);
        comando->comando[2] = NULL;
        if (strcmp(comando->comando[0], "INICIAR_PLANIFICACION") == 0) {
            //pthread_t hilo_planificar;
            //pthread_create(&hilo_planificar,NULL,(void*)planificar_procesos,NULL);
            //pthread_detach(hilo_planificar)
            iniciar_planificador_largo_plazo();
            inicializar_hilos_planificador();
            //sem_post(&inicializar_planificador); //Revisar esto segun planificacion
            log_info(kernel_logger, "Se inicia la planificación");
        }//HECHOOOOOO
        else if(strcmp(comando->comando[0], "MULTIPROGRAMACION") == 0){
            int nuevo_grado_multiprogramacion = atoi(comando->comando[1]);
            actualizar_grado_multiprogramacion(nuevo_grado_multiprogramacion);
        }//HECHOOOOOO
        else if(strcmp(comando->comando[0], "PROCESO_ESTADO") == 0){
            listar_procesos_por_estado();
        }
        else if(strcmp(comando->comando[0], "DETENER_PLANIFICACION") == 0){
            //mostrar
            log_info(kernel_logger, "detener plani");
        }
        else if (strcmp(comando->comando[0], "EJECUTAR_SCRIPT") == 0) {
            char *path = comando->comando[1];
            log_info(kernel_logger, "se ejecuta script y el path es %s", path);
            ejecutar_script(path);
            iniciar_planificador_largo_plazo();
            inicializar_hilos_planificador();
        }//HECHOOOOOO
        else if (strcmp(comando->comando[0], "INICIAR_PROCESO") == 0) {
            // Iniciar proceso
			log_info(kernel_logger, "Se inicia el proceso");
            iniciar_proceso(comando->comando[1]);
            iniciar_planificador_largo_plazo();
            inicializar_hilos_planificador();

        }//HECHOOOOOO
        else if (strcmp(comando->comando[0], "FINALIZAR_PROCESO") == 0) {
            char* pid = comando->comando[1];
            int pid_finalizado = atoi(pid); 
            log_info(kernel_logger, "finalizando proceso... %d", pid_finalizado);
            finalizar_proceso(pid_finalizado);
        }//HECHOOO
        else if (strcmp(comando->comando[0], "EXIT") == 0) {
            log_info(kernel_logger, "Se cierra la consola");
            break;
        }//UNICAMENTE PARA PRUEBA
        else if (strcmp(comando->comando[0], "MOSTRAR") == 0){
            t_paquete* paquete = crear_paquete(MOSTRAR);
            enviar_paquete(paquete, socket_conexion_memoria);
        }
        else if (strcmp(comando->comando[0],"MOSTRAR_PROCESOS") == 0){
            mostrar_procesos();
        }
        else {
            log_error(kernel_logger, "Comando no reconocido");
        }

        free(linea);
        destruir_comando(comando);
    }
    return 0;
}

void ejecutar_script(char* path) {
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        log_error(kernel_logger, "Error al abrir el archivo: %s", path);
        return;
    }

    char buffer[BUFFER_SIZE];
    while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
        // Remover el salto de línea
        buffer[strcspn(buffer, "\n")] = 0;


        // Verificar que la línea comience con "INICIAR_PROCESO"
        if (strncmp(buffer, "INICIAR_PROCESO ", 16) == 0) {
            // Obtener el path del proceso
            char* proceso_path = string_new();
            string_append(&proceso_path, buffer + 16); // +16 para saltar "INICIAR_PROCESO "
            iniciar_proceso(proceso_path);
            free(proceso_path);
        }
    }

    fclose(file);
}

void mostrar_procesos(){
    dictionary_iterator(tabla_procesos, iterator);

}
void actualizar_grado_multiprogramacion(int nuevo_grado_multiprogramacion){
    //DETENER PREVIAMENTE LA PLANIFICACION
    sem_destroy(&grado_multiprogramacion);
    sem_init(&grado_multiprogramacion,0,nuevo_grado_multiprogramacion);
    log_info(kernel_logger, "El grado de multiprogramacion se actualizo a %d", nuevo_grado_multiprogramacion);
}
void listar_procesos_por_estado(){
    dictionary_iterator(tabla_procesos, iterator);
}
void iterator (char* key,void* elemento){
    t_list* procesos = (t_list*)elemento;
    log_info(kernel_logger, "Estado: %s", key);
    for (int i = 0; i < list_size(procesos); i++) {
        t_pcb* pcb = list_get(procesos, i);
        log_info(kernel_logger, "PID: %d", pcb->pid);
        log_info(kernel_logger, "Path: %d", pcb->estado);
        log_info(kernel_logger, "Program Counter: %d", pcb->program_counter);
        log_info(kernel_logger, "Quantum: %d", pcb->quantum);
        log_info(kernel_logger, "Registros CPU: AX: %d, BX: %d, CX: %d, DX: %d, EAX: %d, EBX: %d, ECX: %d, EDX: %d, SI: %d, DI: %d", pcb->registros_cpu->regAX, pcb->registros_cpu->regBX, pcb->registros_cpu->regCX, pcb->registros_cpu->regDX, pcb->registros_cpu->regEAX, pcb->registros_cpu->regEBX, pcb->registros_cpu->regECX, pcb->registros_cpu->regEDX, pcb->registros_cpu->regSI, pcb->registros_cpu->regDI);

    }
}
t_pcb* crear_pcb() {
    t_pcb* pcb = (t_pcb*)malloc(sizeof(t_pcb));
    if (pcb == NULL) {
        return NULL; // Manejo de error
    }

    pcb->registros_cpu = malloc(sizeof(t_registros_cpu));//EVITA SEGMENTATION FAULT
    if (pcb->registros_cpu == NULL) {
        free(pcb);
        return NULL; // Manejo de error
    }

    pcb->estado = NEW;
    pcb->pid = obtener_pid();
    pcb->quantum = QUANTUM;
    pcb->program_counter = 0;
    pcb->registros_cpu->regAX = 0;
    pcb->registros_cpu->regBX = 0;
    pcb->registros_cpu->regCX = 0;
    pcb->registros_cpu->regDX = 0;
    pcb->registros_cpu->regEAX = 0;
    pcb->registros_cpu->regEBX = 0;
    pcb->registros_cpu->regECX = 0;
    pcb->registros_cpu->regEDX = 0;
    pcb->registros_cpu->regSI = 0;
    pcb->registros_cpu->regDI = 0;
    pcb->instruccion = malloc(sizeof(t_instruccion));//EVITA SEGMENTATION FAULT
    if (pcb->instruccion == NULL) {
        free(pcb->registros_cpu);
        free(pcb);
        return NULL; 
    }
    pcb->instruccion->codigo_operacion = NULL;
    pcb->instruccion->parametros[0] = NULL;
    pcb->instruccion->parametros[1] = NULL;
    pcb->instruccion->parametros[2] = NULL;
    pcb->instruccion->codigo_operacion_tamanio = 0;
    pcb->instruccion->parametro1_tamanio = 0;
    pcb->instruccion->parametro2_tamanio = 0;
    pcb->instruccion->parametro3_tamanio = 0;  
    return pcb;
}

void iniciar_proceso(char *path) {
    if (path == NULL) {
        log_error(kernel_logger, "No se ingresó un path válido");
        return;
    }
    t_pcb* pcb = crear_pcb();
    if (pcb == NULL) {
        log_error(kernel_logger, "No se pudo crear el PCB");
        return;
    }
    pcb->path = strdup(path);
    pcb->tamanio_path = sizeof(char) * strlen(path) + 1;
    if (pcb->path == NULL) {
        log_error(kernel_logger, "No se pudo reservar memoria para el path o no se guardó correctamente");
        free(pcb->instruccion);
        free(pcb->registros_cpu);
        free(pcb);
        return;
    }
    solicitar_creacion_memoria(pcb);  
    agregar_a_new(pcb);
    log_info(kernel_logger, "Se crea el proceso con pid %d", pcb->pid);
}
void solicitar_creacion_memoria(t_pcb* pcb) {
    t_paquete* paquete = crear_paquete(INICIAR_PROCESO);
    if (paquete == NULL)
    {
        log_error(kernel_logger, "No se pudo crear el paquete para la solicitud de creación de proceso");
    }
    
    // Serializar el PID por las dudas agrego todo el pcb para que no haya segmentatio fault
    
    agregar_a_paquete(paquete, &pcb->pid, sizeof(uint32_t));
    agregar_a_paquete(paquete,&pcb->tamanio_path,sizeof(uint32_t));
    agregar_a_paquete(paquete, pcb->path, sizeof(char) * pcb->tamanio_path);
    agregar_a_paquete(paquete, &pcb->program_counter, sizeof(uint32_t));
    agregar_a_paquete(paquete, &pcb->quantum, sizeof(uint32_t));
    agregar_a_paquete(paquete, pcb->registros_cpu, sizeof(t_registros_cpu));
    agregar_a_paquete(paquete, pcb->instruccion, sizeof(t_instruccion));
    agregar_a_paquete(paquete, &pcb->estado, sizeof(t_estado));
    int result = enviar_paquete(paquete, socket_conexion_memoria);
    if (result < 0) {
        log_error(kernel_logger, "Error al enviar el paquete a memoria");
    } else {
        log_info(kernel_logger, "Paquete enviado a memoria correctamente");
    }
    log_info(kernel_logger, "Solicitud de creación de tabla de páginas enviada a memoria para el proceso con pid %d", pcb->pid);
    // Liberar el paquete
    eliminar_paquete(paquete);
}

 

void destruir_comando(t_comando *comando) {
    if (comando == NULL) {
        return;
    }
    free(comando->comando[0]);
    free(comando->comando[1]);
}


// log_info(kernel_logger, "Path: %s", pcb->path);
//     log_info(kernel_logger, "Program Counter: %d", pcb->program_counter);
//     log_info(kernel_logger, "Quantum: %d", pcb->quantum);
//     log_info(kernel_logger, "Registros CPU: AX: %d, BX: %d, CX: %d, DX: %d, EAX: %d, EBX: %d, ECX: %d, EDX: %d, PC: %d, SI: %d, DI: %d", pcb->registros_cpu->regAX, pcb->registros_cpu->regBX, pcb->registros_cpu->regCX, pcb->registros_cpu->regDX, pcb->registros_cpu->regEAX, pcb->registros_cpu->regEBX, pcb->registros_cpu->regECX, pcb->registros_cpu->regEDX, pcb->registros_cpu->regSI, pcb->registros_cpu->regDI);
//     log_info(kernel_logger, "Instruccion: %s", pcb->instruccion->codigo_operacion);
//     log_info(kernel_logger, "Parametros: %s, %s, %s", pcb->instruccion->parametros[0], pcb->instruccion->parametros[1], pcb->instruccion->parametros[2]);

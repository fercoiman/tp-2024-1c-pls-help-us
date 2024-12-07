#include "memoria.h"
#include <commons/collections/dictionary.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

t_dictionary* d_instrucciones_x_pid;
t_dictionary* tabla_paginas;
t_dictionary* lista_interfaz;
t_list* lista_stdin;
t_list* lista_stdout;
t_list* lista_dialfs;
pthread_t atencion_cpu;
pthread_t atencion_kernel;
pthread_t hilo_io;
extern int servidor_memoria;
int main(int argc, char* argv[]) {
    // Inicializar memoria
    inicializa_memoria();
    //creo estructuras
    d_instrucciones_x_pid = dictionary_create();
    tabla_paginas = dictionary_create();
    lista_interfaz = dictionary_create();
    lista_stdin = list_create();
    lista_stdout = list_create();
    lista_dialfs = list_create();
    // Espero conexiones
    log_info(memoria_logger, "Esperando Conexiones...");
    esperar_conexiones();
    pthread_t hilo_io;
    int* args = malloc(sizeof(int));
    *args = servidor_memoria;
    pthread_create(&hilo_io, NULL, esperar_io, args);
    pthread_detach(hilo_io);
    log_info(memoria_logger, "Todas las conexiones fueron aceptadas.");

    int *arg_cpu = malloc(sizeof(int));
    int *arg_kernel = malloc(sizeof(int));
    //int *arg_io = malloc(sizeof(int));
    if (arg_cpu == NULL || arg_kernel == NULL) {
        perror("Error al asignar memoria para los argumentos");
        exit(EXIT_FAILURE);
    }

    *arg_cpu = cliente_cpu;
    *arg_kernel = cliente_kernel;

    //pthread_t atencion_io;
    pthread_create(&atencion_cpu, NULL, atender_clientes, arg_cpu);
    pthread_create(&atencion_kernel, NULL, atender_clientes, arg_kernel);
    //pthread_create(&atencion_io, NULL, atender_clientes, arg_kernel);

    // Esperar que los hilos terminen
    log_info(memoria_logger, "Esperando solicitudes de kernel o cpu...");
    pthread_join(atencion_kernel, NULL);
    pthread_join(atencion_cpu, NULL);
    
    //pthread_join(atencion_io, NULL);

    // Liberar recursos y terminar programa
    
    return 0;
}
void* atender_io(void* args){
    return NULL;

}
void handle_kernel_disconnection() {
    log_error(memoria_logger, "El kernel se ha desconectado. Finalizando todas las conexiones y liberando recursos.");
    free(memoria);
    // Liberar recursos
    liberar_memoria_instrucciones();
    dictionary_destroy(tabla_paginas);
    log_info(memoria_logger, "diccionarios finalizados.");
    
    list_destroy_and_destroy_elements(lista_stdin, free);
    list_destroy_and_destroy_elements(lista_stdout, free);
    list_destroy_and_destroy_elements(lista_dialfs, free);

    log_error(memoria_logger, "Recursos liberados. Programa terminado.");
    exit(EXIT_FAILURE);
}

void *atender_clientes(void *args) {
    int cliente_fd = *((int*) args);
    free(args); 
    while (1) {
        int cod_op = recibir_operacion(cliente_fd);
        if (cod_op == -1) {
            // Si se recibe -1, manejar desconexión del cliente
            log_error(memoria_logger, "El cliente se desconectó. Terminando servidor.");
            handle_kernel_disconnection();
            return NULL;
        }
        switch (cod_op) {
            case HANDSHAKE:
            recibir_handshake(cliente_fd);
            break;
            case INICIAR_PROCESO:
                log_info(memoria_logger, "Iniciando Proceso.");
                manejar_solicitud_creacion_proceso(cliente_fd);
                break;
            case FINALIZAR_PROCESO:
                // Logica para finalizar el proceso
                break;
            case INSTRUCCION:
                log_info(memoria_logger, "Solicitud INSTRUCCION del socket %d: ", cliente_fd);
                enviar_instruccion_a_cpu(cliente_fd);
                break;
            case ACCESO_A_PAGINA:
                acceso_pagina(cliente_fd);
                break;
            case WRITE_MEMORY:
                log_trace(memoria_logger, "Solicitud WRITE_MEMORY");
                escribir_en_memoria(cliente_fd);
                break;
            case READ_MEMORY:
                leer_de_memoria(cliente_fd);
                break;
            case RESIZE_MEMORY:
                resize_memory(cliente_fd);
                break;
            default:
                log_info(memoria_logger, "Operacion desconocidas");
                break;
        }
    }
    return NULL;
}
void escribir_en_memoria(int cliente_fd) {
    uint32_t pid;
    uint32_t direccion;
    uint32_t valor;

    t_paquete *paquete = recibir_paquete(cliente_fd);
    paquete->buffer->offset = 0;
    buffer_read(&pid, paquete->buffer, sizeof(uint32_t));
    buffer_read(&direccion, paquete->buffer, sizeof(uint32_t));
    buffer_read(&valor, paquete->buffer, sizeof(uint32_t));
    log_trace(memoria_logger, "Los valores son PID: %d DIRECCION: %d VALOR: %d", pid, direccion, valor);

    // Obtener la lista de páginas para el proceso
    t_list *lista_paginas = dictionary_get(tabla_paginas, string_itoa(pid));
    if (lista_paginas == NULL) {
        printf("No se encontró la tabla de páginas para el proceso %d\n", pid);
        return;
    }

    // Calcular el índice de la página y el offset dentro de la página
    uint32_t pagina_idx = direccion / TAM_PAGINA;
    log_trace(memoria_logger, "El índice de la página es %d", pagina_idx);

    uint32_t offset = direccion % TAM_PAGINA;
    log_trace(memoria_logger, "El offset es %d", offset);

    uint32_t remaining_bytes = sizeof(uint32_t);
    uint32_t bytes_written = 0;

    while (remaining_bytes > 0) {
        t_pagina *pagina = NULL;
        if (pagina_idx < list_size(lista_paginas)) {
            pagina = list_get(lista_paginas, pagina_idx);
        }

        // Si la página no tiene un marco asignado, asignarlo
        if (pagina == NULL || pagina->marco == -1) {
            int marco_asignado = asignar_marco(pid, pagina_idx);
            if (marco_asignado == -1) {
                printf("No se pudo asignar un marco para la página %d del proceso %d\n", pagina_idx, pid);
                return;
            }

            // Si la página no existía, crear una nueva entrada en la lista
            if (pagina == NULL) {
                pagina = malloc(sizeof(t_pagina));
                pagina->pagina = pagina_idx;
                pagina->marco = marco_asignado;
                pagina->offset = 0;
                list_add_in_index(lista_paginas, pagina_idx, pagina);
            } else {
                
                pagina->marco = marco_asignado;
            }
        }

        uint32_t espacio_disponible = TAM_PAGINA - offset;
        uint32_t bytes_a_escribir = remaining_bytes < espacio_disponible ? remaining_bytes : espacio_disponible;

        
        uint32_t direccion_fisica = pagina->marco * TAM_PAGINA + offset;
        log_trace(memoria_logger, "La dirección física es %d", direccion_fisica);

        
        memcpy((char*)memoria + direccion_fisica, (char*)&valor + bytes_written, bytes_a_escribir);
        log_info(memoria_logger, "Parte del valor %d escrito en la dirección %d del proceso %d", valor, direccion_fisica, pid);

        remaining_bytes -= bytes_a_escribir;
        bytes_written += bytes_a_escribir;
        pagina_idx++;
        offset = 0;  
    }

    enviar_mensaje("OK", cliente_fd, WRITE_MEMORY_RESPUESTA);
}
void leer_de_memoria(int cliente_fd) {
    uint32_t pid;
    uint32_t direccion;
    uint32_t num_bytes;
    char *buffer;
    
    t_paquete *paquete = recibir_paquete(cliente_fd);
    paquete->buffer->offset = 0;
    buffer_read(&pid, paquete->buffer, sizeof(uint32_t));
    buffer_read(&direccion, paquete->buffer, sizeof(uint32_t));
    buffer_read(&num_bytes, paquete->buffer, sizeof(uint32_t)); // Leer la cantidad de bytes a leer
    log_trace(memoria_logger, "Los valores son PID: %d DIRECCION: %d NUM_BYTES: %d", pid, direccion, num_bytes);

    buffer = malloc(num_bytes); // Asignar el buffer para almacenar los bytes leídos
    memset(buffer, 0, num_bytes); // Inicializar el buffer con ceros

    // Obtener la lista de páginas para el proceso
    t_list *lista_paginas = dictionary_get(tabla_paginas, string_itoa(pid));
    if (lista_paginas == NULL) {
        log_error(memoria_logger,"No se encontró la tabla de páginas para el proceso %d", pid);
        enviar_mensaje("ERROR: Tabla de páginas no encontrada", cliente_fd, READ_MEMORY_RESPUESTA);
        free(buffer);
        return;
    }

    // Calcular el índice de la página y el offset dentro de la página
    uint32_t pagina_idx = direccion / TAM_PAGINA;
    log_trace(memoria_logger, "El índice de la página es %d", pagina_idx);

    uint32_t offset = direccion % TAM_PAGINA;
    log_trace(memoria_logger, "El offset es %d", offset);

    uint32_t remaining_bytes = num_bytes;
    uint32_t bytes_read = 0;

    while (remaining_bytes > 0) {
        t_pagina *pagina = NULL;
        if (pagina_idx < list_size(lista_paginas)) {
            pagina = list_get(lista_paginas, pagina_idx);
        }

        if (pagina == NULL || pagina->marco == -1) {
            log_error(memoria_logger,"No se encontró la página %d para el proceso %d", pagina_idx, pid);
            enviar_mensaje("ERROR: Página no encontrada", cliente_fd, READ_MEMORY_RESPUESTA);
            free(buffer);
            return;
        }

        uint32_t espacio_disponible = TAM_PAGINA - offset;
        uint32_t bytes_a_leer = remaining_bytes < espacio_disponible ? remaining_bytes : espacio_disponible;

        // Calcular la dirección física
        uint32_t direccion_fisica = pagina->marco * TAM_PAGINA + offset;
        log_trace(memoria_logger, "La dirección física es %d", direccion_fisica);

        // Leer parte del valor de la memoria
        memcpy(buffer + bytes_read, (char*)memoria + direccion_fisica, bytes_a_leer);
        log_info(memoria_logger, "Parte del valor leído de la dirección %d del proceso %d", direccion_fisica, pid);

        remaining_bytes -= bytes_a_leer;
        bytes_read += bytes_a_leer;
        pagina_idx++;
        offset = 0;  // Después de la primera página, el offset siempre será 0
    }

    // Enviar el valor leído de vuelta al cliente
    t_paquete* respuesta = crear_paquete(READ_MEMORY_RESPUESTA);
    agregar_a_paquete(respuesta, buffer, num_bytes);
    enviar_paquete(respuesta, cliente_fd);
    eliminar_paquete(respuesta);
    free(buffer);
}
void manejar_solicitud_creacion_proceso(int cliente_fd) {
    // Recibir el paquete del kernel
    t_paquete* paquete = recibir_paquete(cliente_fd);
    // Verificar si se recibió correctamente el paquete
    if (paquete == NULL) {
        log_error(memoria_logger, "Error al recibir el paquete de solicitud de creación de proceso.");
        return;
    }
    // Inicializar el desplazamiento del buffer
    log_info(memoria_logger, "Paquete recibido correctamente. Deserializando...");
    paquete->buffer->offset = 0;
    // Deserializar el PID del paquete
    uint32_t pid;
    uint32_t tamanio_path;
    uint32_t program_counter;
    uint32_t quantum;
    t_registros_cpu* registros_cpu = malloc(sizeof(t_registros_cpu)); 
    t_instruccion* instruccion= malloc(sizeof(t_instruccion));
    t_estado estado;
    buffer_read(&pid, paquete->buffer, sizeof(uint32_t));
    buffer_read(&tamanio_path,paquete->buffer,sizeof(uint32_t));
    char* path= (char *)malloc(tamanio_path * sizeof(char));
    buffer_read(path, paquete->buffer, sizeof(char)*tamanio_path);
    buffer_read(&program_counter, paquete->buffer, sizeof(uint32_t));
    buffer_read(&quantum, paquete->buffer, sizeof(uint32_t));
    buffer_read(registros_cpu,paquete->buffer, sizeof(t_registros_cpu));
    buffer_read(instruccion,paquete->buffer,sizeof(t_instruccion));
    buffer_read(&estado, paquete->buffer, sizeof(t_estado));
    interpretar_pseudocodigo(path,pid);
    crear_tabla_paginas(pid);
    free(path);
    free(instruccion);
    free(registros_cpu);
    eliminar_paquete(paquete);
}
void* esperar_io(void* args){

    while (1) {
		int cliente_fd;
		log_info(memoria_logger,"Listo para recibir IO's");
		cliente_fd = esperar_cliente(servidor_memoria,memoria_logger,"IO");
		conexion_interfaz* conexion = malloc(sizeof(conexion_interfaz));
		conexion->socket = cliente_fd;
		log_info(memoria_logger,"el socket es %d",cliente_fd);
		log_info(memoria_logger,"esperando opcode");
		int cod_op = recibir_operacion(cliente_fd);
		log_info(memoria_logger,"El codigo de operacion recibidou es %d",cod_op);
		switch (cod_op)
		{
		case HANDSHAKE:
			recibir_handshake(cliente_fd);
			log_info(memoria_logger,"Handshake recibido");
			int codigo = recibir_operacion(cliente_fd);
			if (codigo != NUEVA_INTERFAZ){
				log_error(memoria_logger,"Operacion no reconocida");
				close(cliente_fd);
				continue;
			}
			log_info(memoria_logger,"Nueva interfaz");
			char* nombre = registrar_interfaz(cliente_fd);
			if (nombre != NULL){
			log_info(memoria_logger,"Interfaz %s registrada con exito",nombre);
			dictionary_put(lista_interfaz,nombre, conexion);
			cliente_fd = 0;
			}else {
			log_error(memoria_logger,"No se pudo registrar la interfaz");
			close(cliente_fd);
			continue;
			}
			break;
			log_error(memoria_logger,"Operacion no reconocida");
			break;
		}
    }
}	
char* registrar_interfaz(int cliente_fd){
    t_paquete* paquete = recibir_paquete(cliente_fd);
    paquete->buffer->offset = 0;
    int size;
    buffer_read(&size, paquete->buffer, sizeof(int));
    char* nombre = malloc(sizeof(char) * size);
    buffer_read(nombre, paquete->buffer, sizeof(char) * size);
    int size1;
    buffer_read(&size1, paquete->buffer, sizeof(int));
    char* tipo_interfaz = malloc(sizeof(char) * size1);
    buffer_read(tipo_interfaz, paquete->buffer, sizeof(char) * size1);
    if (strcmp(tipo_interfaz,"STDIN") == 0){
        list_add(lista_stdin, nombre);
        log_info(memoria_logger,"Interfaz stdin registrada: %s",nombre);
        return nombre;
    }else if (strcmp(tipo_interfaz,"STDOUT") == 0){
        list_add(lista_stdout, nombre);
        log_info(memoria_logger,"Interfaz stdout registrada: %s",nombre);
        return nombre;
    }else if (strcmp(tipo_interfaz,"DIALFS") == 0){
        list_add(lista_dialfs, nombre);
        log_info(memoria_logger,"Interfaz dialfs registrada: %s",nombre);
        return nombre;
    }else{
        log_error(memoria_logger,"Tipo de interfaz no reconocido");
        eliminar_paquete(paquete);
        return NULL;
    }
    log_info(memoria_logger,"Interfaz registrada: %s",nombre);
    eliminar_paquete(paquete);
}
//PARA MOSTRAR LAS INSTRUCCIONES DENTRO DE UN DICCIONARIO INSTRUCCIONES
void iterator(char* key, void* value) {
    t_list* lista_instrucciones = (t_list*) value;
    log_info(memoria_logger, "PID: %s", key);

    for (int i = 0; i < list_size(lista_instrucciones); i++) {
        t_instruccion* instruccion = list_get(lista_instrucciones, i);
        log_info(memoria_logger, "Instruccion: %s %s %s %s",
                 instruccion->codigo_operacion,
                 instruccion->parametros[0] ? instruccion->parametros[0] : "",
                 instruccion->parametros[1] ? instruccion->parametros[1] : "",
                 instruccion->parametros[2] ? instruccion->parametros[2] : "");
    }
}
void interpretar_pseudocodigo(char* path_instrucciones,uint32_t pid) {

    char* path = string_new();
    char* buffer = malloc(100);
    getcwd(buffer,100);
    log_info(memoria_logger, "Directorio actual: %s", buffer);
    string_append(&path,buffer);
    string_append(&path,"/");
    string_append(&path,path_instrucciones);
    log_info(memoria_logger, "Interpretando pseudocódigo del archivo %s", path);
    FILE* archivo = fopen(path, "r");
    // Comprobar si el archivo existe
    if (archivo == NULL) {
        log_error(memoria_logger, "Error en la apertura del archivo");
        free(path);
        return;
    }
    
    char* cadena = malloc(500);
    t_list* lista_instrucciones = list_create();
    // Leer el pseudocódigo, poner en la lista
    while (fgets(cadena, 500, archivo) != NULL) {
        // Eliminar el '\n' al final de la cadena si existe
        cadena[strcspn(cadena, "\n")] = '\0';
        // Crear t_instruccion
        t_instruccion* ptr_inst = malloc(sizeof(t_instruccion));
        // Leer el opcode
        char* token = strtok(cadena, " ");
        ptr_inst->codigo_operacion = strdup(token);
        ptr_inst->codigo_operacion_tamanio = strlen(ptr_inst->codigo_operacion) + 1;
        ptr_inst->parametros[0] = NULL;
        ptr_inst->parametros[1] = NULL;
        ptr_inst->parametros[2] = NULL;

        // Leer parámetros
        int n = 0;
        while ((token = strtok(NULL, " ")) != NULL && n < 3) {
            ptr_inst->parametros[n] = strdup(token);
            n++;
        }

        if (ptr_inst->parametros[0] != NULL) {
            ptr_inst->parametro1_tamanio = strlen(ptr_inst->parametros[0]) + 1;
        } else {
            ptr_inst->parametro1_tamanio = 0;
        }
        if (ptr_inst->parametros[1] != NULL) {
            ptr_inst->parametro2_tamanio = strlen(ptr_inst->parametros[1]) + 1;
        } else {
            ptr_inst->parametro2_tamanio = 0;
        }
        if (ptr_inst->parametros[2] != NULL) {
            ptr_inst->parametro3_tamanio = strlen(ptr_inst->parametros[2]) + 1;
        } else {
            ptr_inst->parametro3_tamanio = 0;
        }

        list_add(lista_instrucciones, ptr_inst);
        printf("Instruccion: %s %s %s %s\n", ptr_inst->codigo_operacion, ptr_inst->parametros[0], ptr_inst->parametros[1], ptr_inst->parametros[2]);
    }
    dictionary_put(d_instrucciones_x_pid, string_itoa(pid), lista_instrucciones);
    // Liberar recursos
    free(cadena);
    fclose(archivo);
    free(path);
    log_info(memoria_logger, "Pseudocódigo interpretado correctamente");
}
void crear_tabla_paginas(uint32_t pid) {
    // Crear la tabla de páginas para 1  proceso
    t_list* lista_paginas = list_create();
    t_pagina* pagina_marco = malloc(sizeof(t_pagina));
    pagina_marco->pagina = -1;
    pagina_marco->marco = -1;
    pagina_marco->offset = 0;
    list_add(lista_paginas, pagina_marco);
    dictionary_put(tabla_paginas, string_itoa(pid),lista_paginas);
    log_info(memoria_logger, "Tabla de páginas creada correctamente para el PID %d", pid);
}
void mostrar_lista_instrucciones_x_pid() {
    dictionary_iterator(d_instrucciones_x_pid, iterator);
}
uint32_t recibir_pid(int cliente_fd){
    uint32_t pid_recibido;
    uint32_t pc_recibido;
    t_paquete* paquete = recibir_paquete(cliente_fd);
    paquete->buffer->offset = 0;
    buffer_read(&pid_recibido, paquete->buffer, sizeof(uint32_t));
    buffer_read(&pc_recibido, paquete->buffer, sizeof(uint32_t));
    return pid_recibido;
}

void enviar_instruccion_a_cpu(int cpu_cliente){
    uint32_t pid;
    uint32_t program_counter;
    recibir_programcounter(cpu_cliente,&pid,&program_counter);
	t_list* lista_instrucciones = dictionary_get(d_instrucciones_x_pid,string_itoa(pid));

	if(lista_instrucciones == NULL){
		log_error(memoria_logger, "Proceso sin lista de instrucciones.");
	}
	// //agarro 1 instruccion de la lista
	t_instruccion *instruccion = list_get(lista_instrucciones,program_counter);
    t_paquete* paquete_a_enviar = crear_paquete(INSTRUCCION);    
	//Una vez obtenida la instruccion, creo el paquete y serializo
	agregar_a_paquete(paquete_a_enviar,&instruccion->codigo_operacion_tamanio,sizeof(int));
    agregar_a_paquete(paquete_a_enviar,instruccion->codigo_operacion,sizeof(char) * instruccion->codigo_operacion_tamanio);
    agregar_a_paquete(paquete_a_enviar,&(instruccion->parametro1_tamanio),sizeof(int));
    agregar_a_paquete(paquete_a_enviar,&(instruccion->parametro2_tamanio),sizeof(int));
    agregar_a_paquete(paquete_a_enviar,&(instruccion->parametro3_tamanio),sizeof(int));
	agregar_a_paquete(paquete_a_enviar,instruccion->parametros[0],sizeof(char)* instruccion->parametro1_tamanio);
	agregar_a_paquete(paquete_a_enviar,instruccion->parametros[1],sizeof(char)* instruccion->parametro2_tamanio);
	agregar_a_paquete(paquete_a_enviar,instruccion->parametros[2],sizeof(char)* instruccion->parametro3_tamanio);
     
    enviar_paquete(paquete_a_enviar,cpu_cliente);  
    retardo_respuesta(RETARDO_RESPUESTA); 
    eliminar_paquete(paquete_a_enviar);
}
void recibir_programcounter(int socket_cliente, uint32_t* pid,uint32_t* program_counter ){
	int size;
	int desplazamiento = 0;
	void * buffer;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(program_counter, buffer + desplazamiento, sizeof(uint32_t));
		desplazamiento+=sizeof(uint32_t);

		memcpy(pid, buffer+desplazamiento, sizeof(uint32_t));
		desplazamiento+=sizeof(uint32_t);
	}
	free(buffer);

}
void retardo_respuesta(int retardo) {
    usleep(retardo*1000);
}
void acceso_pagina(int cliente_fd){
    // Recibir el paquete del kernel
    uint32_t pid,pagina;
    recibir_pagina(cliente_fd, &pid, &pagina);

    t_list* lista_paginas = dictionary_get(tabla_paginas, string_itoa(pid));

    if(lista_paginas == NULL){
        log_error(memoria_logger, "Proceso sin tabla de páginas.");
    }
    bool pagina_encontrada = false;
    for (int i = 0; i < list_size(lista_paginas); i++) {
        t_pagina* pagina_actual = list_get(lista_paginas, i);
        if (pagina_actual->pagina == pagina) {
            log_info(memoria_logger, "Página %d encontrada en el marco %d.", pagina, pagina_actual->marco);
            uint32_t marco = pagina_actual->marco;
            enviar_respuesta(cliente_fd, marco);
            pagina_encontrada = true;
            break;
        }
    }

    if (!pagina_encontrada) {
        int marco = asignar_marco(pid, pagina);
        if (marco == -1){
            log_info(memoria_logger, "Out of Memory");
        }
        enviar_respuesta(cliente_fd, marco);
        log_info(memoria_logger, "Página %d asignada a un marco.", pagina);  
    }


}

void recibir_pagina(int cliente_fd, uint32_t* pid, uint32_t* pagina) {
    // Recibir el paquete del kernel
    t_paquete* paquete = recibir_paquete(cliente_fd);
    // Verificar si se recibió correctamente el paquete
    if (paquete == NULL) {
        log_error(memoria_logger, "Error al recibir el paquete de solicitud de creación de proceso.");
        return;
    }
    // Inicializar el desplazamiento del buffer
    log_info(memoria_logger, "Paquete recibido correctamente. Deserializando...");
    paquete->buffer->offset = 0;
    // Deserializar el PID del paquete
    buffer_read(pid, paquete->buffer, sizeof(uint32_t));
    buffer_read(pagina, paquete->buffer, sizeof(uint32_t));
    // Liberar memoria reservada
    eliminar_paquete(paquete);
}
void enviar_respuesta(int cliente_fd, uint32_t respuesta) {
    t_paquete* paquete = crear_paquete(MARCO);
    agregar_a_paquete(paquete, &respuesta, sizeof(uint32_t));
    enviar_paquete(paquete, cliente_fd);
    eliminar_paquete(paquete);
}
void resize_memory(int cliente_fd) {
    uint32_t tamanio, pid;
    t_paquete* paquete = recibir_paquete(cliente_fd);
    paquete->buffer->offset = 0;
    buffer_read(&pid, paquete->buffer, sizeof(uint32_t));
    buffer_read(&tamanio, paquete->buffer, sizeof(uint32_t));
    log_info(memoria_logger, "Nuevo tamaño de memoria para el proceso %d: %d", pid, tamanio);

    t_list* lista_paginas = dictionary_get(tabla_paginas, string_itoa(pid));
    if (lista_paginas == NULL) {
        log_error(memoria_logger, "No se encontró la tabla de páginas para el proceso %d", pid);
        enviar_mensaje("Out of Memory", cliente_fd, RESIZE_MEMORY_RESPUESTA);
        eliminar_paquete(paquete);
        return;
    }

    if (tamanio == 0) {
        // Liberar todos los marcos asignados al proceso
        int marcos_actuales = list_size(lista_paginas);
        for (int i = 0; i < marcos_actuales; i++) {
            t_pagina* pagina = list_remove(lista_paginas, 0);
            bitmap_marcos[pagina->marco] = 0; // Liberar el marco en el bitmap
            free(pagina);
        }
        log_info(memoria_logger, "Todos los marcos del proceso %d han sido liberados.", pid);
        enviar_mensaje("OK", cliente_fd, RESIZE_MEMORY_RESPUESTA);
        eliminar_paquete(paquete);
        return;
    }

    // Calcular el número actual de marcos asignados
    uint32_t marcos_actuales = list_size(lista_paginas);
    // Calcular el número de marcos necesarios para el nuevo tamaño
    uint32_t marcos_necesarios = (tamanio + TAM_PAGINA - 1) / TAM_PAGINA;

    if (marcos_necesarios > marcos_actuales) {
        // Necesitamos más marcos
        for (uint32_t i = marcos_actuales; i < marcos_necesarios; i++) {
            if (asignar_marco(pid, i) == -1) {
                // No hay suficientes marcos disponibles
                log_error(memoria_logger, "No hay marcos suficientes para redimensionar el proceso %d", pid);
                enviar_mensaje("Out of Memory", cliente_fd, RESIZE_MEMORY_RESPUESTA);
                eliminar_paquete(paquete);
                return;
            }
        }
    } else if (marcos_necesarios < marcos_actuales) {
        // Liberar marcos sobrantes
        for (uint32_t i = marcos_necesarios; i < marcos_actuales; i++) {
            t_pagina* pagina = list_remove(lista_paginas, marcos_necesarios);
            bitmap_marcos[pagina->marco] = 0; // Liberar el marco en el bitmap
            free(pagina);
        }
    }

    log_info(memoria_logger, "Memoria del proceso %d redimensionada correctamente.", pid);
    enviar_mensaje("OK", cliente_fd, RESIZE_MEMORY_RESPUESTA);
    eliminar_paquete(paquete);
}


//FUNCION DE CPU PARA SOLICITAR MARCO DE MEMORIA
// uint32_t pid = 1000;
//                 uint32_t pagina = 5;
// 				sleep(15);
// void solicitar_marco_memoria(int socket_memoria, uint32_t pid, uint32_t pagina) {
//     t_paquete* paquete = crear_paquete(ACCESO_A_PAGINA);
//     agregar_a_paquete(paquete, &pid, sizeof(uint32_t));
//     agregar_a_paquete(paquete, &pagina, sizeof(uint32_t));
//     enviar_paquete(paquete, socket_memoria);
//     eliminar_paquete(paquete);

//     // Esperar la respuesta de memoria
// }

//DESTROYERS
void t_instruccion_destroyer(void* instruccion) {
    t_instruccion* instruccion_a_eliminar = (t_instruccion*)instruccion;
    if (instruccion_a_eliminar != NULL) {
        free(instruccion_a_eliminar->codigo_operacion);
        for (int i = 0; i < 3; i++) {
            free(instruccion_a_eliminar->parametros[i]);
        }
        free(instruccion_a_eliminar);
    }
}

// Función para destruir las listas contenidas en el diccionario

void liberar_lista_instrucciones(char* key, void* element) {
    t_list* lista_instrucciones = (t_list*)element;

    void liberar_instruccionn(void* elemento) {
        t_instruccion* instruccion = (t_instruccion*)elemento;
        free(instruccion->codigo_operacion);

        for (int i = 0; i < 3; i++) {
            if (instruccion->parametros[i] != NULL) {
                free(instruccion->parametros[i]);
                
            }
        }

        free(instruccion);
    }

    // Liberar cada instrucción en la lista
    list_destroy_and_destroy_elements(lista_instrucciones, liberar_instruccionn);
    // Liberar la lista
    free(lista_instrucciones);
}
void liberar_memoria_instrucciones() {
    dictionary_iterator(d_instrucciones_x_pid, liberar_lista_instrucciones);
    // Liberar el diccionario
    dictionary_destroy(d_instrucciones_x_pid);
}
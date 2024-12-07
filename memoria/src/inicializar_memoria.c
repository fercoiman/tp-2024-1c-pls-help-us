#include "inicializar_memoria.h"

void inicializa_memoria(){
    inicializar_loggers_memoria();
    inicializar_configs_memoria();
	// imprimirPrueba();
}

void inicializar_loggers_memoria(){
    memoria_logger = log_create("MEMORIA.log","LOGGER",1,LOG_LEVEL_TRACE);
	if(memoria_logger == NULL){
		perror("No se pudo crear el logger");
		exit(EXIT_FAILURE);
	}
}
void inicializar_configs_memoria(){

    memoria_config = config_create("MEMORIA.config");
	if(memoria_config == NULL){
		perror("Error al intentar cargar el config");
		exit(EXIT_FAILURE);
	}

    PUERTO_ESCUCHA = config_get_string_value(memoria_config,"PUERTO_ESCUCHA");
	TAM_PAGINA = config_get_int_value(memoria_config,"TAM_PAGINA");
	TAM_MEMORIA = config_get_int_value(memoria_config,"TAM_MEMORIA");
    RETARDO_RESPUESTA = config_get_int_value(memoria_config,"RETARDO_RESPUESTA");
    PATH_SCRIPTS= config_get_string_value(memoria_config,"PATH_INSTRUCCIONES");
	paginar_memoria();
}
void paginar_memoria() {
    cantidad_marcos = TAM_MEMORIA / TAM_PAGINA;

    // Inicializar el bitmap
    bitmap_marcos = calloc(cantidad_marcos, sizeof(char));
    if (bitmap_marcos == NULL) {
        perror("Error al asignar memoria para el bitmap");
        exit(EXIT_FAILURE);
    }

    // Inicializar la memoria contigua
    memoria = malloc(TAM_MEMORIA);
    if (memoria == NULL) {
        perror("Error al asignar memoria contigua");
        exit(EXIT_FAILURE);
    }

    // Inicializar todos los marcos como libres
    memset(bitmap_marcos, 0, cantidad_marcos * sizeof(char));
    memset(memoria, 0, TAM_MEMORIA);
}
void mostrar_estado_marcos() {
    printf("Estado de los marcos:\n");
    char* marcos = bitmap_marcos;
    for (int i = 0; i < cantidad_marcos; i++) {
        if (marcos[i] == 0) {
            printf("Marco %d: Libre\n", i);
        } else {
            printf("Marco %d: Ocupado\n", i);
        }
    }
}

void marcar_marco_ocupado(int marco) {
    char* marcos = (char*)bitmap_marcos;
    if (marco >= 0 && marco < cantidad_marcos) {
        marcos[marco] = 1;
    } else {
        log_error(memoria_logger, "Out of memory");
    }
}

void marcar_marco_libre(int marco) {
    char* marcos = (char*)bitmap_marcos;
    if (marco >= 0 && marco < cantidad_marcos) {
        marcos[marco] = 0;
    } else {
        log_error(memoria_logger, "Out of memory");
    }
}

int esta_marco_ocupado(int marco) {
    char* marcos = (char*)bitmap_marcos;
    if (marco >= 0 && marco < cantidad_marcos) {
        return marcos[marco];
    } else {
        log_error(memoria_logger, "Out of memory");;
        return -1; // Indica un error
    }
}
int asignar_marco(uint32_t pid, uint32_t pagina) {
    char* pid_str = string_itoa(pid);
    t_list* lista_paginas = dictionary_get(tabla_paginas, pid_str);
    free(pid_str);

    if (lista_paginas == NULL) {
        log_error(memoria_logger, "El proceso %d no tiene una tabla de páginas.", pid);
        return -1;
    }

    // Buscar un marco libre
    int marco = -1;
    char* bitmap = (char*)bitmap_marcos;  // Convertir a char* para trabajar con el bitmap

    for (int i = 0; i < cantidad_marcos; i++) {
        if (bitmap[i] == 0) {
            marco = i;
            bitmap[i] = 1;  // Marcar el marco como ocupado
            break;
        }
    }

    if (marco == -1) {
        log_error(memoria_logger, "No hay marcos libres disponibles.");
        return -1;  // Error: no hay marcos libres
    }

    t_pagina* nueva_pagina = malloc(sizeof(t_pagina));
    nueva_pagina->pagina = pagina;
    nueva_pagina->marco = marco;
    list_add(lista_paginas, nueva_pagina);

    log_info(memoria_logger, "Página %d del proceso %d asignada al marco %d.", pagina, pid, marco);
    return marco;
}

// void imprimirPrueba(){
// 	log_info(memoria_logger,PUERTO_ESCUCHA);

// }


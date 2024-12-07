#include "levantar_interfaces.h"

void iniciar_interfaz(char* nombre , char* nombre_archivo){
    archivo_config = config_create(nombre_archivo);
    log_info(IO_logger, "Iniciando interfaz %s", nombre);
    log_info(IO_logger, "Leyendo archivo de configuracion %s", nombre_archivo);
    if (archivo_config == NULL) {
        log_error(IO_logger,
                "No se ha podido leer el archivo de configuracion");
        exit(EXIT_FAILURE);
    }
    TIPO_INTERFAZ = config_get_string_value(archivo_config,"TIPO_INTERFAZ");
    log_info(IO_logger, "Tipo de interfaz: %s", TIPO_INTERFAZ);
    if(!TIPO_INTERFAZ){
		    log_error(IO_logger,
				"No se ha leido correctamente alguno de los parametros del archivo de configuracion");
            config_destroy(archivo_config);
            exit(EXIT_FAILURE);
        	}     
    if (strcmp(TIPO_INTERFAZ, "GENERICA") == 0) {
        interfaz_generica* Interfaz = inicializar_generica(nombre);
        conectar_interfaz_kernel(Interfaz->nombre,Interfaz->tipo_interfaz);

        free(archivo_config);
        
    } else if (strcmp(TIPO_INTERFAZ, "STDIN") == 0) {
        interfaz_stdin* Interfaz = inicializar_stdin(nombre);
        conectar_interfaz_kernel(Interfaz->nombre,Interfaz->tipo_interfaz);
        conectar_interfaz_memoria(Interfaz->nombre,Interfaz->tipo_interfaz);
        
        free(archivo_config);
        
    } else if (strcmp(TIPO_INTERFAZ, "STDOUT") == 0) {
        interfaz_stdout* Interfaz = inicializar_stdout(nombre);
        conectar_interfaz_kernel(Interfaz->nombre,Interfaz->tipo_interfaz);
        conectar_interfaz_memoria(Interfaz->nombre,Interfaz->tipo_interfaz);
        free(archivo_config);
        
    } else if (strcmp(TIPO_INTERFAZ, "DIALFS") == 0) {
        interfaz_dialfs* Interfaz = inicializar_dialfs(nombre);
        conectar_interfaz_kernel(Interfaz->nombre,Interfaz->tipo_interfaz);
        conectar_interfaz_memoria(Interfaz->nombre,Interfaz->tipo_interfaz);
        free(archivo_config);
        
    } else {
        log_error(IO_logger, "El tipo de interfaz solicitado no es valido");
        exit(EXIT_FAILURE);
    }
    log_info(IO_logger, "Interfaz %s iniciada correctamente", nombre);
    
}


// void mostrar_interfaces(){
//     log_trace(IO_logger, "Interfaces GENERICAS:\n");
//     for(int i = 0;i<list_size(lista_generica);i++){
//         interfaz_generica* interfaz_gen = list_get(lista_generica,i);
//         log_info(IO_logger, "Nombre: %s", interfaz_gen->nombre);
//         log_info(IO_logger, "Tipo de Interfaz: %s", interfaz_gen->tipo_interfaz);
//         log_info(IO_logger, "Tiempo de Unidad de Trabajo: %d", interfaz_gen->tiempo_unidad_trabajo);
//         log_info(IO_logger, "IP del Kernel: %s", interfaz_gen->ip_kernel);
//         log_info(IO_logger, "Puerto del Kernel: %d\n", interfaz_gen->puerto_kernel);
//     }
//     log_trace(IO_logger, "Interfaces STDIN:\n");
//     for(int i = 0;i<list_size(lista_stdin);i++){
//         interfaz_stdin* interfaz_stdin = list_get(lista_stdin,i);
//         log_info(IO_logger, "Nombre: %s", interfaz_stdin->nombre);
//         log_info(IO_logger, "IP del Kernel: %s", interfaz_stdin->ip_kernel);
//         log_info(IO_logger, "Puerto del Kernel: %d", interfaz_stdin->puerto_kernel);
//         log_info(IO_logger, "IP de Memoria: %s", interfaz_stdin->ip_memoria);
//         log_info(IO_logger, "Puerto de Memoria: %d\n", interfaz_stdin->puerto_memoria);
//     }
//     log_trace(IO_logger, "Interfaces STDOUT:\n");
//     for(int i = 0;i<list_size(lista_stdout);i++){
//         interfaz_stdout* interfaz_stdout = list_get(lista_stdout,i);
//         log_info(IO_logger, "Nombre: %s", interfaz_stdout->nombre);
//         log_info(IO_logger, "IP del Kernel: %s", interfaz_stdout->ip_kernel);
//         log_info(IO_logger, "Puerto del Kernel: %d", interfaz_stdout->puerto_kernel);
//         log_info(IO_logger, "IP de Memoria: %s", interfaz_stdout->ip_memoria);
//         log_info(IO_logger, "Puerto de Memoria: %d", interfaz_stdout->puerto_memoria);
//         log_info(IO_logger, "Tiempo de Unidad de Trabajo: %d\n", interfaz_stdout->tiempo_unidad_trabajo);
//     }
//     log_trace(IO_logger, "Interfaces DIALFS:\n");
//     for(int i = 0;i<list_size(lista_dialfs);i++){
//         interfaz_dialfs* interfaz_dialfs = list_get(lista_dialfs,i);
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
//     }
// }

interfaz_generica* inicializar_generica(char* nombre){
    IP_KERNEL = config_get_string_value(archivo_config,"IP_KERNEL");
    PUERTO_KERNEL = config_get_string_value(archivo_config,"PUERTO_KERNEL");
    TIEMPO_UNIDAD_TRABAJO = config_get_int_value(archivo_config,"TIEMPO_UNIDAD_TRABAJO");
    interfaz_generica* nueva = malloc(sizeof(interfaz_generica));
    nueva->nombre = nombre;
    nueva->tipo_interfaz = TIPO_INTERFAZ;
    nueva->tiempo_unidad_trabajo = TIEMPO_UNIDAD_TRABAJO;
    nueva->ip_kernel = IP_KERNEL;
    nueva->puerto_kernel = atoi(PUERTO_KERNEL);
    nueva->estado = 0;
    if(!nueva->tipo_interfaz || !nueva->ip_kernel || !nueva->puerto_kernel){
            log_error(IO_logger,
                "No se ha leido correctamente alguno de los parametros del archivo de configuracion DE ACA");
                free(nueva);
            exit(EXIT_FAILURE);
        	}
    return nueva;
}

//PARA LA 3ER ENTREGA , PERO LO DEJO MODELADO
interfaz_stdin* inicializar_stdin(char* nombre){
    IP_MEMORIA = config_get_string_value(archivo_config,"IP_MEMORIA");
    PUERTO_MEMORIA = config_get_string_value(archivo_config,"PUERTO_MEMORIA");
    IP_KERNEL = config_get_string_value(archivo_config,"IP_KERNEL");
    PUERTO_KERNEL = config_get_string_value(archivo_config,"PUERTO_KERNEL");
    interfaz_stdin* nueva = malloc(sizeof(interfaz_stdin));
    if (nueva == NULL) {
        perror("Error al inicializar la interfaz STDIN");
        free(nueva);
        exit(EXIT_FAILURE);
    }
    nueva->nombre = nombre;
    nueva->tipo_interfaz = TIPO_INTERFAZ;
    nueva->ip_kernel = IP_KERNEL;
    nueva->puerto_kernel = atoi(PUERTO_KERNEL);
    nueva->ip_memoria = IP_MEMORIA;
    nueva->puerto_memoria = atoi(PUERTO_MEMORIA);
    nueva->estado = 0;
    if(!nueva->tipo_interfaz || !nueva->ip_kernel || !nueva->puerto_kernel || !nueva->ip_memoria || !nueva->puerto_memoria){
            log_error(IO_logger,
                "No se ha leido correctamente alguno de los parametros del archivo de configuracion");
                free(nueva);
            exit(EXIT_FAILURE);
        	}
    return nueva;
}

interfaz_stdout* inicializar_stdout(char* nombre){
    
    IP_MEMORIA = config_get_string_value(archivo_config,"IP_MEMORIA");
    PUERTO_MEMORIA = config_get_string_value(archivo_config,"PUERTO_MEMORIA");
    IP_KERNEL = config_get_string_value(archivo_config,"IP_KERNEL");
    PUERTO_KERNEL = config_get_string_value(archivo_config,"PUERTO_KERNEL");
    TIEMPO_UNIDAD_TRABAJO = config_get_int_value(archivo_config,"TIEMPO_UNIDAD_TRABAJO");
    interfaz_stdout* nueva = malloc(sizeof(interfaz_stdout));
    if (nueva == NULL) {
        perror("Error al inicializar la interfaz STDOUT");
        free(nueva);
        exit(EXIT_FAILURE);
    }
    nueva->nombre = nombre;
    nueva->tipo_interfaz = TIPO_INTERFAZ;
    nueva->tiempo_unidad_trabajo = TIEMPO_UNIDAD_TRABAJO;
    nueva->ip_kernel = IP_KERNEL;
    nueva->puerto_kernel = atoi(PUERTO_KERNEL);
    nueva->ip_memoria = IP_MEMORIA;
    nueva->puerto_memoria = atoi(PUERTO_MEMORIA);
    nueva->estado = 0;

    if(!nueva->tipo_interfaz || !nueva->ip_kernel || !nueva->puerto_kernel || !nueva->ip_memoria || !nueva->puerto_memoria){
            log_error(IO_logger,
                "No se ha leido correctamente alguno de los parametros del archivo de configuracion DE ACA");
                free(nueva);
            exit(EXIT_FAILURE);
        	}
    return nueva;
}

interfaz_dialfs* inicializar_dialfs(char* nombre){
    
    IP_MEMORIA = config_get_string_value(archivo_config,"IP_MEMORIA");
    PUERTO_MEMORIA = config_get_string_value(archivo_config,"PUERTO_MEMORIA");
    IP_KERNEL = config_get_string_value(archivo_config,"IP_KERNEL");
    PUERTO_KERNEL = config_get_string_value(archivo_config,"PUERTO_KERNEL");
    TIEMPO_UNIDAD_TRABAJO = config_get_int_value(archivo_config,"TIEMPO_UNIDAD_TRABAJO");
    PATH_BASE_DIALFS = config_get_string_value(archivo_config,"PATH_BASE_DIALFS");
    BLOCK_SIZE = config_get_int_value(archivo_config,"BLOCK_SIZE");
    BLOCK_COUNT = config_get_int_value(archivo_config,"BLOCK_COUNT");
    RETRASO_COMPACTACION = config_get_int_value(archivo_config,"RETRASO_COMPACTACION");

    interfaz_dialfs* nueva = malloc(sizeof(interfaz_dialfs));
    if (nueva == NULL) {
        perror("Error al inicializar la interfaz DIALFS");
        exit(EXIT_FAILURE);
    }
    
    nueva->nombre = nombre;
    nueva->tipo_interfaz = TIPO_INTERFAZ;
    nueva->tiempo_unidad_trabajo = TIEMPO_UNIDAD_TRABAJO;
    nueva->ip_kernel = IP_KERNEL;
    nueva->puerto_kernel = atoi(PUERTO_KERNEL);
    nueva->ip_memoria = IP_MEMORIA;
    nueva->puerto_memoria = atoi(PUERTO_MEMORIA);
    nueva->path_base_dialfs = PATH_BASE_DIALFS;
    nueva->block_size = BLOCK_SIZE;
    nueva->block_count = BLOCK_COUNT;
    nueva->retraso_compactacion = RETRASO_COMPACTACION;
    nueva->estado = 0;


    if (!nueva->tipo_interfaz || !nueva->ip_kernel || !nueva->puerto_kernel ||
        !nueva->ip_memoria || !nueva->puerto_memoria || !nueva->path_base_dialfs ||
        !nueva->block_size || !nueva->block_count) {
        log_error(IO_logger, "No se ha leido correctamente alguno de los parametros del archivo de configuracion");
        free(nueva);
        exit(EXIT_FAILURE);
    }

    return nueva;
}

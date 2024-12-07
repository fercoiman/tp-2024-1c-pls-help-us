#include "inicializar_kernel.h"

void inicializar_kernel(){
    inicializar_loggers_kernel();
    inicializar_configs_kernel();
}

void inicializar_loggers_kernel(){
    kernel_logger = log_create("KERNEL.log","LOGGER",1,LOG_LEVEL_TRACE);
	if(kernel_logger == NULL){
		perror("No se pudo crear el logger");
		exit(EXIT_FAILURE);
	}
}

void inicializar_configs_kernel(){

    kernel_config = config_create("KERNEL.config");
	if(kernel_config == NULL){
		perror("Error al intentar cargar el config");
		exit(EXIT_FAILURE);
	}
    
    PUERTO_ESCUCHA = config_get_string_value(kernel_config,"PUERTO_ESCUCHA");
    IP_MEMORIA = config_get_string_value(kernel_config,"IP_MEMORIA");
    PUERTO_MEMORIA = config_get_string_value(kernel_config,"PUERTO_MEMORIA");
    IP_CPU = config_get_string_value(kernel_config,"IP_CPU");
    PUERTO_CPU_DISPATCH = config_get_string_value(kernel_config,"PUERTO_CPU_DISPATCH");
    PUERTO_CPU_INTERRUPT = config_get_string_value(kernel_config,"PUERTO_CPU_INTERRUPT");
    ALGORITMO_PLANIFICACION = config_get_string_value(kernel_config,"ALGORITMO_PLANIFICACION");
	QUANTUM = config_get_int_value(kernel_config, "QUANTUM");
	GRADO_MULTIPROGRAMACION = config_get_int_value(kernel_config, "GRADO_MULTIPROGRAMACION");
	RECURSOS = config_get_array_value(kernel_config, "RECURSOS");
	INSTANCIAS_RECURSOS = config_get_array_value(kernel_config, "INSTANCIAS_RECURSOS");

        if (!IP_MEMORIA ||!PUERTO_MEMORIA 
			||!IP_CPU ||!PUERTO_CPU_DISPATCH ||!PUERTO_CPU_INTERRUPT
			||!ALGORITMO_PLANIFICACION ||!QUANTUM ||!RECURSOS
			|| !INSTANCIAS_RECURSOS) {
		    log_error(kernel_logger,
				"No se ha leido correctamente alguno de los parametros del archivo de configuracion del kernel");
            log_destroy(kernel_logger);
            config_destroy(kernel_config);
            exit(EXIT_FAILURE);
        	}

}

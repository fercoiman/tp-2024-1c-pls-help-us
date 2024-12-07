#include "cpu_ciclo_instruccion.h"
#include "unistd.h"
bool continuar_ciclo_instruccion = true;
bool interrupcion_pendiente = false;
int pid_ejecutando;
char* pid_desalojo = NULL;

//Ciclo
//Fetch, Decode y Excute

void ciclo_instruccion(int socket_kernel){
	log_info(cpu_logger, "ESPERANDO CONTEXTO");
    t_contexto_ejec *contexto = recibir_contexto(socket_kernel);
	log_info(cpu_logger, "PROCESO EN CICLO DE INSTRUCCION: %d", contexto->pid);
	log_info(cpu_logger, "ANTES DE EJECUTAR LAS OPERACIONES AX: %d, BX: %d, CX: %d, DX: %d , EAX: %d, EBX: %d, ECX: %d, EDX:%d", contexto->registros_CPU->regAX,contexto->registros_CPU->regBX,contexto->registros_CPU->regCX,contexto->registros_CPU->regDX,contexto->registros_CPU->regEAX,contexto->registros_CPU->regEBX,contexto->registros_CPU->regECX,contexto->registros_CPU->regEDX);
	continuar_ciclo_instruccion = true;

	pid_ejecutando = contexto->pid;
	while (continuar_ciclo_instruccion) {
			log_info(cpu_logger, "while entre ");
		//FETCH

		log_info(cpu_logger, "Fetch Instrucción: “PID: %d - FETCH - Program Counter: %d“",contexto->pid, contexto->program_counter);
		
		contexto->instruccion = solicitar_instruccion_memoria(contexto->pid,contexto->program_counter);
		if(contexto->instruccion == NULL){
			log_error(cpu_logger,"Volviendo al kernel por error en la instruccion");
			continuar_ciclo_instruccion = false;
			break;
		}	

		
		
		if (contexto->instruccion->parametro1_tamanio == 0) {
			log_info(cpu_logger, "Instrucción Ejecutada: “PID: %d - Ejecutando: %s“", contexto->pid,contexto->instruccion->codigo_operacion);

		} else if (contexto->instruccion->parametro2_tamanio == 0) {
			log_info(cpu_logger, "Instrucción Ejecutada: “PID: %d - Ejecutando: %s - %s“",contexto->pid, contexto->instruccion->codigo_operacion,contexto->instruccion->parametros[0]);

		} else if (contexto->instruccion->parametro3_tamanio == 0) {
			log_info(cpu_logger, "Instrucción Ejecutada: “PID: %d - Ejecutando: %s - %s %s“",contexto->pid, contexto->instruccion->codigo_operacion,contexto->instruccion->parametros[0], contexto->instruccion->parametros[1]);

		} else {
			log_info(cpu_logger, "Instrucción Ejecutada: “PID: %d - Ejecutando: %s - %s %s %s“",contexto->pid, contexto->instruccion->codigo_operacion,contexto->instruccion->parametros[0], contexto->instruccion->parametros[1],contexto->instruccion->parametros[2]);
		}

		contexto->program_counter++;

		//DECODE y EXECUTE

		if (strcmp(contexto->instruccion->codigo_operacion, "SET") == 0) {
			log_trace(cpu_logger,"Entre a SET");
			operacion_set(&contexto, contexto->instruccion);
		}else if (strcmp(contexto->instruccion->codigo_operacion, "SUM") == 0) {
			log_trace(cpu_logger,"Entre a SUM");
			operacion_sum(&contexto, contexto->instruccion);
			

		}else if (strcmp(contexto->instruccion->codigo_operacion, "SUB") == 0) {
			log_trace(cpu_logger,"Entre a SUB");
			operacion_sub(&contexto, contexto->instruccion);
			log_info(cpu_logger,"el valor final es: %d",contexto->registros_CPU->regEDX);

		}else if (strcmp(contexto->instruccion->codigo_operacion, "JNZ") == 0) {
			log_trace(cpu_logger,"Entre a JNZ");

			operacion_jnz(&contexto, contexto->instruccion);

		}else if (strcmp(contexto->instruccion->codigo_operacion,"IO_GEN_SLEEP") == 0){
			log_trace(cpu_logger,"Entre a IO_GEN_SLEEP");
            operacion_io_gen_sleep(contexto,socket_kernel);
			continuar_ciclo_instruccion = false;

        }else if (strcmp(contexto->instruccion->codigo_operacion, "MOV_IN") == 0){
			//VER LO DE PAGEFAULT
			operacion_mov_in(&contexto,contexto->instruccion);
			
            
        }else if (strcmp(contexto->instruccion->codigo_operacion, "MOV_OUT") == 0){
			//VER LO DE PAGEFAULT
			operacion_mov_out(&contexto,contexto->instruccion);			
            
        }else if(strcmp(contexto->instruccion->codigo_operacion, "RESIZE") == 0){ 
			operacion_resize(&contexto,contexto->instruccion);
			

		} else if(strcmp(contexto->instruccion->codigo_operacion, "COPY_STRING") == 0){ 
			operacion_copy_string(&contexto,contexto->instruccion);

		}else if(strcmp(contexto->instruccion->codigo_operacion, "WAIT") == 0){
			log_trace(cpu_logger,"Entre a WAIT");
			operacion_wait(contexto,socket_kernel);
			continuar_ciclo_instruccion = false;

        }else if(strcmp(contexto->instruccion->codigo_operacion, "SIGNAL") == 0){
			log_trace(cpu_logger,"Entre a SIGNAL");
            operacion_signal(contexto,socket_kernel);
			continuar_ciclo_instruccion = false;
		// }else if(strcmp(instruccion->codigo_operacion, "IO_STDIN_READ") == 0){
			//operacion_io_stdin_read(contexto,socket_kernel);
		// }else if(strcmp(instruccion->codigo_operacion, "IO_STDOUT_WRITE") == 0){
			//operacion_io_stdout_write(contexto,socket_kernel);
		// }else if(strcmp(instruccion->codigo_operacion, "IO_FS_CREATE") == 0){
			//operacion_fs_create(contexto,socket_kernel);
		// }else if(strcmp(instruccion->codigo_operacion, "IO_FS_DELETE") == 0){
			//operacion_fs_delete(contexto,socket_kernel);
		// }else if(strcmp(instruccion->codigo_operacion, "IO_FS_TRUNCATE") == 0){
			//operacion_fs_truncate(contexto,socket_kernel);
		// }else if(strcmp(instruccion->codigo_operacion, "IO_FS_WRITE") == 0){
			//operacion_fs_write(contexto,socket_kernel);
		// }else if(strcmp(instruccion->codigo_operacion, "IO_FS_READ") == 0){
			//operacion_fs_read(contexto,socket_kernel);

        
        }else if (strcmp(contexto->instruccion->codigo_operacion, "EXIT") == 0) {
			
			log_info(cpu_logger, "los valores finales de los registros son AX: %d, BX: %d, CX: %d, DX: %d , EAX: %d, EBX: %d, ECX: %d, EDX:%d", contexto->registros_CPU->regAX,contexto->registros_CPU->regBX,contexto->registros_CPU->regCX,contexto->registros_CPU->regDX,contexto->registros_CPU->regEAX,contexto->registros_CPU->regEBX,contexto->registros_CPU->regECX,contexto->registros_CPU->regEDX);
            operacion_exit(contexto,socket_kernel);
            
			//t_contexto_destroyer(contexto);
			continuar_ciclo_instruccion = true;
			break;
		}
		//CHECK INTERRUPT
		if(interrupcion_pendiente && pid_desalojo != NULL ){ //&& string_equals_ignore_case(pid_desalojo, string_itoa(contexto->pid)
			log_info(cpu_logger,"Atendiendo interrupcionnn %s",pid_desalojo);
			continuar_ciclo_instruccion = false;
			pasar_a_kernel(contexto,INTERRUPCION,socket_kernel);
			interrupcion_pendiente = false;
			free(pid_desalojo);
			pid_desalojo = NULL;
		}
		t_instruccion_destroyer(contexto->instruccion);
	}
		if (contexto==NULL){
			enviar_mensaje("No hay proceso a desalojar",socket_kernel,MENSAJE);
		}
	
		return;
}

void recibir_interrupcion(int cliente_fd){
    char* mensaje = recibir_mensaje(cliente_fd);
    log_info(cpu_logger,"Interrupcion - Motivo: %s",mensaje);
    
    char** array_mensaje = string_split(mensaje,"");
    pid_desalojo = string_array_pop(array_mensaje);

    interrupcion_pendiente = true;

    // if(!continuar_ciclo_instruccion){
    //     interrupcion_pendiente = false;
    //     free(pid_desalojo);
    //     pid_desalojo = NULL;
	// 	log_info(cpu_logger,"ENTRE A EL IF");
    //     enviar_mensaje("No hay ningun proceso ejecutando",cliente_fd,INTERRUPCION);

    // }else
	//if(pid_ejecutando && !string_equals_ignore_case(string_itoa(pid_ejecutando),pid_desalojo)){
      //  interrupcion_pendiente = false;
       // free(pid_desalojo);
       // pid_desalojo = NULL;
		//log_info(cpu_logger,"ENTRE A EL ESE IF");
       // enviar_mensaje("El proceso fue delojado, esta ejecutando otro proceso distinto",socket_kernel_interrupt,INTERRUPCION);
   // }

    free(mensaje);
    string_array_destroy(array_mensaje);
}


t_instruccion* solicitar_instruccion_memoria(uint32_t pid, uint32_t program_counter){
	 t_paquete *paquete_program_counter = crear_paquete(INSTRUCCION);
	agregar_a_paquete(paquete_program_counter, &program_counter, sizeof(uint32_t));
	agregar_a_paquete(paquete_program_counter, &pid, sizeof(uint32_t));
	enviar_paquete(paquete_program_counter, socket_memoria);
	eliminar_paquete(paquete_program_counter);
	op_code opcode = recibir_operacion(socket_memoria);
	if (opcode != INSTRUCCION) {
		log_error(cpu_logger,"No se pudo recibir la instruccion de memoria! codigo de operacion recibido: %d",opcode);
		return NULL;
	}
	t_instruccion* instruccion_recibida = malloc(sizeof(t_instruccion));
	t_paquete *paquete_instruccion = recibir_paquete(socket_memoria);
	paquete_instruccion->buffer->offset = 0;
	int tamanio_instruccion;
	int tamanio_parametro1;
	int tamanio_parametro2;
	int tamanio_parametro3;
	buffer_read(&tamanio_instruccion, paquete_instruccion->buffer, sizeof(int));
	instruccion_recibida->codigo_operacion = malloc(tamanio_instruccion);
	buffer_read(instruccion_recibida->codigo_operacion, paquete_instruccion->buffer, sizeof(char) * tamanio_instruccion);
	buffer_read(&tamanio_parametro1, paquete_instruccion->buffer, sizeof(int));
	buffer_read(&tamanio_parametro2, paquete_instruccion->buffer, sizeof(int));
	buffer_read(&tamanio_parametro3, paquete_instruccion->buffer, sizeof(int));
	instruccion_recibida->parametros[0] = malloc(tamanio_parametro1);
	instruccion_recibida->parametros[1] = malloc(tamanio_parametro2);
	instruccion_recibida->parametros[2] = malloc(tamanio_parametro3);
	buffer_read(instruccion_recibida->parametros[0], paquete_instruccion->buffer, sizeof(char) * tamanio_parametro1);
	buffer_read(instruccion_recibida->parametros[1], paquete_instruccion->buffer, sizeof(char) * tamanio_parametro2);
	buffer_read(instruccion_recibida->parametros[2], paquete_instruccion->buffer, sizeof(char) * tamanio_parametro3);
	instruccion_recibida->parametro1_tamanio = tamanio_parametro1;
	instruccion_recibida->parametro2_tamanio = tamanio_parametro2;
	instruccion_recibida->parametro3_tamanio = tamanio_parametro3;
	eliminar_paquete(paquete_instruccion);
	return instruccion_recibida;
}
void t_instruccion_destroyer(void* instruccion) {
    if (instruccion == NULL) {
        return;
    }

    t_instruccion* instruccion_a_eliminar = (t_instruccion*) instruccion;

    free(instruccion_a_eliminar->codigo_operacion);

    for (int i = 0; i < sizeof(instruccion_a_eliminar->parametros) / sizeof(instruccion_a_eliminar->parametros[0]); i++) {
        if(instruccion_a_eliminar->parametros[i] != NULL){
			free(instruccion_a_eliminar->parametros[i]);
		}
		continue;
    }

    free(instruccion_a_eliminar);
}

void t_contexto_destroyer(void* contexto) {
    if (contexto == NULL) {
        return;
    }

    t_contexto_ejec* contexto_a_eliminar = (t_contexto_ejec*) contexto;
    log_info(cpu_logger, "ELIMINANDO CONTEXTO PID: %d", contexto_a_eliminar->pid);

    if (contexto_a_eliminar->instruccion != NULL) {
        t_instruccion_destroyer(contexto_a_eliminar->instruccion);
        log_info(cpu_logger, "ELIMINANDO INSTRUCCION");
    }

    free(contexto_a_eliminar->registros_CPU);
    log_info(cpu_logger, "ELIMINANDO REGISTROS");

    free(contexto_a_eliminar);
    log_trace(cpu_logger, "CONTEXTO ELIMINADO");
}
//IO_FS_CREATE

// void operacion_fs_create(t_contexto_ejec* un_contexto,int cliente_kernel){
//     
//     pasar_a_kernel(un_contexto,CREAR_ARCHIVO,cliente_kernel); 
// }

//IO_FS_DELETE

// void operacion_fs_delete(t_contexto_ejec* un_contexto,int cliente_kernel){
//     
//     pasar_a_kernel(un_contexto,ELIMINAR_ARCHIVO,cliente_kernel); 
// }

//IO_FS_TRUNCATE

// void operacion_fs_truncate(t_contexto_ejec* un_contexto,int cliente_kernel){
//     
//     pasar_a_kernel(un_contexto,TRUNCAR_ARCHIVO,cliente_kernel); 
// }
//IO_FS_WRITE
// void operacion_fs_write(t_contexto_ejec* un_contexto,int cliente_kernel){
//     bool pagefault = decodificar_direccion_logica(&un contexto,1);
//     if(!pagefault){
//         un_contexto->program_counter++;
//         un_contexto->instruccion->parametros[2]=string_itoa(tamanio_pagina);
//IO_FS_READ
// void operacion_fs_read(t_contexto_ejec* un_contexto,int cliente_kernel){
//     bool pagefault = decodificar_direccion_logica(&un contexto,1);
//     if(!pagefault){
//         un_contexto->program_counter++;
//         un_contexto->instruccion->parametros[2]=string_itoa(tamanio_pagina);
//     } 
// }
// }
//AVERIGUAR DONDE MANDAR ESTO
/*
int solicitar_marco(int numero_pagina, int pid){
	t_paquete* paquete = crear_paquete(ACCESO_A_PAGINA);
	agregar_a_paquete_sin_agregar_tamanio(paquete, &numero_pagina, sizeof(int));
	agregar_a_paquete_sin_agregar_tamanio(paquete, &pid, sizeof(int));
	enviar_paquete(paquete, socket_memoria);
	eliminar_paquete(paquete);
	int size;
	int marco_pagina;
	op_code cod_op = recibir_operacion(socket_memoria);
	if(cod_op == ACCESO_A_PAGINA){
		void* buffer = recibir_buffer(&size, socket_memoria);
		memcpy(&marco_pagina, buffer, sizeof(int));
		free(buffer);
		return marco_pagina;
	} else if(cod_op == PAGE_FAULT) { // sino significaria page fault
		char* mensaje = recibir_mensaje(socket_memoria);
		log_info(cpu_logger, "Se recibio: “%s“ de memoria", mensaje);
		free(mensaje);
		return -1;
	} else {
		log_error(cpu_logger, "Codigo de operacion inesperado de memoria: %d", cod_op);
		return -1;
	}
}
int traducir_direccion_logica(int direccion_logica, int pid, t_contexto_ejec* contexto){
	int numero_pagina = (int) floor(direccion_logica / tamanio_pagina);
	int desplazamiento = direccion_logica - numero_pagina * tamanio_pagina;
	int marco_pagina = solicitar_marco(numero_pagina,pid);
	if(marco_pagina == -1){
		log_info(cpu_logger, "Page Fault: “Page Fault PID: %d - Página: %d“", pid, numero_pagina);
		// AVISO A KERNEL PARA QUE MANEJE EL PAGE fAULT
		t_paquete *paquete_contexto = crear_paquete(PAGE_FAULT);
		agregar_a_paquete_sin_agregar_tamanio(paquete_contexto, &numero_pagina,sizeof(int));
		agregar_a_paquete_sin_agregar_tamanio(paquete_contexto, &(pid),sizeof(int));
		agregar_a_paquete_sin_agregar_tamanio(paquete_contexto,&(contexto->program_counter), sizeof(int));
		agregar_a_paquete_sin_agregar_tamanio(paquete_contexto,&(contexto->registros_CPU->regAX), sizeof(uint32_t));
		agregar_a_paquete_sin_agregar_tamanio(paquete_contexto,&(contexto->registros_CPU->regBX), sizeof(uint32_t));
		agregar_a_paquete_sin_agregar_tamanio(paquete_contexto,&(contexto->registros_CPU->regCX), sizeof(uint32_t));
		agregar_a_paquete_sin_agregar_tamanio(paquete_contexto,&(contexto->registros_CPU->regDX), sizeof(uint32_t));
		agregar_a_paquete(paquete_contexto, contexto->instruccion->codigo_operacion,contexto->instruccion->codigo_operacion_tamanio);
		agregar_a_paquete(paquete_contexto, contexto->instruccion->parametros[0],contexto->instruccion->parametro1_tamanio);
		agregar_a_paquete(paquete_contexto, contexto->instruccion->parametros[1],contexto->instruccion->parametro2_tamanio);
		agregar_a_paquete(paquete_contexto, contexto->instruccion->parametros[2],contexto->instruccion->parametro3_tamanio);
		enviar_paquete(paquete_contexto, cliente_kernel);
		eliminar_paquete(paquete_contexto);
		return -1; // page fault
	} else {
		log_info(cpu_logger, "Obtener Marco: “PID: %d - OBTENER MARCO - Página: %d - Marco: %d“", pid, numero_pagina, marco_pagina);
	}
	return desplazamiento + marco_pagina * tamanio_pagina;
}
bool decodificar_direccion_logica(t_contexto_ejec** contexto, int posicion_parametro_direccion_logica){
	int direccion_logica = atoi((*contexto)->instruccion->parametros[posicion_parametro_direccion_logica]);
	free((*contexto)->instruccion->parametros[posicion_parametro_direccion_logica]);
	if(posicion_parametro_direccion_logica == 0){
		(*contexto)->instruccion->parametro1_tamanio = 0;
	} else if(posicion_parametro_direccion_logica == 1){
		(*contexto)->instruccion->parametro2_tamanio = 0;
	}
	int direccion_fisica = traducir_direccion_logica(direccion_logica, (*contexto)->pid, *contexto);
	if(direccion_fisica == -1){
		return true;
	}
	char *direccion_fisica_string = string_itoa(direccion_fisica);
	int tam_direccion_fisica_string = strlen(direccion_fisica_string) + 1;
	(*contexto)->instruccion->parametros[posicion_parametro_direccion_logica] = malloc(sizeof(char) * tam_direccion_fisica_string);
	strcpy((*contexto)->instruccion->parametros[posicion_parametro_direccion_logica], direccion_fisica_string);
	if(posicion_parametro_direccion_logica == 0){
		(*contexto)->instruccion->parametro1_tamanio = tam_direccion_fisica_string;
	} else if(posicion_parametro_direccion_logica == 1){
		(*contexto)->instruccion->parametro2_tamanio = tam_direccion_fisica_string;
	}
	free(direccion_fisica_string);
	return false;
}
*/
/*

t_fcb* inicializar_fcb(t_config* config){
    if(config == NULL){
        log_error(IO_logger,"Archivo de FCB erroneo o inexistente");
        return NULL;
    }

    t_fcb* fcb = malloc(sizeof(t_fcb));

    fcb->nombre = strdup(config_get_string_value(config, "NOMBRE_ARCHIVO"));
	fcb->tamanio = config_get_int_value(config, "TAMANIO_ARCHIVO");
	fcb->bloque_inicial = config_get_int_value(config, "BLOQUE_INICIAL");

    if(fcb->nombre == NULL ){
		log_error(IO_logger, "Archivo de FCB erroneo o inexistente");
		return NULL;
	}

	return fcb;
}

t_fcb* crear_fcb( t_instruccion* instruccion, char* path){
	t_config* config = malloc(sizeof(t_config));

	config->path = strdup(path);
	config->properties = dictionary_create();

	config_set_value(config, "NOMBRE_ARCHIVO", string_duplicate(instruccion->parametros[0]));
	config_set_value(config, "TAMANIO_ARCHIVO", string_itoa(0));
	config_set_value(config, "BLOQUE_INICIAL", "");

	config_save_in_file(config, path);

	t_fcb* fcb = malloc(sizeof(t_fcb));

	fcb->nombre = string_duplicate(config_get_string_value(config, "NOMBRE_ARCHIVO"));
	fcb->tamanio = config_get_int_value(config, "TAMANIO_ARCHIVO");
	fcb->bloque_inicial = config_get_int_value(config, "BLOQUE_INICIAL");

	config_destroy(config);
	return fcb;
}

void crear_archivo(int cliente){
    t_instruccion* instruccion = recibir_instruccion(cliente_fd);

	char* nombre_archivo = strdup(instruccion->parametros[0]);

	log_info(logger, "Crear Archivo: “Crear Archivo: %s“", nombre);

    //Ruta
	char* direccion_fcb = string_new();
	string_append(&direccion_fcb, path_fcb);
	string_append(&direccion_fcb, "/");
	string_append(&direccion_fcb, nombre_archivo);

	t_fcb* fcb = malloc(sizeof(t_fcb));
	fcb = crear_fcb(instruccion, direccion_fcb);

	dictionary_put(fcb_por_archivo, nombre_archivo, fcb);
	enviar_mensaje("OK", cliente_fd, CREAR_ARCHIVO_RESPUESTA);
}


void eliminar_archivo(int cliente){
    t_instruccion* instruccion = recibir_instruccion(cliente);
    char* nombre = strdup(instruccion->parametros[0]);

    log_info(IO_logger, "Eliminar Archivo: “Eliminar Archivo: %s“", nombre);

    //Ruta
    char* direccion_fcb = string_new();
    string_append(&direccion_fcb, path_fcb);
    string_append(&direccion_fcb, "/");
    string_append(&direccion_fcb, nombre);

    // Verificar si el archivo existe en el diccionario
    if (!dictionary_has_key(fcb_por_archivo, nombre)) {
        log_error(IO_logger, "El archivo no existe: %s", nombre);
        free(nombre);
        free(direccion_fcb);
        return;
    }

    t_fcb* fcb = dictionary_remove(fcb_por_archivo, nombre);

    free(fcb->nombre);
    free(fcb);

    if (remove(direccion_fcb) != 0) {
        log_error(IO_logger, "No se pudo eliminar el archivo: %s", direccion_fcb);
        return;
    } else {
        log_info(IO_logger, "Archivo eliminado correctamente: %s", direccion_fcb);
        enviar_mensaje("OK", cliente, ELIMINAR_ARCHIVO_RESPUESTA);
    }

    free(nombre);
    free(direccion_fcb);
}

void truncar_archivo(int cliente){
    t_instruccion* instruccion = (t_instruccion*) recibir_instruccion(cliente_fd);

		char* nombre = strdup(instruccion->parametros[0]);
		int nuevo_tamanio = atoi(instruccion->parametros[1]);


		log_info(IO_logger, "Truncate de Archivo: “Truncar Archivo: %s - Tamaño: %d“", nombre, nuevo_tamanio);
		t_fcb* fcb_a_truncar = dictionary_get(fcb_por_archivo, nombre);

	    int n_bloques_nuevo = calcular_bloques(nuevo_tamanio);

	    int n_bloques_actual = calcular_bloques(fcb_a_truncar->tamanio);

		log_info(IO_logger, "Bloques actual %d vs Bloque nuevo %d", n_bloques_actual, n_bloques_nuevo);

	    if(n_bloques_nuevo > n_bloques_actual){
		   // Ampliar tamanio
	    	int bloques_a_agregar = n_bloques_nuevo - n_bloques_actual;
	    	agregar_bloques(fcb_a_truncar, bloques_a_agregar);


	    } else if(n_bloques_nuevo < n_bloques_actual) {
		   // Reducir tamanio
		  int bloques_a_sacar = n_bloques_actual - n_bloques_nuevo;
		  eliminar_bloques(fcb_a_truncar, bloques_a_sacar, n_bloques_actual);

	   }

	   fcb_a_truncar->tamanio = nuevo_tamanio;

	 	char* direccion_fcb = string_new();
		string_append(&direccion_fcb, path_fcb);
		string_append(&direccion_fcb, "/");
		string_append(&direccion_fcb, fcb_a_truncar->nombre);

	   	t_config* archivo_fcb = config_create(direccion_fcb);

		if(archivo_fcb == NULL){
			log_error(logger, "No se encontro el fcb del archivo %s", fcb_a_truncar->nombre);
			enviar_mensaje("ERROR", cliente, TRUNCAR_ARCHIVO_RESPUESTA);
			return;
		}

		config_set_value(archivo_fcb, "TAMANIO_ARCHIVO", string_itoa(fcb_a_truncar->tamanio));
		config_set_value(archivo_fcb, "BLOQUE_INICIAL", string_itoa(fcb_a_truncar->bloque_inicial));

		config_save(archivo_fcb);

		enviar_mensaje("OK", cliente, TRUNCAR_ARCHIVO_RESPUESTA);

		free(nombre);
}

int calcular_bloques(int tamanio_en_bytes){
	float numero_bloques_aux = tamanio_en_bytes/(float)tam_bloque;

	double parte_fraccional;
    double nuevo_numero_bloques;
	parte_fraccional = modf(numero_bloques_aux, &nuevo_numero_bloques);

	if( parte_fraccional != 0)
		nuevo_numero_bloques = nuevo_numero_bloques + 1 ;
    }
    
	return (int)nuevo_numero_bloques;
}

void agregar_bloques(t_fcb* fcb, int bloques_a_agregar){
    uint32_t posicion_fat = fcb->bloque_inicial;

    //Inicializar con el primer bloque si el archivo no tiene bloques.
    if (posicion_fat == UINT32_MAX) {
        posicion_fat = primer_bloque_fat;
        while (bits_fat[posicion_fat] != 0) {
            esperar_por_fs(retardo_acceso_fat);
            posicion_fat++;
        }
        fcb->bloque_inicial = posicion_fat;
        bits_fat[posicion_fat] = UINT32_MAX;
        bloques_a_agregar--;
    }

    //Ultimo bloque asignado
    uint32_t bloque_actual = fcb->bloque_inicial;
    while (bits_fat[bloque_actual] != UINT32_MAX) {
        esperar_por_fs(retardo_acceso_fat);
        bloque_actual = bits_fat[bloque_actual];
    }

    uint32_t bloque_anterior = bloque_actual;

    //Asigna nuevos bloques
    while (bloques_a_agregar > 0) {
        uint32_t bloque_libre = primer_bloque_fat;

        // Busca el siguiente bloque libre
        while (bits_fat[bloque_libre] != 0) {
            esperar_fs(retardo_acceso_fat);
            bloque_libre++;
        }

        bits_fat[bloque_anterior] = bloque_libre;
        bits_fat[bloque_libre] = UINT32_MAX;
        bloque_anterior = bloque_libre;
        bloques_a_agregar--;
    }

    fcb->tamanio += bloques_a_agregar * TAMANIO_BLOQUE;
}

void eliminar_bloques(t_fcb* fcb, int bloques_a_eliminar) {
    uint32_t posicion_fat = fcb->bloque_inicial;

    if (posicion_fat == UINT32_MAX) {
        log_error(IO_logger, "El archivo no tiene bloques asignados.");
        return;
    }

    //Bloque después de la eliminación
    int bloques_totales = calcular_bloques(fcb->tamanio);
    int bloques_a_conservar = bloques_totales - bloques_a_eliminar;
    if (bloques_a_conservar < 0) bloques_a_conservar = 0;

    uint32_t bloque_actual = posicion_fat;
    uint32_t bloque_anterior = UINT32_MAX;

    //Bloques a conservar
    for (int i = 0; i < bloques_a_conservar; i++) {
        bloque_anterior = bloque_actual;
        bloque_actual = bits_fat[bloque_actual];
        esperar_fs(retardo_acceso_fat);
    }

    //Si se eliminan todos los bloques
    if (bloques_a_conservar == 0) {
        fcb->bloque_inicial = UINT32_MAX;
    } else {
        bits_fat[bloque_anterior] = UINT32_MAX;
    }

    //Liberar
    while (bloque_actual != UINT32_MAX) {
        uint32_t siguiente_bloque = bits_fat[bloque_actual];
        bits_fat[bloque_actual] = 0; //Bloque libre
        esperar_por_fs(retardo_acceso_fat);
        bloque_actual = siguiente_bloque;
    }

    fcb->tamanio -= bloques_a_eliminar * TAMANIO_BLOQUE;
}

void leer_archivo(int cliente){
    int size;
	void * buffer = recibir_buffer(&size ,cliente_fd);
	int p, pid, desplazamiento = 0 ;

	t_instruccion* instruccion ;
	memcpy(&pid, buffer+desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);
	instruccion = deserializar_instruccion_en(buffer, &desplazamiento);
	memcpy(&p, buffer+desplazamiento, sizeof(int));

	char* nombre = strdup(instruccion->parametros[0]);

	log_info(IO_logger, "Lectura de Archivo: “Leer Archivo: %s - Puntero: %d - Memoria: %s”", nombre, p, instruccion->parametros[1]);

	t_fcb *fcb = dictionary_get(fcb_por_archivo, nombre);

	if(dictionary_has_key(fcb_por_archivo, nombre)){

		int bloque_a_leer = p/tam_bloque;
		int i = 0;
		int aux_busqueda_fat = fcb->bloque_inicial;

		while(i < bloque_a_leer){
			log_info(logger, "Acceso a FAT: “Acceso a FAT - Entrada: %d - Valor: %d“", aux_busqueda_fat, bits_fat[aux_busqueda_fat]);
			esperar_fs(retardo_acceso_fat);
			aux_busqueda_fat = bits_fat[aux_busqueda_fat];
			i++;
		}

		log_info(logger, "Acceso a Bloque Archivo: “Acceso a bloque - Archivo: %s - Bloque archivo: %d - Bloque FS: %d“", nombre, bloque_a_leer, aux_busqueda_fat);
		esperar_fs(retardo_acceso_bloque);
		void *contenido_leido = array_bloques[aux_busqueda_fat];

		char* aux_direccion_fisica = strdup(instruccion->parametros[0]);
		instruccion->parametros[0] = strdup(instruccion->parametros[1]);
		instruccion->parametros[1] = aux_direccion_fisica;

		int aux_len_direccion_fisica = instruccion->parametro1_tamanio;
		instruccion->parametro1_tamanio = instruccion->parametro2_tamanio;
		instruccion->parametro2_tamanio = aux_len_direccion_fisica;

		int resultado_memoria = conectar_memoria(ip_memoria, puerto_memoria);

		if (resultado_memoria == -1) {
			log_error(logger, "IO no se conecto a MEMORIA");
			return;
		}

		t_paquete *paquete = crear_paquete(WRITE_MEMORY_FS);
		agregar_a_paquete_sin_agregar_tamanio(paquete, &pid, sizeof(int));
		agregar_a_paquete(paquete, instruccion->codigo_operacion,instruccion->codigo_operacion_tamanio);
		agregar_a_paquete(paquete, instruccion->parametros[0],instruccion->parametro1_tamanio);
		agregar_a_paquete(paquete, instruccion->parametros[1],instruccion->parametro2_tamanio);
		agregar_a_paquete(paquete, instruccion->parametros[2],instruccion->parametro3_tamanio);
		agregar_a_paquete_sin_agregar_tamanio(paquete, contenido_leido, tam_bloque);
		enviar_paquete(paquete, socket_memoria);

		eliminar_paquete(paquete);

		log_info(logger, "Archivo leido: %s - Puntero: %d - Memoria: %s", nombre, p, instruccion->parametros[1]);

		op_code codigo = recibir_operacion(socket_memoria);

		if(codigo != WRITE_MEMORY_FS_RESPUESTA){
			log_error(IO_logger, "No se pudo escribir el contenido del archivo en la memoria");
			return;
		}

		char* mensaje = recibir_mensaje(socket_memoria);

		if(strcmp(mensaje,"OK") == 0){
			log_info(IO_logger, "se recibio %s de memoria", mensaje);
			enviar_mensaje(mensaje, cliente_fd, LEER_ARCHIVO_RESPUESTA);
			free(mensaje);
		}

	}else{
		log_error(IO_logger, "El archivo no se encuentra en el FS");
        return;
	}


	free(nombre);
	free(buffer);
	instruccion_destroy(instruccion);
	close(socket_memoria);
}

void escribir_archivo(int cliente){
    int size;
	void * buffer = recibir_buffer(&size ,cliente_fd);
	int pid, p, desplazamiento  = 0;
	t_instruccion* instruccion ;

	memcpy(&pid, buffer+desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);
	instruccion = deserializar_instruccion_en(buffer, &desplazamiento);
	memcpy(&p, buffer+desplazamiento, sizeof(int));

	char* nombre = strdup(instruccion->parametros[0]);

	log_info(IO_logger, "Escritura de Archivo: “Escribir Archivo: %s - Puntero: %d - Memoria: %s”",nombre, p, instruccion->parametros[1]);

	t_fcb *fcb = dictionary_get(fcb_por_archivo, nombre);

	int resultado_memoria = conectar_memoria(ip_memoria, puerto_memoria);

	if (resultado_memoria == -1) {
		log_error(IO_logger, "IO no se conecto con MEMORIA.");
		return;
	}

	t_paquete *paquete = crear_paquete(READ_MEMORY);
	agregar_a_paquete_sin_agregar_tamanio(paquete, &pid, sizeof(int));
	agregar_a_paquete(paquete, instruccion->codigo_operacion,instruccion->codigo_operacion_tamanio);
	agregar_a_paquete(paquete, instruccion->parametros[0],instruccion->parametro1_tamanio);
	agregar_a_paquete(paquete, instruccion->parametros[1],instruccion->parametro2_tamanio);
	agregar_a_paquete(paquete, instruccion->parametros[2],instruccion->parametro3_tamanio);
	enviar_paquete(paquete, socket_memoria);

	op_code codigo = recibir_operacion(socket_memoria);

	if(codigo != READ_MEMORY_RESPUESTA){
		log_error(IO_logger, "No se pudo leer el contenido del archivo en la memoria");
		return;
	}

	int size_recibido;
	void* buffer_recibido = recibir_buffer(&size_recibido, socket_memoria);

	void * contenido_a_escribir = malloc(tam_bloque);
	memcpy(contenido_a_escribir, buffer_recibido, tam_bloque);

	if(dictionary_has_key(fcb_por_archivo, nombre)){

		int bloque_a_escribir = p/tam_bloque;
		int i = 0;
		int aux_busqueda_fat = fcb->bloque_inicial;

		while(i < bloque_a_escribir){
			esperar_fs(retardo_acceso_fat);
			log_info(IO_logger, "Acceso a FAT: “Acceso a FAT - Entrada: %d - Valor: %d“", aux_busqueda_fat, bits_fat[aux_busqueda_fat]);
			aux_busqueda_fat = bits_fat[aux_busqueda_fat];
			i++;
		}
		log_info(IO_logger, "Acceso a Bloque Archivo: “Acceso a bloque - Archivo: %s - Bloque archivo: %d - Bloque FS: %d“", nombre_archivo, bloque_a_escribir, aux_busqueda_fat);

		esperar_fs(retardo_acceso_bloque);
		memcpy(array_bloques[aux_busqueda_fat], contenido_a_escribir, tam_bloque);

		log_info(IO_logger, "Archivo escrito: %s - Puntero: %d - Memoria: %s", nombre_archivo, p, instruccion->parametros[1]);

		enviar_mensaje("OK", cliente, ESCRIBIR_ARCHIVO_RESPUESTA);

	}else{
		log_error(IO_logger, "El archivo no se encuentra en el FS");
        return;
	}


	free(nombre);
	instruccion_destroy(instruccion);
	close(socket_memoria);
	free(buffer);
}

void esperar_fs(int milisegundos_a_esperar){
	// el * 1000 es para pasarlo a microsegundos
	usleep(milisegundos_a_esperar*1000);
}

*/
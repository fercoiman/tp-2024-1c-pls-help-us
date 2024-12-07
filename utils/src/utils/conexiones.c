#include "conexiones.h"
#include "estructuras.h"


//UTILS PARA CLIENTE
void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	// Ahora vamos a crear el socket.
	int socket_cliente = socket(server_info->ai_family,
                    server_info->ai_socktype,
                    server_info->ai_protocol);

	// Ahora que tenemos el socket, vamos a conectarlo
	connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);

	freeaddrinfo(server_info);

	return socket_cliente;
}

void enviar_mensaje(char* mensaje, int socket_cliente, op_code codigo)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo;

	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) +1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);
	free(a_enviar);
	eliminar_paquete(paquete);
}


t_buffer* buffer_create(uint32_t size) {
    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size = size;
    buffer->offset = 0;
    buffer->stream = malloc(size);
    return buffer;
}


// t_paquete* crear_paquete(op_code codigo) {
//     t_paquete* paquete = malloc(sizeof(t_paquete));
//     paquete->codigo_operacion = codigo;
//     paquete->buffer = buffer_create(0); // Inicialmente vacío
//     return paquete;
// }
void buffer_destroy(t_buffer* buffer) {
    free(buffer->stream);
    free(buffer);
}
void buffer_add(t_buffer* buffer, void* data, uint32_t size) {
    memcpy(buffer->stream + buffer->offset, data, size);
    buffer->offset += size;
}
void recibir_program_counter(int socket_cliente, uint32_t *pid,uint32_t* program_counter ){
	int size;
	int desplazamiento = 0;
	void * buffer;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(pid, buffer+desplazamiento, sizeof(uint32_t));
		desplazamiento+=sizeof(uint32_t);
		memcpy(program_counter, buffer + desplazamiento, sizeof(uint32_t));
		desplazamiento+=sizeof(uint32_t);
	}
	free(buffer);

}
void recibir_handshake(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	if(strcmp(buffer, "OK") == 0)
		enviar_mensaje("OK", socket_cliente, HANDSHAKE);
	else
		enviar_mensaje("ERROR", socket_cliente, HANDSHAKE);
	free(buffer);
}
//para deserializacion
// void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
// {
// 	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

// 	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
// 	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

// 	paquete->buffer->size += tamanio + sizeof(int);
// }

//aniade solo datos al buffer (da igual el tamanio)

// void enviar_paquete(t_paquete* paquete, int socket_cliente)
// {
// 	int bytes = paquete->buffer->size + 2*sizeof(int);
// 	void* a_enviar = serializar_paquete(paquete, bytes);
// 	send(socket_cliente, a_enviar, bytes, 0);
// 	free(a_enviar);
// }


void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}
void paquete(int conexion)
{
	char* leido;
	t_paquete* paquete = crear_paquete(PAQUETE);
	leido = readline("> ");
	while(strcmp(leido, "")!=0)
	{
		agregar_a_paquete(paquete, leido, strlen(leido)+1);
		free(leido);
		leido = readline("> ");
	}
	free(leido);
	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}



void terminar_programa(t_log* logger, t_config* config)
{

	if(logger != NULL){  
		log_destroy(logger);
	}
	if(config != NULL){
		config_destroy(config);
	}

}

//UTILS PARA EL SERVIDOR
int iniciar_servidor(char* PUERTO, t_log* logger,char* mensaje)
{
	
	int socket_servidor;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	getaddrinfo(NULL, PUERTO, &hints, &servinfo);

	socket_servidor = socket(servinfo->ai_family,
                         servinfo->ai_socktype,
                         servinfo->ai_protocol);
	
	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);
	
	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);
	return socket_servidor;
}

int esperar_cliente(int socket_servidor, t_log* logger,char* mensaje)
{
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	return socket_cliente;
}
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio);
    memcpy(paquete->buffer->stream + paquete->buffer->size, valor, tamanio);
    paquete->buffer->size += tamanio;
}

int enviar_paquete(t_paquete* paquete, int socket_destino) {
    int bytes = paquete->buffer->size + sizeof(op_code) + sizeof(uint32_t);
    void* a_enviar = malloc(bytes);
    int offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(op_code));
    offset += sizeof(op_code);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

    int result = send(socket_destino, a_enviar, bytes, 0);
    if (result < 0) {
        printf("Error al enviar el paquete\n");
    }else{
		printf("Paquete enviado \n");
	}
    free(a_enviar);
    return result;
}

t_paquete* crear_paquete(op_code codigo_operacion) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = codigo_operacion;
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = 0;
    paquete->buffer->stream = NULL;
    return paquete;
}
t_paquete* recibir_paquete(int socket) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    // Recibir el tamaño del buffer
    recv(socket, &paquete->buffer->size, sizeof(uint32_t), 0);
    // Recibir el buffer
    paquete->buffer->stream = malloc(paquete->buffer->size);
    recv(socket, paquete->buffer->stream, paquete->buffer->size, 0);
    return paquete;
}
void eliminar_paquete(t_paquete* paquete) {
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}


void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}
void buffer_read(void* dest, t_buffer* buffer, uint32_t size) {
    memcpy(dest, buffer->stream + buffer->offset, size);
    buffer->offset += size;
}
int recibir_operacion(int cliente_fd) {
    op_code codigo_operacion;
    int bytes_recibidos = recv(cliente_fd, &codigo_operacion, sizeof(op_code), 0);
    if (bytes_recibidos <= 0) {
        return -1;
    }
    return codigo_operacion;
}
char* recibir_mensaje(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	int length_buffer = strlen(buffer);
	buffer = realloc(buffer, length_buffer + 2);
	buffer[length_buffer +1] = '\0';

	return buffer;
}

// t_paquete* recibir_paquete(int socket) {
//     t_paquete* paquete = malloc(sizeof(t_paquete));
//     paquete->buffer = malloc(sizeof(t_buffer));

//     // Recibir el código de operación
//     recv(socket, &paquete->codigo_operacion, sizeof(op_code), 0);

//     // Recibir el tamaño del buffer
//     recv(socket, &paquete->buffer->size, sizeof(uint32_t), 0);

//     // Recibir el buffer
//     paquete->buffer->stream = malloc(paquete->buffer->size);
//     recv(socket, paquete->buffer->stream, paquete->buffer->size, 0);

//     return paquete;
// }
// void buffer_read(void* dest, t_buffer* buffer, uint32_t size) {
//     memcpy(dest, buffer->stream + buffer->offset, size);
//     buffer->offset += size;
// }
// void eliminar_paquete(t_paquete* paquete) {
//     if (paquete) {
//         if (paquete->buffer) {
//             free(paquete->buffer->stream);
//             free(paquete->buffer);
//         }
//         free(paquete);
//     }
// }

t_instruccion *deserializar_instruccion_en(void *buffer, int* desplazamiento){
	t_instruccion * instruccion = malloc(sizeof(t_instruccion));

	memcpy(&(instruccion->codigo_operacion_tamanio), buffer + *desplazamiento, sizeof(int));
	*desplazamiento+=sizeof(int);
	instruccion->codigo_operacion = malloc( instruccion->codigo_operacion_tamanio);
	memcpy( instruccion->codigo_operacion, buffer+*desplazamiento,  instruccion->codigo_operacion_tamanio);
	*desplazamiento+= instruccion->codigo_operacion_tamanio;

	memcpy(&( instruccion->parametro1_tamanio), buffer+*desplazamiento, sizeof(int));
	*desplazamiento+=sizeof(int);
	 instruccion->parametros[0] = malloc( instruccion->parametro1_tamanio);
	memcpy( instruccion->parametros[0], buffer + *desplazamiento,  instruccion->parametro1_tamanio);
	*desplazamiento +=  instruccion->parametro1_tamanio;

	memcpy(&( instruccion->parametro2_tamanio), buffer+*desplazamiento, sizeof(int));
	*desplazamiento+=sizeof(int);
	 instruccion->parametros[1] = malloc( instruccion->parametro2_tamanio);
	memcpy( instruccion->parametros[1], buffer + *desplazamiento,  instruccion->parametro2_tamanio);
	*desplazamiento +=  instruccion->parametro2_tamanio;


	memcpy(&( instruccion->parametro3_tamanio), buffer+*desplazamiento, sizeof(int));
	*desplazamiento+=sizeof(int);
	 instruccion->parametros[2] = malloc( instruccion->parametro3_tamanio);
	memcpy( instruccion->parametros[2], buffer + *desplazamiento,  instruccion->parametro3_tamanio);
	*desplazamiento +=  instruccion->parametro3_tamanio;

	return instruccion;
}
t_contexto_ejec* recibir_contexto(int socket_cliente) {
    t_paquete* paquete = recibir_paquete(socket_cliente);
    t_contexto_ejec* contexto = malloc(sizeof(t_contexto_ejec));
    
    if (paquete == NULL) {
        perror("Error al recibir el paquete");
        return NULL;
    }
    
    paquete->buffer->offset = 0;
    contexto->registros_CPU = malloc(sizeof(t_registros_cpu));
    contexto->instruccion = malloc(sizeof(t_instruccion));
    buffer_read(&(contexto->pid), paquete->buffer, sizeof(uint32_t));
    buffer_read(&(contexto->program_counter), paquete->buffer, sizeof(uint32_t));
    buffer_read(&(contexto->registros_CPU->regAX), paquete->buffer, sizeof(uint8_t));
    buffer_read(&(contexto->registros_CPU->regBX), paquete->buffer, sizeof(uint8_t));
    buffer_read(&(contexto->registros_CPU->regCX), paquete->buffer, sizeof(uint8_t));
    buffer_read(&(contexto->registros_CPU->regDX), paquete->buffer, sizeof(uint8_t));
    buffer_read(&(contexto->registros_CPU->regEAX), paquete->buffer, sizeof(uint32_t));
    buffer_read(&(contexto->registros_CPU->regEBX), paquete->buffer, sizeof(uint32_t));
    buffer_read(&(contexto->registros_CPU->regECX), paquete->buffer, sizeof(uint32_t));
    buffer_read(&(contexto->registros_CPU->regEDX), paquete->buffer, sizeof(uint32_t));
    buffer_read(&(contexto->registros_CPU->regSI), paquete->buffer, sizeof(uint32_t));
    buffer_read(&(contexto->registros_CPU->regDI), paquete->buffer, sizeof(uint32_t));
    int tamanio;
    int tamanio_param_1;
    int tamanio_param_2;
    int tamanio_param_3;
    
    buffer_read(&tamanio, paquete->buffer, sizeof(int));
    char* codigo_operacion = malloc(sizeof(char) * tamanio);
    buffer_read(codigo_operacion, paquete->buffer, sizeof(char) * tamanio);
    buffer_read(&tamanio_param_1, paquete->buffer, sizeof(int));
    char* param_1 = malloc(sizeof(char) * tamanio_param_1);
    buffer_read(param_1, paquete->buffer, sizeof(char) * tamanio_param_1);
    buffer_read(&tamanio_param_2, paquete->buffer, sizeof(int));
    char* param_2 = malloc(sizeof(char) * tamanio_param_2);
    buffer_read(param_2, paquete->buffer, sizeof(char) * tamanio_param_2);
    buffer_read(&tamanio_param_3, paquete->buffer, sizeof(int));
    char* param_3 = malloc(sizeof(char) * tamanio_param_3);
    buffer_read(param_3, paquete->buffer, sizeof(char) * tamanio_param_3);

    contexto->instruccion->codigo_operacion = codigo_operacion;
    contexto->instruccion->codigo_operacion_tamanio = tamanio;
    contexto->instruccion->parametro1_tamanio = tamanio_param_1;
    contexto->instruccion->parametro2_tamanio = tamanio_param_2;
    contexto->instruccion->parametro3_tamanio = tamanio_param_3;
    
    if (tamanio_param_1 != 0) {
        contexto->instruccion->parametros[0] = param_1;
    } else {
        free(param_1);
        contexto->instruccion->parametros[0] = NULL;
    }
    
    if (tamanio_param_2 != 0) {
        contexto->instruccion->parametros[1] = param_2;
    } else {
        free(param_2);
        contexto->instruccion->parametros[1] = NULL;
    }
    
    if (tamanio_param_3 != 0) {
        contexto->instruccion->parametros[2] = param_3;
    } else {
        free(param_3);
        contexto->instruccion->parametros[2] = NULL;
    }

    eliminar_paquete(paquete);
    return contexto;
}
void recibir_path_y_pid(int socket_cliente, char **path, int *pid)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		*path = malloc(tamanio);

		memcpy(*path, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;

		memcpy(pid, buffer+desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
	}
	free(buffer);
}

void liberar_instruccion(t_instruccion* instruccion){
	free(instruccion->codigo_operacion);
	if(instruccion->parametro1_tamanio !=0 && instruccion->parametro2_tamanio!=0 && instruccion->parametro3_tamanio!=0){
		for(int i=0;i<3; i++){
			free(instruccion->parametros[i]);
		}
	}else if(instruccion->parametro1_tamanio !=0 && instruccion->parametro2_tamanio!=0){
		for(int i=0;i<2; i++){
			free(instruccion->parametros[i]);
		}
	}else if(instruccion->parametro1_tamanio !=0){
		free(instruccion->parametros[0]);
	}
	free(instruccion);
}

void liberar_contexto(t_contexto_ejec* contexto){
	if(contexto->instruccion != NULL){
		liberar_instruccion(contexto->instruccion);
	}
	free(contexto);
}

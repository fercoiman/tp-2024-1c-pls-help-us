#include "kernel_conexiones.h"
int socket_cpu_dispatch;
int socket_cpu_interrupt;
int socket_memoria;
int servidor_kernel;
int socket_io;
//t_dictionary *tabla_global_de_archivos_abiertos;ad


int iniciar_kernel_servidor(){
	 	servidor_kernel = iniciar_servidor(PUERTO_ESCUCHA,kernel_logger,"KERNEL");
    	if (servidor_kernel == -1) {
		log_info(kernel_logger,"No fue posible poner crear socket servidor kernel");
       exit (EXIT_FAILURE);
   		}
		return servidor_kernel;
}
void* esperar_io(void* args){

    while (1) {
		int cliente_fd;
		log_info(kernel_logger,"Listo para recibir IO's");
		cliente_fd = esperar_cliente(servidor_kernel,kernel_logger,"IO");
		conexion_interfaz* conexion = malloc(sizeof(conexion_interfaz));
		conexion->socket = cliente_fd;
		log_info(kernel_logger,"el socket es %d",cliente_fd);
		log_info(kernel_logger,"esperando opcode");
		int cod_op = recibir_operacion(cliente_fd);
		log_info(kernel_logger,"El codigo de operacion recibidou es %d",cod_op);
		switch (cod_op)
		{
		case HANDSHAKE:
			recibir_handshake(cliente_fd);
			log_info(kernel_logger,"Handshake recibido");
			int codigo = recibir_operacion(cliente_fd);
			if (codigo != NUEVA_INTERFAZ){
				log_error(kernel_logger,"Operacion no reconocida");
				close(cliente_fd);
				continue;
			}
			log_info(kernel_logger,"Nueva interfaz");
			char* nombre = registrar_interfaz(cliente_fd,conexion);
			if (nombre != NULL){
			log_info(kernel_logger,"Interfaz %s registrada con exito",nombre);
			cliente_fd = 0;
			}else {
			log_error(kernel_logger,"No se pudo registrar la interfaz");
			close(cliente_fd);
			continue;
			}
			break;
			log_error(kernel_logger,"Operacion no reconocida");
			break;
		}
    }
}	
int conectar_cpu_dispatch() {
	socket_cpu_dispatch = crear_conexion(IP_CPU, PUERTO_CPU_DISPATCH);
	enviar_mensaje("OK", socket_cpu_dispatch,HANDSHAKE);
	op_code cod_op = recibir_operacion(socket_cpu_dispatch);
	if (cod_op != HANDSHAKE) {
		return -1;
	}
	int size;
	char* buffer = recibir_buffer(&size, socket_cpu_dispatch);
	free(buffer);
	log_info(kernel_logger,"Exito Conexion distpach");
	return socket_cpu_dispatch;
} 
int conectar_memoria() {
	socket_memoria = crear_conexion(IP_MEMORIA,PUERTO_MEMORIA);
	enviar_mensaje("OK", socket_memoria,HANDSHAKE);
	log_info(kernel_logger,"mensake enviado esperando hand");
	op_code cod_op = recibir_operacion(socket_memoria);
	if (cod_op != HANDSHAKE) {
		return -1;
	}
	int size;
	char* buffer = recibir_buffer(&size, socket_memoria);
	free(buffer);
	log_info(kernel_logger,"Exito conexion memoria");
	return socket_memoria;
}
int conectar_cpu_interrupt() {
	socket_cpu_interrupt = crear_conexion(IP_CPU, PUERTO_CPU_INTERRUPT);
	enviar_mensaje("OK", socket_cpu_interrupt,HANDSHAKE);
	op_code cod_op = recibir_operacion(socket_cpu_interrupt);

	if(cod_op != HANDSHAKE){
		return -1;
	}
	int size;
	char* buffer = recibir_buffer(&size, socket_cpu_interrupt);
	free(buffer);
	log_info(kernel_logger,"Exito conexion interrupt");
	return socket_cpu_interrupt;
}

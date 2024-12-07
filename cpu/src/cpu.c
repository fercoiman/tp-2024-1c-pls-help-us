#include "cpu.h"
#include <commons/string.h>
#include <string.h>

int servidor_cpu_dispatch;
int servidor_cpu_interrupt;
int socket_kernel_distpatch;
int socket_kernel_interrupt;
int socket_kernel;
int socket_memoria;

int main(int argc, char* argv[]) {

   inicializa_cpu();
   socket_memoria = conectar_cpu_memoria();
   log_info(cpu_logger, "Conectado a memoria en el socket %d", socket_memoria);
   log_info(cpu_logger, "Conectado a memoria");
   servidor_cpu_dispatch = iniciar_servidor(PUERTO_CPU_DISPATCH, cpu_logger,"CPU_DISPATCH");
   servidor_cpu_interrupt = iniciar_servidor(PUERTO_CPU_INTERRUPT, cpu_logger,"CPU_INTERRUPT");
   log_info(cpu_logger, "Esperando conexiones de kernel");
   socket_kernel_interrupt = esperar_cliente(servidor_cpu_interrupt,cpu_logger,"kernel conectado a CPU interrupt");
   pthread_t hilo_interrupt;
   int* socket_ptr = malloc(sizeof(int));
	if (socket_ptr == NULL) {
    perror("Error al asignar memoria");
    exit(EXIT_FAILURE);
	}
   *socket_ptr = socket_kernel_interrupt;

   pthread_create(&hilo_interrupt,NULL,atender_interrupciones, socket_ptr);
   pthread_detach(hilo_interrupt);
   
   atender_peticiones_instrucciones();
   //Terminar el programa
   log_error(cpu_logger, "Terminando programa");
   terminar_programa(cpu_logger,cpu_config);
   
   log_error(cpu_logger, "Programa terminado");
   return 0;
}
void* atender_interrupciones(void* args){
	int cliente_fd = *((int*) args);
	free(args);
	while (1) {
		int cod_op = recibir_operacion(cliente_fd);
		switch (cod_op) {
		case HANDSHAKE:
			recibir_handshake(cliente_fd);
			break;
		case INTERRUPCION:
			recibir_interrupcion(cliente_fd);
			break;
		case -1:
			log_error(cpu_logger, "El cliente se desconecto. Terminando servidor");
            close(servidor_cpu_dispatch);
            close(servidor_cpu_interrupt);
			return NULL;
		default:
			log_warning(cpu_logger,"Operacion desconocida.");
			break;
		}
	}

	return NULL;
}


void atender_peticiones_instrucciones() {

	int cliente_fd = esperar_cliente(servidor_cpu_dispatch,cpu_logger,"kernel conectado a CPU dispatch");
	log_info(cpu_logger, "Todas las conexiones fueron aceptadas");
	while (1) {
        int cod_op = recibir_operacion(cliente_fd);
        switch (cod_op) {
			case HANDSHAKE:
                recibir_handshake(cliente_fd);
                break;
            case PETICION_CPU:
                log_info(cpu_logger, "opcode en el case %d", cod_op);
                ciclo_instruccion(cliente_fd);
                break;
            case MENSAJE:
                //
                break;
            case PAQUETE:
                log_info(cpu_logger, "Me llegaron los siguientes valores:\n");
                break;
            case -1:
                log_error(cpu_logger, "el cliente se desconecto. Terminando servidor");
                return;
            default:
                log_warning(cpu_logger,"Operacion desconocida.");
                break;
        }
    }
}

void iterator(char* value) {
    log_info(cpu_logger,"%s", value);
}

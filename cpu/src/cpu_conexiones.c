#include "cpu_conexiones.h"

int conectar_cpu_memoria() {
	conexion_memoria = crear_conexion(IP_MEMORIA,PUERTO_MEMORIA);
	//log_info(cpu_logger, "Esperando...");
	enviar_mensaje("OK", conexion_memoria,HANDSHAKE);
	op_code cod_op = recibir_operacion(conexion_memoria);
	if (cod_op != HANDSHAKE) {
		return -1;
	}
	int size;
	char* buffer = recibir_buffer(&size, conexion_memoria);
	free(buffer);
	return conexion_memoria;
}


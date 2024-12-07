#include "estructuras.h"
#include <stddef.h>

void parametros_lenght(t_instruccion *ptr_inst) {

	ptr_inst->codigo_operacion_tamanio= strlen(ptr_inst->codigo_operacion) + 1;

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
}

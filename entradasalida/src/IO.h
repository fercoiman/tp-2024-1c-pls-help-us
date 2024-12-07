#ifndef IO_H_
#define IO_H_
#include <commons/log.h>
#include <commons/config.h>
#include <utils/conexiones.h>
#include <utils/estructuras.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <pthread.h>
#include <unistd.h>
#include <commons/string.h>

extern int TIEMPO_UNIDAD_TRABAJO;

void iniciar_interfaz(char* ,char*);
void mostrar_interfaces();
extern t_log* IO_logger;
int conectar_io_kernel();
int conectar_io_memoria();
void inicializa_IO();
void conectar_ios();
//hacer una funcion para cada tipo de interfaz
void usar_interfaz_generica();
void usar_interfaz_stdin();
void usar_interfaz_stdout();
void usar_interfaz_dialfs();
void destruir_comando(t_comando*);
t_comando* armar_comando(char*);
void conectar_interfaz_kernel(char*,char*);
void manejar_solicitudes(int);
void recibir_sleep(int);
t_instruccion* recibir_instruccion_io(t_paquete*);
interfaz_generica* buscar_interfaz_generica(char* nombre);
interfaz_stdin* buscar_interfaz_stdin(char* nombre);
interfaz_stdout* buscar_interfaz_stdout(char* nombre);
interfaz_dialfs* buscar_interfaz_dialfs(char* nombre);
void usar_interfaz_generica();
void usar_interfaz_stdin();
void usar_interfaz_stdout();
void usar_interfaz_dialfs();
#endif 
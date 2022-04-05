#include "memory.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

/* Função que inicia um novo processo restaurante através da função fork do SO. O novo
* processo irá executar a função execute_restaurant respetiva, fazendo exit do retorno.
* O processo pai devolve o pid do processo criado.

EXEMPLO TODO DELETE

int launch_process(int process_id, int process_code, struct communication_buffers* buffers, struct main_data* data, struct semaphores* sems) {
	int pid;
	if ((pid = fork()) == -1) {
		exit(1);
	}
	if (pid == 0) {
		switch (process_code) {
			case 0: exit(execute_client(process_id, buffers, data, sems));
			case 1:	exit(execute_proxy(process_id, buffers, data, sems));
			case 2:	exit(execute_server(process_id, buffers, data, sems));
			default: exit(-1);
		}
	} else {
		return pid;
	}
}

*/
int launch_restaurant(int restaurant_id, struct communication_buffers* buffers, struct main_data* data);


/* Função que inicia um novo processo motorista através da função fork do SO. O novo
* processo irá executar a função execute_driver, fazendo exit do retorno.
* O processo pai devolve o pid do processo criado.
*/
int launch_driver(int driver_id, struct communication_buffers* buffers, struct main_data* data);


/* Função que inicia um novo processo cliente através da função fork do SO. O novo
* processo irá executar a função execute_client, fazendo exit do retorno.
* O processo pai devolve o pid do processo criado.
*/
int launch_client(int client_id, struct communication_buffers* buffers, struct main_data* data);


/* Função que espera que um processo termine através da função waitpid. 
* Devolve o retorno do processo, se este tiver terminado normalmente.
*/
int wait_process(int process_id) {
	int result;
	waitpid(process_id, &result, 0);
	if (WIFEXITED(result)) {
		return WEXITSTATUS(result);
	} else {
		exit(1);
	}
}

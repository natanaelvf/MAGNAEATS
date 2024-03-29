#include "memory.h"
#include "main.h"
#include "driver.h"
#include "client.h"
#include "restaurant.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>


/**
 * Grupo: SO-26
Natanael Ferreira: 52678
Pedro Santos: 53677
 * /

/* Função que inicia um novo processo restaurante através da função fork do SO. O novo
* processo irá executar a função execute_restaurant respetiva, fazendo exit do retorno.
* O processo pai devolve o pid do processo criado.


*/
int launch_restaurant(int restaurant_id, struct communication_buffers* buffers, struct main_data* data)
{
	int pid;
	if ((pid = fork()) == -1) {
		exit(1);
	}

	if (pid == 0) {
		exit(execute_restaurant(restaurant_id, buffers, data));
	} else {
		return pid;
	}
}


/* Função que inicia um novo processo motorista através da função fork do SO. O novo
* processo irá executar a função execute_driver, fazendo exit do retorno.
* O processo pai devolve o pid do processo criado.
*/
int launch_driver(int driver_id, struct communication_buffers* buffers, struct main_data* data)
{
	int pid;
	if ((pid = fork()) == -1) {
		exit(1);
	}

	if (pid == 0) {
		exit(execute_driver(driver_id, buffers, data));
	} else {
		return pid;
	}
}

/* Função que inicia um novo processo cliente através da função fork do SO. O novo
* processo irá executar a função execute_client, fazendo exit do retorno.
* O processo pai devolve o pid do processo criado.
*/
int launch_client(int client_id, struct communication_buffers* buffers, struct main_data* data)
{
	int pid;
	if ((pid = fork()) == -1) {
		exit(1);
	}

	if (pid == 0) {
		exit(execute_client(client_id, buffers, data));
	} else {
		return pid;
	}
}

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

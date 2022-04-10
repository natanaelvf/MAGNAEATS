#include "memory.h"
#include "main.h"
#include "process.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

/**
 * Grupo: SO-26
Natanael Ferreira: 52678
Pedro Santos: 53677
 * /

void print_help()
{
	printf( "Ações disponíveis: \n");
	printf( "\t request <cliente> <restaurante> <dish> – criar um pedido de encomenda ao restaurante restaurant, feito pelo cliente client, pedindo o prato dish; \n" );
	printf( "\t status <id> – consultar o estado do pedido especificado por id; \n" );
	printf( "\t stop - termina a execução do do sistema MagnaEats \n" );
	printf( "\t help - mostrar as informações de ajuda sobre as opções anteriores. \n" );
}


/* Função que lê os argumentos da aplicação, nomeadamente o número
* máximo de operações, o tamanho dos buffers de memória partilhada
* usados para comunicação, e o número de clientes, de motoristas e de
* restaurantes. Guarda esta informação nos campos apropriados da
* estrutura main_data.
*/
void main_args(int argc, char* argv[], struct main_data* data)
{
	data->max_ops       = atoi(argv[1]);
	data->buffers_size  = atoi(argv[2]);
	data->n_clients     = atoi(argv[3]);
	data->n_drivers     = atoi(argv[4]);
	data->n_restaurants = atoi(argv[5]);
}

/* Função que reserva a memória dinâmica necessária para a execução
* do MAGNAEATS, nomeadamente para os arrays *_pids e *_stats da estrutura 
* main_data. Para tal, pode ser usada a função create_dynamic_memory.
*/
void create_dynamic_memory_buffers(struct main_data* data)
{
	data->client_pids = create_dynamic_memory(sizeof*(data->client_pids));
	data->restaurant_pids = create_dynamic_memory(sizeof*(data->restaurant_pids));
	data->driver_pids = create_dynamic_memory(sizeof*(data->driver_pids));

	data->client_stats = create_dynamic_memory(sizeof*(data->client_stats));
	data->restaurant_stats = create_dynamic_memory(sizeof*(data->restaurant_stats));
	data->driver_stats = create_dynamic_memory(sizeof*(data->driver_stats));
}

/* Função que reserva a memória partilhada necessária para a execução do
* MAGNAEATS. É necessário reservar memória partilhada para todos os buffers da
* estrutura communication_buffers, incluindo os buffers em si e respetivos
* pointers, assim como para o array data->results e variável data->terminate.
* Para tal, pode ser usada a função create_shared_memory.
*/

void create_shared_memory_buffers(struct main_data* data, struct communication_buffers* buffers)
{
	buffers->main_rest->ptrs = create_shared_memory(STR_SHM_MAIN_REST_PTR,sizeof(data->n_restaurants * sizeof(int)));
	buffers->main_rest->buffer = create_shared_memory(STR_SHM_MAIN_REST_BUFFER, sizeof(data->n_restaurants * sizeof(struct operation)));

	buffers->rest_driv->ptrs = create_shared_memory(STR_SHM_REST_DRIVER_PTR, sizeof(data->n_drivers * sizeof(int)));
	buffers->rest_driv->buffer = create_shared_memory(STR_SHM_REST_DRIVER_BUFFER, sizeof(data->n_drivers * sizeof(struct operation)));

	buffers->driv_cli->ptrs = create_shared_memory(STR_SHM_DRIVER_CLIENT_PTR,sizeof(data->n_clients * sizeof(int)));
	buffers->driv_cli->buffer = create_shared_memory(STR_SHM_DRIVER_CLIENT_BUFFER, sizeof(data->n_clients * sizeof(struct operation)));

	/* cria o espaço em memoria para o historico de operações */
	data->results = create_shared_memory(STR_SHM_RESULTS, sizeof(struct operation) * data->max_ops);

	/* Indica que a aplicação está a funcionar */
	data->terminate = create_shared_memory(STR_SHM_TERMINATE, sizeof(int));

}

/* Função que inicia os processos dos restaurantes, motoristas e
* clientes. Para tal, pode usar as funções launch_*,
* guardando os pids resultantes nos arrays respetivos
* da estrutura data.
*/
void launch_processes(struct communication_buffers* buffers, struct main_data* data)
{
	// client
	for(int n = 0; n < sizeof(data->client_pids) / sizeof(int); n++) 
		data->client_pids[n] = launch_client(data->client_pids[n], buffers, data);
	
	// driver
	for(int n = 0; n < sizeof(data->driver_pids) / sizeof(int); n++)
		data->driver_pids[n] = launch_driver(data->driver_pids[n], buffers, data);

	// restaurant
	for(int n = 0; n < sizeof(data->restaurant_pids) / sizeof(int); n++)
		data->restaurant_pids[n] = launch_restaurant(data->restaurant_pids[n], buffers, data);
}

/* Função que faz interação com o utilizador, podendo receber 4 comandos:
* request - cria uma nova operação, através da função create_request
* status - verifica o estado de uma operação através da função read_status
* stop - termina o execução do MAGNAEATS através da função stop_execution
* help - imprime informação sobre os comandos disponiveis
*/
void user_interaction(struct communication_buffers* buffers, struct main_data* data)
{
	char* exit_action = "stop";
	int op_counter = 0;
	const int max_answer = 12;
	print_help();

	char action[max_answer];
	char* user_action = fgets(action, max_answer, stdin);

	while (strcmp(user_action,exit_action) == 0) {
		if(strcmp(user_action, "request\n\n") == 0){
			create_request(&op_counter, buffers, data);
			printf( "Operação %d criada com sucesso!", op_counter);
			printf( "Introduzir ação: \n" );
		} else if (strcmp(user_action, "read") == 0){
			read_status(data);
			printf( "Introduzir ação: \n" );
		} else if(strcmp(user_action, "help") == 0){
			print_help();
			printf( "Introduzir ação: \n" );
			user_action = fgets(action, max_answer, stdin);
		} else {
			printf( "Ação não reconhecida, insira \'help\' para assistência.\n");
			printf( "Introduzir ação: \n" );
			user_action = fgets(action, max_answer, stdin);
		}
	}
	stop_execution(data, buffers);
}

/* Se o limite de operações ainda não tiver sido atingido, cria uma nova
* operação identificada pelo valor atual de op_counter e com os dados passados em
* argumento, escrevendo a mesma no buffer de memória partilhada entre main e restaurantes.
* Imprime o id da operação e incrementa o contador de operações op_counter.
*/
void create_request(int* op_counter, struct communication_buffers* buffers, struct main_data* data)
{
	if (data->max_ops > *op_counter) {
		struct operation* request = malloc(sizeof(struct operation));
		request->id = *op_counter;

		write_main_rest_buffer(buffers->main_rest, data->buffers_size, request);

		printf("O pedido #%d foi criado!", request->id);
		*op_counter = *op_counter + 1;
	}
}

/* Função que lê um id de operação do utilizador e verifica se a mesma
* é valida. Em caso afirmativo,
* imprime informação da mesma, nomeadamente o seu estado, o id do cliente
* que fez o pedido, o id do restaurante requisitado, o nome do prato pedido
* e os ids do restaurante, motorista, e cliente que a receberam e processaram.
*/
void read_status(struct main_data* data)
{
	int input;
	scanf("%d", &input);

	struct operation* op = malloc(sizeof(struct operation));

	*op = data->results[input];

	// TODO +segunda parte do if esta mal
	if(data->max_ops > op->id) {
		printf("op com estado: %d ", op->status);
		printf("pedida pelo cliente: %d ", op->requesting_client);
		printf("ao restaurante: %d \n", op->requested_rest);
		printf("correspondente ao prato: %s \n", op->requested_dish);
		printf("processada e recebida: \n - pelo restaurante: %d; \n", op->receiving_rest);
		printf("- pelo motorista: %d e \n ", op->receiving_driver);
		printf("- pelo cliente: %d. \n ", op->receiving_client);
	}

	free(op);
}

/* Função que termina a execução do programa MAGNAEATS. Deve começar por 
* afetar a flag data->terminate com o valor 1. De seguida, e por esta
* ordem, deve esperar que os processos filho terminem, deve escrever as
* estatisticas finais do programa, e por fim libertar
* as zonas de memória partilhada e dinâmica previamente 
* reservadas. Para tal, pode usar as outras funções auxiliares do main.h.
*/
void stop_execution(struct main_data* data, struct communication_buffers* buffers)
{
	*data->terminate = 1;
	wait_processes(data);
	write_statistics(data);
	destroy_memory_buffers(data, buffers);
}

/* Função que espera que todos os processos previamente iniciados terminem,
* incluindo restaurantes, motoristas e clientes. Para tal, pode usar a função 
* wait_process do process.h.
*/
void wait_processes(struct main_data* data)
{
	for(int i = 0; i < sizeof(data->client_pids[0])/sizeof(int); i++)
		wait_process(data->client_pids[i]);

	for(int i = 0; i < sizeof(data->client_pids[0])/sizeof(int); i++)
		wait_process(data->client_pids[i]);

	for(int i = 0; i < sizeof(data->client_pids[0])/sizeof(int); i++)
		wait_process(data->client_pids[i]);
}

/* Função que imprime as estatisticas finais do MAGNAEATS, nomeadamente quantas
* operações foram processadas por cada restaurante, motorista e cliente.
*/
void write_statistics(struct main_data* data)
{
	for (int i = 0; i < data->n_clients; i++)
		printf("Client %d, recebeu %d pedidos!\n", i, data->client_stats[data->client_pids[i]]);
	

	for (int i = 0; i < data->n_drivers; i++)
		printf("Driver %d, recebeu %d pedidos!\n", i, data->driver_stats[data->driver_pids[i]]);
	

	for (int i = 0; i < data->n_restaurants; i++)
		printf("Restaurante %d, recebeu %d pedidos!\n", i, data->restaurant_stats[data->restaurant_pids[i]]);
	
}

/* Função que liberta todos os buffers de memória dinâmica e partilhada previamente
* reservados na estrutura data.
*/
void destroy_memory_buffers(struct main_data* data, struct communication_buffers* buffers)
{
	// TODO probably wrong, ****_PTR devia ser destroyed de maneira diferente
	destroy_shared_memory(STR_SHM_MAIN_REST_PTR, buffers->main_rest, sizeof(buffers->main_rest));
	destroy_shared_memory(STR_SHM_MAIN_REST_BUFFER, buffers->main_rest, sizeof(buffers->main_rest));

	destroy_shared_memory(STR_SHM_REST_DRIVER_PTR, buffers->rest_driv, sizeof(buffers->rest_driv));
	destroy_shared_memory(STR_SHM_REST_DRIVER_BUFFER, buffers->rest_driv, sizeof(buffers->rest_driv));

	destroy_shared_memory(STR_SHM_DRIVER_CLIENT_PTR, buffers->driv_cli, sizeof(buffers->driv_cli));
	destroy_shared_memory(STR_SHM_DRIVER_CLIENT_BUFFER, buffers->driv_cli, sizeof(buffers->driv_cli));

	destroy_shared_memory(STR_SHM_RESULTS, data->results, sizeof(data->results));
	destroy_shared_memory(STR_SHM_TERMINATE, data->terminate, sizeof(data->terminate));

	destroy_dynamic_memory(data->client_pids);
	destroy_dynamic_memory(data->restaurant_pids);
	destroy_dynamic_memory(data->driver_pids);
	destroy_dynamic_memory(data->client_stats);
	destroy_dynamic_memory(data->restaurant_stats);
	destroy_dynamic_memory(data->driver_stats);
}

int main(int argc, char *argv[]) { 
	//init data structures 
	struct main_data* data = create_dynamic_memory(sizeof(struct 
	main_data)); 
	struct communication_buffers* buffers = 
	create_dynamic_memory(sizeof(struct communication_buffers)); 
	buffers->main_rest = create_dynamic_memory(sizeof(struct 
	rnd_access_buffer)); 
	buffers->rest_driv = create_dynamic_memory(sizeof(struct 
	circular_buffer)); 
	buffers->driv_cli = create_dynamic_memory(sizeof(struct 
	rnd_access_buffer)); 

	//execute main code 
	main_args(argc, argv, data); 
	create_dynamic_memory_buffers(data); 
	create_shared_memory_buffers(data, buffers); 
	launch_processes(buffers, data); 
	user_interaction(buffers, data); 

	//release memory before terminating 
	destroy_dynamic_memory(data); 
	destroy_dynamic_memory(buffers->main_rest); 
	destroy_dynamic_memory(buffers->rest_driv); 
	destroy_dynamic_memory(buffers->driv_cli); 
	destroy_dynamic_memory(buffers); 
}

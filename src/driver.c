#include "memory.h"
#include "main.h"
#include "driver.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

/**
 * Grupo: SO-26
Natanael Ferreira: 52678
Pedro Santos: 53677
 * /

/* Função principal de um Motorista. Deve executar um ciclo infinito onde em 
* cada iteração lê uma operação dos restaurantes e se a mesma tiver id 
* diferente de -1 e se data->terminate ainda for igual a 0, processa-a e
* escreve a resposta para os driveres. Operações com id igual a -1 são 
* ignoradas (op inválida) e se data->terminate for igual a 1 é porque foi 
* dada ordem de terminação do programa, portanto deve-se fazer return do
* número de operações processadas. Para efetuar estes passos, pode usar os
* outros métodos auxiliares definidos em driver.h.
*/
int execute_driver(int driver_id, struct communication_buffers* buffers, struct main_data* data)
{
    while(1)
    {
        struct operation* op = malloc(sizeof(struct operation));
        driver_receive_operation(op, buffers, data);
        int counter = 0;

        if (op->id != -1 && *data->terminate == 0) {

            driver_process_operation(op, driver_id, data, &counter);
            write_rest_driver_buffer(buffers->rest_driv, sizeof(buffers->rest_driv), op);

        }

        read_rest_driver_buffer(buffers->rest_driv, sizeof(buffers->rest_driv), op);

        if (op->id != -1 && *data->terminate == 0) {
            data->results[op->id] = *op;
        } 

        free(op);
        if (*data->terminate == 1) {
            return counter;
        }
    }
}


/* Função que lê uma operação do buffer de memória partilhada entre restaurantes e motoristas.
* Antes de tentar ler a operação, deve verificar se data->terminate tem valor 1.
* Em caso afirmativo, retorna imediatamente da função.
*/
void driver_receive_operation(struct operation* op, struct communication_buffers* buffers, struct main_data* data)
{
    if (*data->terminate == 1) {
        return;
    }
    read_rest_driver_buffer(buffers->rest_driv, data->buffers_size, op);
}


/* Função que processa uma operação, alterando o seu campo receiving_driver para o id
* passado como argumento, alterando o estado da mesma para 'D' (driver), e 
* incrementando o contador de operações. Atualiza também a operação na estrutura data.
*/
void driver_process_operation(struct operation* op, int driver_id, struct main_data* data, int* counter)
{
    op->receiving_driver = driver_id;
    op->status = 'D';
    *counter+=1;
    data->results = op;
}


/* Função que escreve uma operação no buffer de memória partilhada entre
* motoristas e driveres.
*/
void driver_send_answer(struct operation* op, struct communication_buffers* buffers, struct main_data* data)
{
    write_rest_driver_buffer(buffers->rest_driv, data->buffers_size, op);
}

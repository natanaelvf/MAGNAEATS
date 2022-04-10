#include "main.h"
#include "memory.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>   

/**
 * Grupo: SO-26
Natanael Ferreira: 52678
Pedro Santos: 53677
 * /


/* Função que reserva uma zona de memória partilhada com tamanho indicado
* por size e nome name, preenche essa zona de memória com o valor 0, e 
* retorna um apontador para a mesma. Pode concatenar o resultado da função
* getuid() a name, para tornar o nome único para o processo.
*/
void* create_shared_memory(char* name, int size)
{
    int fd   = shm_open(name,O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
    int *ptr = mmap(0, size, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    int ret;

    if (fd == -1) 
    {
        perror(name);
        exit(1);
    }

    ret = ftruncate(fd,size);
    if (ret == -1 && fd == -1) 
    {
        perror(name);
        exit(1);
    }
    return ptr;
}


/* Função que reserva uma zona de memória dinâmica com tamanho indicado
* por size, preenche essa zona de memória com o valor 0, e retorna um 
* apontador para a mesma.
*/
void* create_dynamic_memory(int size)
{
    int* buff;

    buff = calloc(size, sizeof(int));

    if (buff == NULL) {
        exit(1);
    }

    return buff;
}


/* Função que liberta uma zona de memória dinâmica previamente reservada.
*/
void destroy_shared_memory(char* name, void* ptr, int size)
{
    int ret = munmap(ptr, size);
    if (ret == -1)
    {
        perror(name);
        exit(2);
    }

    ret = shm_unlink(name);
    if (ret == -1)
    {
        perror(name);
        exit(2);
    }

    exit(0);
}


/* Função que liberta uma zona de memória partilhada previamente reservada.
*/
void destroy_dynamic_memory(void* ptr)
{
    free(ptr);
}


/* Função que escreve uma operação no buffer de memória partilhada entre a Main
* e os restaurantes. A operação deve ser escrita numa posição livre do buffer, 
* tendo em conta o tipo de buffer e as regras de escrita em buffers desse tipo.
* Se não houver nenhuma posição livre, não escreve nada.
*/
void write_main_rest_buffer(struct rnd_access_buffer* buffer, int buffer_size, struct operation* op)
{
    int i, *val;
	for(i=0; i < buffer_size; i++) {
		val = (int *)(buffer->ptrs + sizeof(int)*i);
		if ( *val == 0) {
			memcpy(buffer->ptrs + sizeof(struct operation)*i, op, sizeof(struct operation));
			*val = 1;
			return;
		}
	}
	return;
}


/* Função que escreve uma operação no buffer de memória partilhada entre os restaurantes
* e os motoristas. A operação deve ser escrita numa posição livre do buffer, 
* tendo em conta o tipo de buffer e as regras de escrita em buffers desse tipo.
* Se não houver nenhuma posição livre, não escreve nada.
*/
void write_rest_driver_buffer(struct circular_buffer* buffer, int buffer_size, struct operation* op)
{
   while (1) {
		while (((buffer->ptrs->in + 1) % buffer_size) == buffer->ptrs->out) {
			//buffer cheio; esperar que out seja avançado pelo consumidor
			sleep(1000);
		}
		memcpy(buffer->buffer + sizeof(struct operation) * buffer->ptrs->in, op, sizeof(struct operation));
		buffer->ptrs->in = (buffer->ptrs->in + 1) % buffer_size;
	}
}


/* Função que escreve uma operação no buffer de memória partilhada entre os motoristas
* e os clientes. A operação deve ser escrita numa posição livre do buffer, 
* tendo em conta o tipo de buffer e as regras de escrita em buffers desse tipo.
* Se não houver nenhuma posição livre, não escreve nada.
*/
void write_driver_client_buffer(struct rnd_access_buffer* buffer, int buffer_size, struct operation* op)
{
    int i, *val;
	for(i=0; i < buffer_size; i++) {
		val = (int *)(buffer->ptrs + sizeof(int)*i);
		if ( *val == 0) {
			memcpy(buffer->ptrs + sizeof(struct operation)*i, op, sizeof(struct operation));
			*val = 1;
			return;
		}
	}
	return;
}


/* Função que lê uma operação do buffer de memória partilhada entre a Main
* e os restaurantes, se houver alguma disponível para ler que seja direcionada ao restaurante especificado.
* A leitura deve ser feita tendo em conta o tipo de buffer e as regras de leitura em buffers desse tipo.
* Se não houver nenhuma operação disponível, afeta op->id com o valor -1.
*/
void read_main_rest_buffer(struct rnd_access_buffer* buffer, int rest_id, int buffer_size, struct operation* op)
{
    int i, *val;
	for(i=0; i < buffer_size; i++) {
		val = (int *)(buffer->ptrs + sizeof(int)*i);
		if ( *val == 1) {
			memcpy(op,buffer->ptrs + sizeof(struct operation)*i,  sizeof(struct operation));
			*val = 0;
			return;
		}
	}
	op = (struct operation *)-1;
}


/* Função que lê uma operação do buffer de memória partilhada entre os restaurantes e os motoristas,
* se houver alguma disponível para ler (qualquer motorista pode ler qualquer operação).
* A leitura deve ser feita tendo em conta o tipo de buffer e as regras de leitura em buffers desse tipo.
* Se não houver nenhuma operação disponível, afeta op->id com o valor -1.
*/
void read_rest_driver_buffer(struct circular_buffer* buffer, int buffer_size, struct operation* op)
{
    while (1) {
			while (buffer->ptrs->in == buffer->ptrs->out) {
				//buffer vazio;
				sleep(1000);
			}
			memcpy(op, buffer->buffer+sizeof(struct operation)*buffer->ptrs->in, sizeof(struct operation));
			buffer->ptrs->out = (buffer->ptrs->out + 1) % buffer_size;
	}
}


/* Função que lê uma operação do buffer de memória partilhada entre os motoristas e os clientes,
* se houver alguma disponível para ler dirijida ao cliente especificado. A leitura deve
* ser feita tendo em conta o tipo de buffer e as regras de leitura em buffers desse tipo. Se não houver
* nenhuma operação disponível, afeta op->id com o valor -1.
*/
void read_driver_client_buffer(struct rnd_access_buffer* buffer, int client_id, int buffer_size, struct operation* op)
{
    int i, *val;
	for(i=0; i < buffer_size; i++) {
		val = (int *)(buffer->ptrs + sizeof(int)*i);
		if ( *val == 1) {
			memcpy(op,buffer->ptrs + sizeof(struct operation)*i,  sizeof(struct operation));
			*val = 0;
			return;
		}
	}
	op = (struct operation *)-1;
}
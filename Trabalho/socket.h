/*
 *      File: socket.c
 *      Descrição: Biblioteca com as funções relacionadas com socket   
 */

#ifndef SOCKET_H
#define SOCKET_H


#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <assert.h>
#include <signal.h>
#include "arquivos.h"
#include <time.h>


#define TRUE 1
#define FALSE 0
#define TENTE 5 //Número de tentativas caso o pacote seja perdido. 
#define TAMNOME 128 //Tamanho do nome do arquivo
#define TAMBUFFER 256 //Tamanho do buffer

/**
 * Estrutura para o pacote que contem os metadados do arquivo.
 */
typedef struct metadados {
    char nome[TAMNOME]; // Nome do arquivo
    int tamanho; // Tamanho do arquivo
} metadados_t;


/**
 * Estrutura para os pacotes de segmentos.
 */
typedef struct segmentos {
    int seqNum; // Número de sequência
    int tamanho; // Tamanho do pacote sendo enviado
    int limite; // Variável auxiliar do temporizador
    unsigned char buff[TAMBUFFER]; //Buffer como fragmento do arquivo
    unsigned int checksum; // Soma de verificação
} segmentos_t;

/**
 * Estrutura para os ACKS. 
 */
typedef struct ACK {
    int seqNum; // Número de sequência
    int limite; // Variável auxiliar do temporizador
    unsigned int flag; // Flag para confirmar se pacote foi entregue.
} ack_t;

/**
 * checksum:    Faz a soma de verificação de palavras de TAM bytes
 * parametros:  buffer: Palavra de bytes em que será feito o checksum
 * 				tam:    Tamanho da palavra de bytes
 * returno:     Valor do checksum
 */
unsigned checksum(void *buffer, size_t tam);

/**
 * criaSocket: Dado o tipo de socket, cria um socket
 * @return Retorna o socket
 */
int criaSocket();

/**
 * fechaSocket: Fecha o socket dado se estiver aberto
 * parametro:   sock: Socket que será fechado
 */
void fechaSocket(int sock);

/**
 * enviarArquivo Dado um arquivo o mesmo é fragmentado enpacotado e enviado para o servidor
 * parametros: fp       Ponteiro para o arquivo a ser enviado
 * 			   sock     socket do cliente
 * 			   dados_s dados do socket do cliente
 */
void enviarArquivo(FILE *fp, int sock, struct sockaddr_in dados_s);

#endif /* SOCKET_H */
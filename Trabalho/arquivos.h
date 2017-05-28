/*
 *      File: arquivos.c
 *      Descrição: Biblioteca com as funções de manipulação 
 *				   de arquivos (leitura, escrita, abertura,
 *				   fechamento)      
 */

#ifndef ARQUIVOS_H
#define ARQUIVOS_H

#include <stdio.h>
#include <stdlib.h>


#define TRUE 1
#define FALSE 0

/**
 * criarArquivo: cria um arquivo
 * Parametros:   nome: Nome do arquivo
 * Retorno:      Ponteiro para o arquivo criado
 */
FILE *criaArquivo(char *nome);

/**
 * abrirArquivo: Abre o arquivo
 * Parametros:   nome: Nome do arquivo
 * Retorno:      Ponteiro para o arquivo aberto
 */
FILE *abrirArquivo(char *nome);

/**
 * fechaArquivo: Dado um ponteiro que determina o arquivo
 * 				 a ser fechado fecha o arquivo
 * Parametros: 	 fp: Ponteiro relacionado ao arquivo
 * Return:		 Nenhum
 */
void fechaArquivo (FILE *fp);

/**
 * tamanhoArquivo: Encontra o tamanho do arquivo
 * Parametros:     fp: Ponteiro que aponta para o arquivo
 * returno:         Tamanho do arquivo
 */
int tamanhoArquivo(FILE *fp);

#endif /* ARQUIVOS_H */
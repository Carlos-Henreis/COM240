
/**
 * abrirArquivo: Abre o arquivo
 * Parametros:   nome: Nome do arquivo
 * Retorno:      Ponteiro para o arquivo aberto
 */
#include "arquivos.h"

FILE *criaArquivo(char *nome) {
    FILE *fp = fopen(nome, "wb");
    if (!fp) {
        perror("O arquivo não criado!");
        exit(1);//paramos aqui
    }
    return fp;
}

FILE *abrirArquivo(char *nome) {
    FILE *fp = fopen(nome, "rb");

    if (!fp) {
        perror("O arquivo não existe!");
        exit(1);//paramos aqui
    }
    return fp;
}

/**
 * fechaArquivo: Dado um ponteiro que determina o arquivo
 *               a ser fechado fecha o arquivo
 * Parametros:   fp: Ponteiro relacionado ao arquivo
 * Return:       Nenhum
 */
void fechaArquivo (FILE *fp) {
    fclose (fp);
    return;
}

/**
 * tamanhoArquivo: Encontra o tamanho do arquivo
 * Parametros:     fp: Ponteiro que aponta para o arquivo
 * returno:         Tamanho do arquivo
 */
int tamanhoArquivo(FILE *fp) {
    int tam;

    fseek(fp, 0L, SEEK_END);
    tam = ftell(fp);
    printf("O arquivo tem %d bytes\n", tam);
    return tam;
}
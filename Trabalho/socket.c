/*
 *      File: socket.c
 *      Descrição: Biblioteca com as funções relacionadas com socket   
 */
#include "socket.h"

/**
 * checksum:    Faz a soma de verificação de palavras de TAM bytes
 * parametros:  buffer: Palavra de bytes em que será feito o checksum
 * 				tam:    Tamanho da palavra de bytes
 * returno:     Valor do checksum
 */
unsigned int checksum(void *buffer, size_t tam) {
    unsigned int seed = 0;
    unsigned char *buf = (unsigned char *) buffer;
    size_t i;

    for (i = 0; i < tam; ++i) {
        seed += (unsigned int) (*buf++);
    }

    return seed;
}

/**
 * criaSocket: Dado o tipo de socket, cria um socket
 * @return Retorna o socket
 */
int criaSocket() {
    int sock, tipoSocket;

    tipoSocket = SOCK_DGRAM; //socket UDP

    if ((sock = socket(AF_INET, tipoSocket, 0)) == -1) {
        perror("FALHA ao criar socket!\n");
        exit(-1);
    }

    printf("\nSocket criado!\n");

    return sock;
}

/**
 * fechaSocket: Fecha o socket dado se estiver aberto
 * parametro:   sock: Socket que será fechado
 */
void fechaSocket(int sock) {
    if (sock == -1) return;

    if (close(sock) == -1) {
        perror("ERRO ao fechar o socket! \n");
        exit(-1);
    }

    printf("Socket fechado!\n\n");
}


/**
 * enviarArquivo Dado um arquivo o mesmo é fragmentado enpacotado e enviado para o servidor
 * parametros: fp       Ponteiro para o arquivo a ser enviado
 * 			   sock     socket do cliente
 * 			   dados_s dados do socket do cliente
 */
void enviarArquivo(FILE *fp, int sock, struct sockaddr_in dados_s){
    ack_t ackMsg;
    segmentos_t segmento;
    unsigned slen = sizeof (dados_s);
    int read_from_file = TRUE;

    memset(segmento.buff, 0, sizeof (segmento.buff));

    /**
     * Setando o ponteiro do arquivo para o inicio do arquivo.
     */
    fseek(fp, 0L, SEEK_SET);

    /**
     * Setando algumas variáveis de flag.
     */
    printf("Enviando arquivo...\n");
    int seqNum = 0;
    int i = 0;
    int rval;
    int sends = 0;

    /**
     * Começando a enviar o arquivo.
     */
    while (!feof(fp)) {

        /**
         * Lógica para enviar um buffer inteiro
         */
        if (read_from_file == TRUE) {
            segmento.limite = 0;
            for (i = 0; i < sizeof (segmento.buff); i++) {
                fread(&(segmento.buff[i]), 1, 1, fp);
                if (feof(fp)) {
                    segmento.buff[i] = '\0';
                    break;
                }
            }
        } else {
            /**
             * Caso o pacote tenha sido perdido, aqui iniciamos o temporizador
             * no lado do cliente, e setamos o ponteiro para o pacote perdido.
             */
            ackMsg.limite = ++segmento.limite;
            segmento.seqNum = --seqNum;
            if (segmento.limite <= TENTE) {
                printf("\nTentativa %d\n", ackMsg.limite);

                /**
                 * Voltamos o ponteiro do arquivo para o pacote perdido.
                 */
                fseek(fp, (segmento.seqNum)*100, SEEK_SET);

                for (i = 0; i < sizeof (segmento.buff); i++) {
                    fread(&(segmento.buff[i]), 1, 1, fp);
                    if (feof(fp)) {
                        segmento.buff[i] = '\0';
                        break;
                    }
                }
            }
        }

        /**
         * Lógica para enviar menos que o buffer bytes 
         */
        if (i != 0) {
            /**
             * Calcula o checksum a ser enviado
             */
            segmento.checksum = checksum(segmento.buff, i);
            segmento.tamanho = i;
            segmento.seqNum = ++seqNum;
            printf("\nMSG Enviada: Id:%d ", segmento.seqNum);
            printf("tamanho:%d ", segmento.tamanho);
            printf("checksum:%d\n", segmento.checksum);

            /**
             * Caso seja um pacote válido é iniciada a transferência.
             * Enviando arquivo para o servidor.
             */
            ENVIO://Tá porco aqui :(
            if (sendto(sock, &segmento, sizeof (segmento), 0,
                    (struct sockaddr *) &dados_s, slen) <= 0) {//Tenta enviar
                perror("FALHA ao enviar!");
                fechaArquivo(fp);
                fechaSocket(sock);
                exit(1);
            }

            fd_set select_fds; /* fd's usado por select */
            struct timeval timeout; /* Valor de tempo para timeout */

            /* -----------------------------------------
             Setando a descrição para o select()
             ----------------------------------------- */
            FD_ZERO(&select_fds); /* Limpando o fd's */
            FD_SET(sock, &select_fds); /* Setando o bit que corresponde ao socket */

            /* -----------------------------------------
             Setando o valor do timeout
             ----------------------------------------- */
            timeout.tv_sec = 1; /* Timeout setado para 5 segundos + 0 micro segundos*/
            timeout.tv_usec = 0;

            printf("Esperando pela mensagem...\n");

            /**
             * Perda de pacotes.
             * O tempo de espera máximo permitido é de TENTE tentativas.
             */
            if (segmento.limite == TENTE) {
                printf("\nO temporizador estourou!\nFALHA na transferência do arquivo!\n\n");
                fechaArquivo(fp);
                fechaSocket(sock);
                exit(1);
            }

            /**
	     * [TEMPORIZADOR]
             * Perda de conexão.
             * O tempo maximo permitido é de 5 segundos;
             */
            if (select(32, &select_fds, NULL, NULL, &timeout) == 0) {
                if(sends < TENTE) {
                    ++sends;
                    printf("\nReenviando, tentativa %d\n", sends);
                    goto ENVIO;
                }
                /**
                 * Perda de conexão.
                 * O tempo maximo permitido é de 5 segundos;
                 */
                printf("\nO temporizador estourou!\nFALHA na transferência do arquivo!\n\n");
                fechaArquivo(fp);
                fechaSocket(sock);
                exit(1);
            } else {
                sends = 0;
                rval = recvfrom(sock, &ackMsg, sizeof (ackMsg), 0,
                        (struct sockaddr *) &dados_s, &slen);
            }

            /**
             * Recebendo o ACK.
             */
            if (rval < 0) {
                perror("ERRO ao ler o fluxo de mensagens!\n");
            } else if (rval == 0) {
                printf("Finalizando conexão...\n");
            } else {
                printf("ACK %d RECEBIDO.\n", ackMsg.seqNum);
            }

            /**
             * Verificando o ACK.
             * Se flag = 0 não ocorreu erro.
             */
            if (seqNum == ackMsg.seqNum && ackMsg.flag == 0) {
                read_from_file = TRUE;
            } else {
                read_from_file = FALSE;
            }
        }
    }
}
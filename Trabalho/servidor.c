#include "socket.h"

int main(void) {
    metadados_t id;
    int sock; 
    struct sockaddr_in servidor;
    int mysock;
    int porta;
    char buff[5];
    int rval;
    FILE *fp;
    segmentos_t segmento;
    ack_t ack_msg;
    unsigned tamSocket = sizeof (servidor);
    int seqNum = 1;

    /**
     *  Enquanto não for informada uma porta válida,
     *  o servidor ficará impedido de ser ativado.
     */
    printf("Entre com o valor da porta para conexão\n");
    scanf (" %d", &porta);

    /**
     * Criação do socket para comunicão entre cliente e servidor.
     */
    sock = criaSocket();

    /**
     * Setando as opoções para permitir que uma porta seja
     * reutilizada, logo após o servidor ser fechado.
     */
    int reuseAddr = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof (reuseAddr)) == -1) {
        perror("ERRO ao setar as opções do socket!\n");
        fechaSocket(sock);
        exit(-1);
    }
    /**
     * Configurando o endereço do socket do servidor.
     */
    memset((void *) &servidor, 0, sizeof (servidor));
    servidor.sin_family = AF_INET;
    servidor.sin_port = htons(porta);
    servidor.sin_addr.s_addr = htonl(INADDR_ANY);

    /**
     * Vinculando a porta informada ao socket do servidor.
     */
    if (bind(sock, (struct sockaddr *) &servidor, sizeof (servidor)) == -1) {
        perror("FALHA ao vincular a porta ao socket!");
        fechaSocket(sock);
        exit(-1);
    }

    printf("Aguardando por conexão...\n\n");

    /**
     *  Os dados de cabeçalho são enviados para a criação do novo arquivo.
     *  @param nome    [nome do novo arquivo]
     *  @param tamanho         [tamanho do novo arquivo]
     */
    mysock = sock;
    if ((rval = recvfrom(mysock, &id, sizeof (id), 0, (struct sockaddr *) &servidor, &tamSocket)) < 0) {
        perror("ERRO ao ler o fluxo de mensagens!\n");
    } else if (rval == 0) {
        printf("Finalizando conexão...\n");
    } else {
        printf("MSG RECEBIDA -->[arquivo]: %s ", id.nome);
        printf("- [tamanho]: %d bytes\n", id.tamanho);
    }

    /**
     * De fato aqui o novo arquivo é criado no servidor.
     */
    memset(buff, 0, sizeof (buff));
    printf("Copia do arquivo iniciada...\n\n");
    fp = criaArquivo(id.nome);
    int read_bytes = 0;

    /**
     * Finalmente iniciamos o envido dos pacotes do cliente para o servidor.
     */
    while (read_bytes < id.tamanho) {
        fd_set select_fds; /* fd's usado por select */
        struct timeval timeout; /* Valor de tempo para timeout */

        /* -----------------------------------------
         Setando a descrição para o select()
         ----------------------------------------- */
        FD_ZERO(&select_fds); /* Limpando o fd's */
        FD_SET(mysock, &select_fds); /* Setando o bit que corresponde ao socket */

        /* -----------------------------------------
         Setando o valor do timeout
         ----------------------------------------- */
        timeout.tv_sec = 5; /* Timeout setado para 5 segundos + 0 micro segundos*/
        timeout.tv_usec = 0;

        printf("Esperando pela mensagem...\n");

        /**
         * Espera pela mensagem do cliente.
         */
        if (select(32, &select_fds, NULL, NULL, &timeout) == 0) {

            /**
	     * [TEMPORIZADOR]
             * Perda de conexão.
             * O tempo maximo permitido é de 5 segundos;
             */
            printf("\nO temporizador estourou!\nFALHA na transferência do arquivo!\n\n");
            fechaArquivo(fp);
            fechaSocket(mysock);
            exit(1);
        } else {
            rval = recvfrom(mysock, &segmento, sizeof (segmento), 0,
                    (struct sockaddr *) &servidor, &tamSocket);
        }

        if (rval < 0) {
            perror("ERRO ao ler o fluxo de mensagens!\n");
        } else if (rval == 0) {
            printf("Finalizando conexão...\n");
        } else {
            unsigned int x = checksum(segmento.buff, segmento.tamanho);

            /**
             * Se o checksum calculado for o mesmo que o checksum recebido,
             * enviamos um ACK para o pacote correspondente.
             */
            printf("Checksum calculado : %d\n", x);
            printf("MSG VÁLIDA %d recebida: ", segmento.seqNum);
            printf("tamanho:%d ", segmento.tamanho);
            printf("checksum:%d\n", segmento.checksum);

            ack_msg.seqNum = seqNum; // número de sequencia esperado
            ack_msg.limite = segmento.limite; // temporizador

            /**
             * Tentamos enviar o pacote pelo numero de sequência solicitado.
             */
            if ((x == segmento.checksum) && (segmento.seqNum == seqNum)) {
                ack_msg.flag = 0;
                ++seqNum;

                /**
                 * Enviando ACK de confirmação ao cliente.
                 */
                printf("Enviando ACK %d\n", ack_msg.seqNum);
                if ((rval = sendto(mysock, &ack_msg, sizeof (ack_msg), 0,
                        (struct sockaddr *) &servidor, tamSocket)) < 0) {
                    perror("ERRO ao ler o fluxo de mensagens!\n");
                } else if (rval == 0) {
                    printf("Finalizando conexão...\n");
                } else {

                    /**
                     * Gravação dos bytes que vem dos pacotes no arquivo no servidor.
                     */
                    fwrite(&segmento.buff[0], segmento.tamanho, 1, fp);
                    printf("ACK ENVIADO.\n\n");
                    read_bytes += segmento.tamanho;
                }

                /**
                 * Caso o pacote tenha sido perdido, é feita a solicitação novamente.
                 * E também é iniciado o temporizador.
                 */
            } else {
                ack_msg.flag = 1;
                printf("PACOTE PERDIDO OU DEFEITUOSO...\n");
                ack_msg.seqNum -= 1; 
                if (ack_msg.limite <= (TENTE - 1)) {
                    printf("Reenviando ACK %d - ", ack_msg.seqNum);
                    printf("Tentativa %d\n", ack_msg.limite + 1);
                    if ((rval = sendto(mysock, &ack_msg, sizeof (ack_msg), 0,
                            (struct sockaddr *) &servidor, tamSocket)) < 0) {
                        perror("ERRO ao ler o fluxo de mensagens!\n");
                    } else if (rval == 0) {
                        printf("Finalizando conexão...\n");
                    } else {
                        printf("ACK REENVIADO.\n\n");
                    }
                } else {
                    printf("\nO temporizador estourou!\n FALHA na transferência do arquivo!\n");
                    fechaArquivo(fp);
                    fechaSocket(mysock);
                    return 0;
                }
            }

        }
    }

    /**
     * O arquivo foi transferido com sucesso.
     * Todos os arquivos e sockets são fechados.
     */
    printf("Cópia do arquivo completada com sucesso!\n");
    fechaArquivo(fp);
    fechaSocket(mysock);

    return 0;
}

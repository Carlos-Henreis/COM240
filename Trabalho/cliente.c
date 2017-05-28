
#include "socket.h"

#define IP "127.0.0.1"

int main(void) {
    int sock;
    int porta;
    metadados_t id;
    struct sockaddr_in cliente;
    FILE *fp;
    char nomeArquivo[TAMNOME];
    char nomeServidor[TAMNOME];
    struct hostent *hp;
    int tamArquivo;
    int tamSocket = sizeof (cliente);

    printf("Entre com a porta de conexão\n");
    scanf (" %d", &porta);
    
    printf("Entre com o caminho do arquivo a ser enviado\n");
    setbuf (stdin, NULL);
    scanf(" %[^\n]", nomeArquivo);

   	printf("Entre com o nome que o arquivo vai receber no servidor\n");
    setbuf (stdin, NULL);
    scanf(" %[^\n]", nomeServidor);

    printf("\nServidor: %s", IP);
    printf(":%d\n", porta);
    printf("Arquivo enviado: %s\n", nomeArquivo);

    fp = abrirArquivo(nomeArquivo);
    tamArquivo = tamanhoArquivo(fp);

    /**
     * Os dados do arquivo a ser enviado são alocados
     * na struct correspondente.
     */
    strcpy(id.nome, nomeServidor);
    id.tamanho = tamArquivo;

    /**
     * Criação do socket para comunicão entre cliente e servidor
     */
    sock = criaSocket();

    /**
     * Configurando o endereço do socket do servidor.
     */
    cliente.sin_family = AF_INET;

    /**
     * Estabelecimento de conexão se o cliente e servidor estiverem
     * rodando no mesmo computador.
     */
    if (strcmp(IP, "localhost") == 0) {
        if (strcmp(nomeArquivo, nomeServidor) == 0) {
            perror("Tentativa de sobreescrever o arquivo no mesmo computador!\n");
            fechaArquivo(fp);
            fechaSocket(sock);
            exit(1);
        }

        /**
         * Pega o endereço de IP de 32-bit do host
         * por frescurite pode entrar com o dominio em vez do IP
         */

        hp = gethostbyname(IP);
        if (hp == 0) {
            perror("FALHA ao pegar endereço de IP!");
            fechaArquivo(fp);
            fechaSocket(sock);
            exit(1);
        }
        //Se estamos aqui deu tudo certo com IP do servidor
        memcpy(&cliente.sin_addr, hp->h_addr, hp->h_length);
    } else {
        /**
         * Estabelecimento de conexão se o cliente e servidor estiverem
         * rodando em computadores diferentes.
         */
        cliente.sin_addr.s_addr = inet_addr(IP);

    }

    /**
     * Vinculando a porta informada ao socket do cliente.
     */
    cliente.sin_port = htons(porta);

    /**
     * Enviando os dados para o servidor.
     */
    if (sendto(sock, &id, sizeof (id), 0, (struct sockaddr *) &cliente, tamSocket) <= 0) {
        perror("Falha ao enviar!");
        fechaArquivo(fp);
        fechaSocket(sock);
        exit(1);
    }

    printf("\nChecando o estabelecimento de conexão: enviando %s ", id.nome);
    printf("com %d bytes\n", id.tamanho);

    /**
     * Enviando o arquivo
     */
    enviarArquivo(fp, sock, cliente);

    /**
     * O arquivo foi transferido com sucesso.
     * Todos os arquivos e sockets são fechados.
     */
    printf("\nO arquivo foi enviado com sucesso!\n");
    fechaArquivo(fp);
    fechaSocket(sock);

    return 0;
}

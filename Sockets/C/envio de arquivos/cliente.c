#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define TIMEOUT 10
#define MAX_FALHAS_SEGUIDAS 1

#define SERVIDOR "127.0.0.1"
#define PORTA 1227

#define TAMANHO_CAMINHO_ARQUIVO 512
#define TAMANHO_NOME_ARQUIVO 256
#define TAMANHO_MENSAGEM 4

FILE *arquivo;

struct pacote {
    unsigned short checkSum;
    char mensagem[TAMANHO_MENSAGEM];
    unsigned int numeroSequencia;
    unsigned int numeroReconhecimento;
    unsigned int flag;
    unsigned int ack;
};

struct pacote pkg;
struct pacote pkgResponse;

struct hostent *hostent;

struct timeval temporizador;

struct sockaddr_in cliAddr;
struct sockaddr_in servAddr;            

int socketDescriptor;

int cliLen = sizeof (cliAddr);
int servLen = sizeof (servAddr);

void configureClientSocket();
void configureRemote();
void createSocketClient();
void configureLocal();

// CLIENTE

void enviarArquivo();
int sendSingleMessage();
int receiveSingleResponse(int numeroSequencia);
int checkPackageResponse(int numeroSequencia);

// ARQUIVO

FILE * abrirArquivoLeitura(char nomeArquivo[TAMANHO_NOME_ARQUIVO]);
FILE * abrirArquivoEscrita(char nomeArquivo[TAMANHO_NOME_ARQUIVO]);
void fecharArquivo(FILE *arquivo);

// OUTRAS FUNCOES

unsigned short somaVerificacao(unsigned short *ptr, int nbytes);
void closeSocket();
void closeApplication(char *s);

int main(void) {
    char opcao;

    do {
        printf("MENU\n\n1 - Receber arquivo\n2 - Sair\n\nOPCAO: ");
        scanf("%c", &opcao);
        switch (opcao) {
            case '1':
                configureClientSocket();
                enviarArquivo();
                break;
            case '2':
                closeApplication("Programa finalizado\n");
                break;
            default:
                printf("Opção inválida\n");
                break;

        }
    } while (opcao != 2);
    return 0;
}

// SOKET CLIENTE

void configureClientSocket() {
    configureRemote();
    createSocketClient();
    configureLocal();
}

void configureRemote() {
    hostent = gethostbyname(SERVIDOR);
    if (hostent == NULL) {
        closeApplication("Host desconhecido\n");
    }
    servAddr.sin_family = hostent->h_addrtype;
    memcpy((char *) &servAddr.sin_addr.s_addr, hostent->h_addr_list[0], hostent->h_length);
    servAddr.sin_port = htons(PORTA);
}

void createSocketClient() {
    socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
    temporizador.tv_sec = TIMEOUT;
    temporizador.tv_usec = 0;
    setsockopt(socketDescriptor, SOL_SOCKET, SO_RCVTIMEO, (char *) &temporizador, sizeof (temporizador));
    if (socketDescriptor == -1) {
        closeApplication("Não foi possivel abrir socket\n");
    }
}

void configureLocal() {

    cliAddr.sin_family = AF_INET;
    cliAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    cliAddr.sin_port = htons(0);

    int bindStatus = bind(socketDescriptor, (struct sockaddr *) &cliAddr, cliLen);
    if (bindStatus == -1) {
        closeApplication("Não foi possivel abrir a porta\n");
    }
}

// CLIENTE

void enviarArquivo() {

    long int tamanhoArquivo;
    unsigned int numeroPacotes;
    unsigned int contadorPacotes = 1;
    unsigned int numeroFalhasSeguidas;
    long int posicaoAtual;

    int bytesRead;
    unsigned int numeroSequencia = 0;
    char bufferMensagem[TAMANHO_MENSAGEM];
    char caminhoArquivo[TAMANHO_CAMINHO_ARQUIVO];

    printf("Caminho do caminho que deseja enviar(EXIT PARA ENCERRAR): ");
    scanf("%s", caminhoArquivo);

    if (strcmp(caminhoArquivo, "EXIT") == 0) {
        closeApplication("Programa finalizado\n");
    }

    arquivo = abrirArquivoLeitura(caminhoArquivo);
    if (arquivo == NULL) {
        closeApplication("Arquivo não encontrado\n");
    }

    posicaoAtual = ftell(arquivo);
    fseek(arquivo, 0, SEEK_END);
    tamanhoArquivo = ftell(arquivo);
    fseek(arquivo, posicaoAtual, SEEK_SET);

    numeroPacotes = (tamanhoArquivo / TAMANHO_MENSAGEM) + 1;
    //numeroPacotes = ceil(tamanhoArquivo/TAMANHO_MENSAGEM);

    printf("\nEnviando arquivo ...\n\n");

    while (!feof(arquivo)) {

        bytesRead = fread(bufferMensagem, 1, TAMANHO_MENSAGEM, arquivo);
        bufferMensagem[bytesRead] = 0;

        strcpy(pkg.mensagem, bufferMensagem);

        pkg.checkSum = somaVerificacao((unsigned short*) bufferMensagem, strlen(pkg.mensagem));

        pkg.numeroSequencia = numeroSequencia;

        if (contadorPacotes == numeroPacotes) {
            pkg.flag = 0;
        } else {
            pkg.flag = 1;
        }

        numeroFalhasSeguidas = 0;

        while (1) {

            printf("PACOTE %d DE %d\t", contadorPacotes, numeroPacotes);

            if (sendSingleMessage(pkg, socketDescriptor, servAddr, servLen) == 1) {
                numeroSequencia += TAMANHO_MENSAGEM;
                printf("RESPOSTA DO SERVIDOR\t");
                if (receiveSingleResponse(numeroSequencia) == 1) {
                    break;
                } else {
                    numeroFalhasSeguidas++;
                    if (numeroFalhasSeguidas == MAX_FALHAS_SEGUIDAS) {
                        closeApplication("Número máximo de falhas foi alcançado");
                    }
                    numeroSequencia = pkgResponse.numeroReconhecimento;
                    pkg.numeroSequencia = numeroSequencia;
                }
            }
        }
        contadorPacotes += 1;
    }
    fecharArquivo(arquivo);
    closeSocket();
}

int sendSingleMessage() {
    int bytesSent = sendto(socketDescriptor, &pkg, sizeof (struct pacote), 0, (struct sockaddr *) &servAddr, servLen);
    if (bytesSent >= 0) {
        printf("0k\n");
        return 1;
    }
    printf("FALHA\n");
    return 0;
}

int receiveSingleResponse(int numeroSequencia) {
    int bytesRecived = recvfrom(socketDescriptor, &pkgResponse, sizeof (struct pacote), 0, (struct sockaddr *) &servAddr, (socklen_t*) & servLen);
    if ((checkPackageResponse(numeroSequencia)) && (bytesRecived >= 0)) {
        printf("0k\n\n");
        return 1;
    } else {
        printf("FALHA\n\n");
        return 0;
    }
}

int checkPackageResponse(int numeroSequencia) {
    if ((pkgResponse.numeroReconhecimento == numeroSequencia) && (pkgResponse.ack == 1)) {
        return 1;
    } else {
        return 0;
    }
}

void closeApplication(char *s) {
    perror(s);
    exit(-1);
}
FILE * abrirArquivoEscrita(char nomeArquivo[TAMANHO_NOME_ARQUIVO]) {
    FILE *arquivo = fopen(nomeArquivo, "w");
    return arquivo;
}

void fecharArquivo(FILE *arquivo) {
    if (fclose(arquivo) != 0) {
        closeApplication("Erro ao fechar arquivo\n");
    }
}

// OUTRAS FUNÇÕES

void closeSocket() {
    close(socketDescriptor);
}


unsigned short somaVerificacao(unsigned short *ptr, int nbytes) {
    register long soma;
    unsigned short byte_impar;
    register short resposta;

    soma = 0;
    while (nbytes > 1) {
        soma += *ptr++;
        nbytes -= 2;
    }
    if (nbytes == 1) {
        byte_impar = 0;
        *((u_char*) & byte_impar) = *(u_char*) ptr;
        soma += byte_impar;
    }

    soma = (soma >> 16) + (soma & 0xffff);
    soma = soma + (soma >> 16);
    resposta = (short) ~soma;

    return (resposta);
}

FILE * abrirArquivoLeitura(char nomeArquivo[TAMANHO_NOME_ARQUIVO]) {
    FILE *arquivo = fopen(nomeArquivo, "r");
    if (arquivo != NULL) {
        return arquivo;
    } else {
        closeApplication("Erro ao abrir o arquivo para leitura\n");
        return NULL;
    }
}
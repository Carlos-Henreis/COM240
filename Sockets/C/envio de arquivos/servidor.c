
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

// SOCKET SERVIDOR

void configureServerSocket();
void createSocketServer();
void configureServer();

// SERVIDOR

void listenToClient();
void receiveFile();
int receiveSingleMessage(int numeroReconhecimento);
int checkPackage(int numeroReconhecimento);
int sendSingleResponse();

// ARQUIVO

FILE * abrirArquivoEscrita(char nomeArquivo[TAMANHO_NOME_ARQUIVO]);
void fecharArquivo(FILE *arquivo);

// OUTRAS FUNCOES

unsigned short somaVerificacao(unsigned short *ptr, int nbytes);
void closeSocket();
void closeApplication(char *s);

int main(void) {

    char opcao;

    do {
        printf("MENU\n\n1 - Eviar arquivo\n2 - Sair\n\nOPCAO: ");
        scanf("%c", &opcao);
        switch (opcao) {
            case '1':
                configureServerSocket();
                listenToClient();
                break;
            case '2':
                closeApplication("Programa finalizado\n");
                break;
            default:
                printf("Opção inválida\n");
                break;

        }
    }while (opcao != 2);

    return 0;

}


// SOKET SERVIDOR

void configureServerSocket() {
    createSocketServer();
    configureServer();
}

void createSocketServer() {
    socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketDescriptor == -1) {
        closeApplication("Não foi possivel abrir socket\n");
    }
}

void configureServer() {
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(PORTA);
    int bindStatus = bind(socketDescriptor, (struct sockaddr *) &servAddr, servLen);
    if (bindStatus == -1) {
        closeApplication("Não foi possivel abrir a porta\n");
    }
}

// SERVIDOR

void listenToClient() {
    printf("\nAguardando pacotes ...\n\n");
    receiveFile();
    closeSocket();
}

void receiveFile() {
    int numeroReconhecimento = 0;
    int contadorPacotes = 1;
    while (1) {
        printf("PACOTE %d RECEBIDO\t", contadorPacotes);
        if (receiveSingleMessage(numeroReconhecimento) == 1) {
            if (contadorPacotes == 1) {
                time_t mytime;
                mytime = time(NULL);
                arquivo = abrirArquivoEscrita(ctime(&mytime));
            }

            fwrite(pkg.mensagem, 1, strlen(pkg.mensagem), arquivo);

            numeroReconhecimento += TAMANHO_MENSAGEM;
            pkgResponse.ack = 1;
        } else {
            pkgResponse.ack = 0;
        }

        contadorPacotes += 1;

        pkgResponse.numeroReconhecimento = numeroReconhecimento;

        while (1) {
            printf("RESPOSTA ENVIADA\t");
            if ((sendSingleResponse(pkgResponse, socketDescriptor, cliAddr, cliLen)) == 1) {
                break;
            }
        }

        if (pkg.flag == 0) {
            printf("FIM DOS PACOTES\n\n");
            break;
        }
    }
    fecharArquivo(arquivo);
}

int receiveSingleMessage(int numeroReconhecimento) {

    int bytesRecived = recvfrom(socketDescriptor, &pkg, sizeof (struct pacote), 0, (struct sockaddr *) &cliAddr, (socklen_t*) & cliLen);
    if ((checkPackage(numeroReconhecimento)) && (bytesRecived >= 0)) {
        printf("0k\n");
        return 1;
    } else {
        printf("FALHA\n");
        return 0;
    }
}

int checkPackage(int numeroReconhecimento) {
    if ((pkg.numeroSequencia == numeroReconhecimento) && (pkg.checkSum == somaVerificacao((unsigned short*) pkg.mensagem, strlen(pkg.mensagem)))) {
        return 1;
    } else {
        return 0;
    }
}

int sendSingleResponse() {
    int bytesSent = sendto(socketDescriptor, &pkgResponse, sizeof (struct pacote), 0, (struct sockaddr *) &cliAddr, cliLen);
    if (bytesSent >= 0) {
        printf("0k\n\n");
        return 1;
    }
    printf("FALHA\n\n");
    return 0;
}


// ARQUIVO



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

void closeSocket() {
    close(socketDescriptor);
}

void closeApplication(char *s) {
    perror(s);
    exit(-1);
}
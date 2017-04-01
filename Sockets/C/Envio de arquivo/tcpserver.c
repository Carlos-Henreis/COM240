#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define PORTASERVIDOR 2017
#define TAMMENSG 1024
#define TAMBUFFER 1
#define TAMNOME 64


int main() {
    FILE *arquivo;
    int sock, conectado, bytes_recebidos , true = 1, pacote;  
    char buffer[TAMBUFFER], msg_enviada [TAMMENSG], mensagem[TAMMENSG], nome_arq[TAMNOME];       

    struct sockaddr_in server_addr, client_addr;    
    socklen_t sin_size;
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {//Checa problemas
        perror("Socket");
        exit(1);
    }
    /*http://www.ataliba.eti.br/sections/old-hack/unsekurity/texto1/sockets2.txt*/
    if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int)) < 0) {
        perror("Setsockopt");
        exit(1);
    }
    
    server_addr.sin_family = AF_INET;         
    server_addr.sin_port = htons(PORTASERVIDOR);     
    server_addr.sin_addr.s_addr = INADDR_ANY; 
    bzero(&(server_addr.sin_zero),8); 

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))
                                                                   == -1) {
        perror("Não é possível conectar");
        exit(1);
    }

    if (listen(sock, 5) < 0) {
        perror("Listen");
        exit(1);
    }

    


    while(1) { //loop para ocorrer várias conexões(não simultânea) 
        printf("\nTCPServidor esperando cliente na porta %d", PORTASERVIDOR);
        fflush(stdout);
        sin_size = sizeof(struct sockaddr_in);

        conectado = accept(sock, (struct sockaddr *)&client_addr,&sin_size);
        printf("\n Recebendo arquivo de: (%s , %d)", inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
        bytes_recebidos = recv(conectado,nome_arq,TAMNOME,0);//Revcebe resposta do cliente
        nome_arq[bytes_recebidos] = '\0';//Armazena a mgs
        arquivo = fopen (nome_arq, "w");
        if (strcmp(nome_arq, "s")){//finaliza conexao
            break;
        }

        if (!arquivo){
            printf("erro\n");//finaliza a execução do servidor
            return 0;
        }
        
        pacote =1;
        while (1) {//loop para persistir o arquivo
			bytes_recebidos = recv(conectado,buffer,TAMBUFFER,0);//Revcebe resposta do cliente
			buffer[bytes_recebidos] = '\0';//Armazena a o buffer
			bytes_recebidos = recv(conectado,mensagem,TAMMENSG,0);
			mensagem[bytes_recebidos] = '\0';
			if (strcmp (mensagem, "terminou") == 0){
				break;
			}
			else{ 
                printf("PACOTE %d recebido\n", pacote);
				fwrite(buffer, 1, TAMBUFFER, arquivo);
                pacote ++;
			}

        } 
        fclose(arquivo);
        printf("arquivo %s recebido e persistido com sucesso\n", nome_arq);
        strcpy (msg_enviada, "arquivo Recebido");
        send(conectado, msg_enviada,strlen(msg_enviada), 0);//Enviar mensagem de confirmação para cliente  

    }       

  close(sock);//Termina conexão do servidor
  return 0;
} 

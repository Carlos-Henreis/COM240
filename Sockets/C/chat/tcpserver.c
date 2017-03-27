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


int main() {
    int sock, conectado, bytes_recebidos , true = 1;  
    char msg_enviada [TAMMENSG] , msg_recebida[TAMMENSG];       

    struct sockaddr_in server_addr, client_addr;    
    int sin_size;
    
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

    printf("\nTCPServidor esperando cliente na porta %d", PORTASERVIDOR);
    fflush(stdout);


    while(1) { //loop para ocorrer várias conexões(não simultânea) 

        sin_size = sizeof(struct sockaddr_in);

        conectado = accept(sock, (struct sockaddr *)&client_addr,&sin_size);

        printf("\n Possuo conexão com: (%s , %d)", inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));

        while (1) {//loop para mensagem
          printf("\n Enviar mensagem (s para sair): ");
          gets(msg_enviada);//le a mensagem do servidor
          
          if (strcmp(msg_enviada , "s") == 0) {//Teste para sair (caso seja digitado s)
            send(conectado, msg_enviada,strlen(msg_enviada), 0); 
            close(conectado);
            break;
          }
           
          else
             send(conectado, msg_enviada,strlen(msg_enviada), 0);//Enviar mensagem para cliente  

          bytes_recebidos = recv(conectado,msg_recebida,TAMMENSG,0);//Revcebe resposta do cliente

          msg_recebida[bytes_recebidos] = '\0';//Armazena a mgs

          if (strcmp(msg_recebida , "s") == 0) {//Se a mgs for 's' devemos encerrar a conexão
            close(conectado);
            break;
          }

          else 
            printf("\n Mensagem Recebida = %s " , msg_recebida);
          fflush(stdout);
        }
    }       

  close(sock);//Termina conexão do servidor
  return 0;
} 

/* tcpclient.c */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define IP_SERVIDOR "127.0.0.1"
#define PORTASERVIDOR 2017
#define TAMMENSG 1024


int main() {

	int sock, bytes_recebidos;  
	char msg_enviada[TAMMENSG],msg_recebida[TAMMENSG];
	struct hostent *host;
	struct sockaddr_in server_addr;  

	host = gethostbyname(IP_SERVIDOR);

	if(host==NULL) {
		perror("Erro no host(ip)");
		exit(1);
	}
	//cria socket e verifica se há algum erro
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Socket");
		exit(1);
	}
	//ligando na porta
	server_addr.sin_family = AF_INET;     
	server_addr.sin_port = htons(PORTASERVIDOR);   
	server_addr.sin_addr = *((struct in_addr *)host->h_addr);
	bzero(&(server_addr.sin_zero),8); 

	if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
		perror("Connect");
		exit(1);
	}

	while(1){//loop para as mensagens
		bytes_recebidos=recv(sock,msg_recebida,TAMMENSG,0);//recebe mensagem do servidor
		msg_recebida[bytes_recebidos] = '\0';
		if (strcmp(msg_recebida , "conexão encerrada pelo servidor\n") == 0){//Se o servidor querer sair
			printf("%s\n", msg_recebida);
			close(sock);//fecha o socket
			break;
		}
		else
			printf("\n Mensagem Recebida = %s " , msg_recebida);
		printf("\n Enviar mensagem (s para sair): ");
        fgets(msg_enviada, TAMMENSG, stdin);//le a mensagem do cliente
          
		if (strcmp(msg_enviada , "s\n") != 0)
			send(sock,msg_enviada,strlen(msg_enviada), 0);//A mensagem é enviada

		else{ //senão devemos encerrar a conexão
			strcpy (msg_enviada, "conexão encerrada pelo cliente\n");
			send(sock,msg_enviada,strlen(msg_enviada), 0);   
			close(sock);
			break;
		}

	}   
	return 0;
}

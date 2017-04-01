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
#define PORTASERVIDOR 2019
#define TAMMENSG 1024
#define TAMBUFFER 1
#define TAMNOME 64


int main() {
	FILE *arquivo;
	int sock, bytes_recebidos, pacote;  
	char buffer[TAMBUFFER],msg_enviada[TAMMENSG] ,msg_recebida[TAMMENSG], nome_arq[TAMNOME];
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

	printf("Entre com o nome do arquivo\n");
	scanf ("%s", nome_arq);
	arquivo = fopen (nome_arq, "r");

	if (!arquivo) {
		printf("O arquivo %s não existe\n", nome_arq);

	}
	else {
		send(sock, nome_arq,TAMNOME, 0);//O nome do arquivo é enviada
		pacote =1;
		while (!feof(arquivo)) {
	       	fread(buffer, TAMBUFFER, 1, arquivo);
			send(sock,buffer,TAMBUFFER, 0);//buffer enviado
			strcpy (msg_enviada, "mandei");
			send (sock, msg_enviada, TAMMENSG, 0);//msg de confirmação
			printf("PACOTE %d enviado\n", pacote);
			pacote++;
		
		}
		strcpy (buffer, "");
		send(sock,buffer,TAMBUFFER, 0);//Esse aqui não vai ser persistido mais é enviado para manter a integridade no server
		strcpy (msg_enviada, "terminou");
		send(sock, msg_enviada,TAMMENSG, 0);
		printf("Arquivo enviado para servidor\n");		

	}
	fclose (arquivo);
	bytes_recebidos=recv(sock,msg_recebida,TAMMENSG,0);//recebe mensagem do servidor
	msg_recebida[bytes_recebidos] = '\0';
	close(sock);//Termina conexão do cliente
	return 0;
}


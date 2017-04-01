#  Para compilar

gcc -Wall -o * tcp*.c
 onde * == server ou client
 
#  Para rodar 
 ./server ou
 ./client

O cliente conecta ao servidor e envia um arquivo ao servidor.

Esta implementação está configurada de tal forma para ser excecutada localmente, para rodar remotamente deve-se mudar a constante IPSERVER no tcpclient.c para o ip do servidor para o qual se deseja conectar.

Ainda falta fazer a modularização do código...

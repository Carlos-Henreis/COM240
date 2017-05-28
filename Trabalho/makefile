# variables
CC = gcc

SERVER = servidor
CLIENT = cliente
ARQUIVOS = arquivos
SOCKET = socket

# rules
all: clean ${SERVER} ${CLIENT} cleanobj

udp: clean ${SERVER} ${CLIENT} cleanobj

${SERVER}: ${SOCKET}.o ${ARQUIVOS}.o
${CLIENT}: ${SOCKET}.o ${ARQUIVOS}.o
${ARQUIVOS}.o: ${ARQUIVOS}.h
${SOCKET}.o: ${SOCKET}.h


clean:
	rm -f *.o ${SERVER} ${CLIENT}
cleanobj:
	rm -f *.o


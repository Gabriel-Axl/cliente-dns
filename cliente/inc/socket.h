#ifndef SOCKET_H
#define SOCKET_H

#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/time.h>

#define SERVER_IP "8.8.8.8"
#define SERVER_PORT 53

//Estrutura baseada no artigo: <https://www.binarytides.com/dns-query-code-in-c-with-winsock/>
struct cabecalhoDNS {
    unsigned short id; // número de identificação
    unsigned char rd :1; // recursão desejada
    unsigned char tc :1; // mensagem truncada
    unsigned char aa :1; // resposta autoritativa
    unsigned char opcode :4; // propósito da mensagem
    unsigned char qr :1; // sinalizador de consulta/resposta
    unsigned char rcode :4; // código de resposta
    unsigned char cd :1; // verificação desativada
    unsigned char ad :1; // dados autenticados
    unsigned char z :1; // reservado
    unsigned char ra :1; // recursão disponível
    unsigned short q_count; // número de entradas de questão
    unsigned short ans_count; // número de entradas de resposta
    unsigned short auth_count; // número de entradas de autoridade
    unsigned short add_count; // número de entradas de recurso
};

void enviarPacoteDNS(char * hostname, char * client);

#endif /* SOCKET_H */
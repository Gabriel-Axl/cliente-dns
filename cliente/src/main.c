#include <stdio.h> 
#include "socket.h"

int main ( ){
    unsigned char hostname[100];
    unsigned char client[100];

    printf("Informe o Nome do dominio: ");
    scanf("%s", hostname);
    printf("Informe o Nome do cliente DNS: ");
    scanf("%s", client);

    enviarPacoteDNS(hostname, client);
}
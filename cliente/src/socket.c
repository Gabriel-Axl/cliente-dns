#include "socket.h"

int sockfd;
struct sockaddr_in server_addr;
unsigned char consultaDNS[65536];
unsigned char respostaDNS[65536];

void inicializaSocket(char * client){
    // Criação do socket UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("Erro ao criar o socket");
        exit(EXIT_FAILURE);
    }

    struct timeval timeout;
    timeout.tv_sec = 2; // Tempo limite de 2 segundos
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) < 0)
    {
        perror("Erro ao configurar timeout");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));

    // Configuração do endereço do servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, client, &server_addr.sin_addr) <= 0) {
        perror("Endereço inválido");
        exit(EXIT_FAILURE);
    }
}

void formatarDominio(unsigned char* dns,unsigned char* hostname) {
    //função pra transformar o dominio no formato da solicitação dns 
    // youtube.com para 07youtube03com\0
    strcat((char*)hostname, ".");
    int cont = 0;
    int point1 = 0;
    for (int i = 0; hostname[i] != '\0'; i++) {
        if (hostname[i] != '.') {
            cont++;
        } else {
            *dns++ = cont; // Insere o comprimento da parte do nome
            strncpy((char*)dns, (char*)(hostname + point1), cont); // Copia a parte do nome para dns
            dns += cont; // Move o ponteiro dns para o final da parte do nome
            cont = 0; // Reinicia o contador
            point1 = i + 1; // Atualiza o ponto de início para a próxima parte do nome
        }
    }
    *dns++ = 0; // Insere o byte de terminação
}

void enviarPacoteDNS(char * hostname, char * client){
    inicializaSocket(client);

    // Configuração do cabeçalho DNS
    unsigned char *nomeDominio = &consultaDNS[sizeof(struct cabecalhoDNS)];
    formatarDominio(nomeDominio, hostname);//formata o dominio e insere ao final da dns query


    unsigned short *infoPergunta = (unsigned short *)&consultaDNS[sizeof(struct cabecalhoDNS) + (strlen((char *)nomeDominio) + 1)];
    *infoPergunta++ = htons(2); // qtype
    *infoPergunta = htons(1);   // qclass

    struct cabecalhoDNS *dns = (struct cabecalhoDNS *)&consultaDNS;
    dns->id = htons(getpid());
    dns->qr = 0;
    dns->opcode = 0;
    dns->aa = 0;
    dns->tc = 0;
    dns->rd = 1;
    dns->ra = 0;
    dns->z = 0;
    dns->ad = 0;
    dns->cd = 0;
    dns->rcode = 0;
    dns->q_count = htons(1);
    dns->ans_count = 0;
    dns->auth_count = 0;
    dns->add_count = 0;

    // Envio da consulta DNS
    if (sendto(sockfd, (char *)consultaDNS, sizeof(struct cabecalhoDNS) + (strlen((const char *)nomeDominio) + 1) + sizeof(infoPergunta), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Erro ao enviar a consulta");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Recebimento da resposta DNS
    int server_addr_len = sizeof(server_addr);
    int tamanhoResposta;
    int counter = 0;
    while (counter < 3)
    {
        tamanhoResposta = recvfrom(sockfd, (char *)respostaDNS, sizeof(respostaDNS), 0, (struct sockaddr *)&server_addr, &server_addr_len);
        if (tamanhoResposta < 0)
        {
            counter++;
            if (counter == 3)
            {
                printf("Nao foi possivel coletar entrada NS para %s\n", hostname);
                close(sockfd);
                exit(EXIT_FAILURE);
            }
        }
        else break;
    }

    int isNS = 0;
    int notNS = 0;
    int control = 0;
    char temp[20] = "";
    int len = strlen(temp); 
    for (int i = 0; i < tamanhoResposta; i++) {
        // Checa se existe alguma consulta do tipo NS
        if(respostaDNS[i] == 0x00 && isNS == 0) {
            isNS++;
        }else if(respostaDNS[i] == 0x02 && isNS == 1) {
            isNS++;
            notNS = 1;
        } else {
            isNS = 0;
        }

        // Pega o name server
        if(control >= 3) {
            control++;
            len = strlen(temp); 
            temp[len] = respostaDNS[i];
            temp[++len] = '\0';
        } else {
            if(respostaDNS[i] == 0x00 && control != 1) {
                control++;
            }else if(respostaDNS[i] == 0x07 && control != 2) {
                control++;
            }else if(respostaDNS[i] == 0x04 && control != 3) {
                control++;
            }else if (control < 3){
                control = 0;
            };
        };
        if(control >= 7 ) {
            printf("%s <> %s.%s\n", hostname, temp, hostname);
            temp[0] = '\0';
            control = 0;
        };
        // if(respostaDNS[i] == 0xc0 && respostaDNS[i+1] == 0x0c) {
        //     printf("Domain Name: %s <> nome_servidor_email: %s\n", nomeDominio, nomeDominio);
        //     control = 0;
        // };
    }
    if (notNS == 0) {
        printf("Dominio %s nao possui entrada NS", hostname);
    }
    printf("\n");

    close(sockfd);
}

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

int hexToInt(unsigned char hex) {
    int inteiro;
    char temp[3]; // Array temporário para armazenar o caractere hexadecimal e o terminador de string
    
    // Converte o valor hexadecimal para sua representação como caractere
    sprintf(temp, "%02X", hex);
    temp[2] = '\0'; // Adiciona um terminador de string

    sscanf(temp, "%X", &inteiro); // Use %02X para converter o valor hexadecimal para inteiro
    
    return inteiro;
}
char *ponteiroParaServerName(unsigned char *respostaDNS, int index, int limite) {
    char *host = malloc(256);
    if (host == NULL) {
        exit(1);
    }

    int hostLen = 0; 

    for(int x=index; x < limite; x++) {
        if(respostaDNS[x] == 0xc0) {
            int newIndex = hexToInt(respostaDNS[x+1]);
            char *temp = ponteiroParaServerName(respostaDNS, newIndex, limite);
            strcat(host, temp);
            break;
        }
        if(respostaDNS[x] == 0x00) {        
            break;
        }

        if(isalnum(respostaDNS[x]) || respostaDNS[x] == '.'){
            hostLen = strlen(host); 
            host[hostLen] = respostaDNS[x];
            host[++hostLen] = '\0';                
        } else {
            if(host){
                hostLen = strlen(host); 
            }
            host[hostLen] = '.';
            host[++hostLen] = '\0';
        }        
    }
    return host;   
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
    
    int jump = 0;
    char temp[99] = "";
    int len = strlen(temp); 
    int control = 0;
    for (int i = 0; i < tamanhoResposta; i++) {
        if(respostaDNS[i] == 0x02 && respostaDNS[i + 2] == 0x01){              
            if ( jump == 0){
                jump = 1;
                continue;
            }
            int j = i + 8;
            int tamanho = hexToInt(respostaDNS[j]);
            for(int k = 0; k < tamanho; k++){
                if(respostaDNS[k+ (j+1)] == 0xc0 ) {
                    int index = hexToInt(respostaDNS[k+ (j+2)]);

                    char *temp2;
                    temp2 = ponteiroParaServerName(respostaDNS, index, tamanhoResposta);

                    strcat(temp, temp2);              
                    break;
                };
                // colocar ponto
                if(isalnum(respostaDNS[k+ (j+1)])){                    
                    len = strlen(temp); 
                    temp[len] = respostaDNS[k+ (j+1)];
                    temp[++len] = '\0';                
                } else {
                    len = strlen(temp); 
                    temp[len] = '.';
                    temp[++len] = '\0';
                }
            }
            printf("%s <> %s\n", hostname, temp);
            temp[0] = '\0';
            control=1;
        }
    }
    if(control==0){
        printf("Dominio %s nao possui entrada NS\n", hostname);
    }

    close(sockfd);
}
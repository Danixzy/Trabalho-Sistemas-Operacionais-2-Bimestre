#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <locale.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
    setlocale(LC_ALL, "Portuguese");
    WSADATA wsa;
    SOCKET socketCliente;
    struct sockaddr_in enderecoServidor;
    char buffer[512];
    int resultado;
    char resposta[10];


    resultado = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (resultado != 0) {
        printf("Erro ao inicializar o Winsock: %d\n", resultado);
        return 1;
    }


    socketCliente = socket(AF_INET, SOCK_STREAM, 0);
    if (socketCliente == INVALID_SOCKET) {
        printf("Erro ao criar o socket: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    enderecoServidor.sin_family = AF_INET;
    enderecoServidor.sin_addr.s_addr = inet_addr("127.0.0.1"); // IP local
    enderecoServidor.sin_port = htons(8888);


    resultado = connect(socketCliente, (struct sockaddr *)&enderecoServidor, sizeof(enderecoServidor));
    if (resultado == SOCKET_ERROR) {
        printf("Erro ao conectar ao servidor: %ld\n", WSAGetLastError());
        closesocket(socketCliente);
        WSACleanup();
        return 1;
    }

 
    while (1) {
       
        resultado = recv(socketCliente, buffer, sizeof(buffer), 0);
        if (resultado == SOCKET_ERROR) {
            printf("Erro ao receber dados do servidor: %ld\n", WSAGetLastError());
            closesocket(socketCliente);
            WSACleanup();
            return 1;
        }

        buffer[resultado] = '\0'; 


        printf("%s\n", buffer);

      
        if (strstr(buffer, "Jogar novamente?") != NULL) {
            fgets(resposta, sizeof(resposta), stdin);
            resposta[strcspn(resposta, "\n")] = '\0'; 

     
            send(socketCliente, resposta, strlen(resposta), 0);

     
            if (strcmp(resposta, "nao") == 0) {
                break;
            } else {
              
                system("cls");
                continue; 
            }
        }

    
        if (strstr(buffer, "Reiniciando o jogo") != NULL) {
            system("cls"); 
            continue; 
        }

       
        if (strstr(buffer, "Jogador") != NULL) {
            printf("Sua jogada (1-9): ");
            if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
                printf("Erro ao ler a entrada.\n");
                continue;
            }

      
            resultado = send(socketCliente, buffer, strlen(buffer), 0);
            if (resultado == SOCKET_ERROR) {
                printf("Erro ao enviar dados: %ld\n", WSAGetLastError());
                closesocket(socketCliente);
                WSACleanup();
                return 1;
            }
        }
    }

    closesocket(socketCliente);
    WSACleanup();
    return 0;
}



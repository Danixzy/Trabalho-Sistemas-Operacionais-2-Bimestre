#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <pthread.h>
#include <locale.h>

#pragma comment(lib, "ws2_32.lib")

char tabuleiro[3][3] = { {'1', '2', '3'}, {'4', '5', '6'}, {'7', '8', '9'} };
int jogadorAtual = 1; 
pthread_mutex_t travaTabuleiro;

void resetarTabuleiro() {
    char c = '1';
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            tabuleiro[i][j] = c++;
        }
    }
}


void imprimirTabuleiro() {
    int i, j;

    system("cls");
    printf("\n\nJogo da Velha\n\n");
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            printf(" %c ", tabuleiro[i][j]);
            if (j < 2) printf("|");
        }
        if (i < 2) printf("\n---|---|---\n");
    }
    printf("\n");
}

int verificarVencedor() {
    int i;
    for (i = 0; i < 3; i++) {
        if (tabuleiro[i][0] == tabuleiro[i][1] && tabuleiro[i][1] == tabuleiro[i][2]) return 1;
        if (tabuleiro[0][i] == tabuleiro[1][i] && tabuleiro[1][i] == tabuleiro[2][i]) return 1;
    }
    if (tabuleiro[0][0] == tabuleiro[1][1] && tabuleiro[1][1] == tabuleiro[2][2]) return 1;
    if (tabuleiro[0][2] == tabuleiro[1][1] && tabuleiro[1][1] == tabuleiro[2][0]) return 1;
    return 0;
}


int verificarEmpate() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (tabuleiro[i][j] != 'X' && tabuleiro[i][j] != 'O') {
                return 0; 
            }
        }
    }
    return 1; 
}

void *lidarComCliente(void *socketCliente) {
    SOCKET cliente = *(SOCKET *)socketCliente;
    char buffer[512];
    int resultado;
    char resposta[10]; 

    while (1) {
        imprimirTabuleiro();

      
        sprintf(buffer, "Jogador %d, é sua vez.", jogadorAtual);
        send(cliente, buffer, strlen(buffer), 0);


        resultado = recv(cliente, buffer, sizeof(buffer), 0);
        if (resultado == SOCKET_ERROR) {
            printf("Erro ao receber dados do cliente.\n");
            closesocket(cliente);
            pthread_exit(NULL);
        }

        int posicao = buffer[0] - '0'; 

      
        if (tabuleiro[(posicao - 1) / 3][(posicao - 1) % 3] == 'X' || tabuleiro[(posicao - 1) / 3][(posicao - 1) % 3] == 'O') {
            strcpy(buffer, "Erro: Posição já ocupada. Escolha outra posição.");
            send(cliente, buffer, sizeof(buffer), 0);
            continue; // Mantém a vez do jogador
        }

    
        char marca = (jogadorAtual == 1) ? 'X' : 'O';
        pthread_mutex_lock(&travaTabuleiro); // Trava o tabuleiro
        tabuleiro[(posicao - 1) / 3][(posicao - 1) % 3] = marca;
        pthread_mutex_unlock(&travaTabuleiro); // Libera o tabuleiro

     
        if (verificarVencedor()) {
           
            
            do{
            	
            	imprimirTabuleiro();
            	sprintf(buffer, "Jogador %d venceu! Jogar novamente? (sim/nao): ", jogadorAtual);
            	send(cliente, buffer, strlen(buffer), 0);

            	resultado = recv(cliente, resposta, sizeof(resposta), 0);
           		resposta[resultado] = '\0';
            
			} while (strcmp(resposta, "sim") != 0 && strcmp(resposta, "nao") != 0);
            

            if (strcmp(resposta, "nao") == 0) {
                closesocket(cliente);
                pthread_exit(NULL);
            } else if (strcmp(resposta, "sim") == 0) {
                resetarTabuleiro(); // Reinicia o tabuleiro
                jogadorAtual = 1;
                continue;
            }
        }

   
        if (verificarEmpate()) {
        	  do{
            imprimirTabuleiro();
            sprintf(buffer, "O jogo terminou em empate! Jogar novamente? (sim/nao): ");
            send(cliente, buffer, strlen(buffer), 0);

            resultado = recv(cliente, resposta, sizeof(resposta), 0);
            resposta[resultado] = '\0';
            	} while (strcmp(resposta, "sim") != 0 && strcmp(resposta, "nao") != 0);

            if (strcmp(resposta, "nao") == 0) {
                closesocket(cliente);
                pthread_exit(NULL);
            } else if (strcmp(resposta, "sim") == 0) {
                resetarTabuleiro(); // Reinicia o tabuleiro
                jogadorAtual = 1;
                continue;
            }
        }

      
        jogadorAtual = (jogadorAtual == 1) ? 2 : 1;
    }
}


int main() {
    setlocale(LC_ALL, "Portuguese");
    WSADATA wsa;
    SOCKET socketServidor, socketCliente;
    struct sockaddr_in enderecoServidor, enderecoCliente;
    int tamanhoEndereco = sizeof(enderecoCliente);
    pthread_t threadId;


    WSAStartup(MAKEWORD(2, 2), &wsa);

  
    socketServidor = socket(AF_INET, SOCK_STREAM, 0);
    if (socketServidor == INVALID_SOCKET) {
        printf("Erro ao criar o socket!\n");
        return 1;
    }

 
    enderecoServidor.sin_family = AF_INET;
    enderecoServidor.sin_addr.s_addr = INADDR_ANY;
    enderecoServidor.sin_port = htons(8888);

   
    if (bind(socketServidor, (struct sockaddr *)&enderecoServidor, sizeof(enderecoServidor)) == SOCKET_ERROR) {
        printf("Erro ao fazer bind!\n");
        closesocket(socketServidor);
        WSACleanup();
        return 1;
    }


    listen(socketServidor, 3);
    printf("Aguardando jogador...\n");

    pthread_mutex_init(&travaTabuleiro, NULL);

  
    while ((socketCliente = accept(socketServidor, (struct sockaddr *)&enderecoCliente, &tamanhoEndereco)) != INVALID_SOCKET) {
        printf("Jogador conectado.\n");
        pthread_create(&threadId, NULL, lidarComCliente, (void *)&socketCliente);
    }

    closesocket(socketServidor);
    WSACleanup();
    return 0;
}



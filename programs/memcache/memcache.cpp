#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include "include/laserpants/dotenv/dotenv.h"

using namespace std;

int main(){
    dotenv::init();
    //deficion de variables
     int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    //crear socket del servidor
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Error al crear el socket del servidor");
        exit(EXIT_FAILURE);
    }

    // Configurar la dirección del servidor
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(3001); // Puerto del servidor
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Enlazar el socket a la dirección del servidor
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Error al enlazar el socket");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

     // Escuchar conexiones entrantes
    if (listen(serverSocket, 5) == -1) {
        perror("Error al escuchar");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    cout << "Esperando conexiones entrantes..." << endl;

     // Lógica del cliente (conectarse al otro servidor)
    const char *otherServerAddress = "127.0.0.1";
    int otherServerPort = 3002;

    int clientSocketToOtherServer = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocketToOtherServer == -1) {
        perror("Error al crear el socket del cliente para el otro servidor");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in otherServerAddr;
    memset(&otherServerAddr, 0, sizeof(otherServerAddr));
    otherServerAddr.sin_family = AF_INET;
    otherServerAddr.sin_port = htons(otherServerPort);
    inet_pton(AF_INET, otherServerAddress, &otherServerAddr.sin_addr);

    // Intentar conectarse al otro servidor
    if (connect(clientSocketToOtherServer, (struct sockaddr *)&otherServerAddr, sizeof(otherServerAddr)) == -1) {
        perror("Error al conectar con el otro servidor");
        close(serverSocket);
        close(clientSocketToOtherServer);
        exit(EXIT_FAILURE);
    }

    cout << "Conexión establecida con el otro servidor" << endl;


    while (true) {
        // Aceptar la conexión entrante
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            perror("Error al aceptar la conexión");
            continue; // Continuar esperando conexiones
        }

        cout << "Cliente conectado" << endl;
    }

    return 0;
}   
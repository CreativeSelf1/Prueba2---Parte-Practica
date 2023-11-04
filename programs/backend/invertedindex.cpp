#include <iostream>
#include "include/laserpants/dotenv/dotenv.h"
#include "include/loads.h"
#include <unordered_map>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include <unistd.h>
#include <cstdlib>

using namespace std;

int main(){
    dotenv::init();
    //Definición de variables
    string extention, files_in, files_out, idx;
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    extention = string(getenv("EXTENTION"));
    files_in = string(getenv("FILES_IN"));
    files_out = string(getenv("FILES_OUT"));
    idx = string(getenv("FILE"));
    int numThreads = atoi(getenv("AMOUNT_THREADS"));


    //generar inverted_index_file
    generateFILEIDX(extention,files_in, files_out, numThreads, idx);


    //cargar en memoria
    unordered_map<string, vector<pair<string, int>>> UmapIDX = mapIDX(idx);
    cout << UmapIDX.size()<<endl;


    //crear socket del servidor
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Error al crear el socket del servidor");
        exit(EXIT_FAILURE);
    }

    // Configurar la dirección del servidor
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(3000); // Puerto del servidor
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
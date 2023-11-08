#include <iostream>
#include "include/laserpants/dotenv/dotenv.h"
#include "include/loads.h"
#include "include/buscador.h"
#include <unordered_map>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include "include/json.hpp"

using namespace std;
using json = nlohmann::json;

void sendJsonToMemcache(int clientSocket, const json& resultadoJson) {
    string serializedJson = resultadoJson.dump();
    // Enviar la cadena al servidor de Memcache
    ssize_t bytesSent = send(clientSocket, serializedJson.c_str(), serializedJson.length(), 0);

    if (bytesSent == -1) {
        perror("Error al enviar datos al servidor de Memcache");
        close(clientSocket);
        // Puedes agregar un return aquí para salir de la función o manejar el error según tu lógica.
    }
}

void procesarMensaje(int clientSocket, const char *mensaje, unordered_map<string, vector<pair<string, int>>>& UmapIDX) {
    int topk = atoi(getenv("TOPK"));
    cout << "Datos recibidos del cliente: " << mensaje << endl;
    string mensajeStr(mensaje);
    json j = json::parse(mensajeStr);
    json contexto = j["contexto"];  // Obtener "contexto" como un objeto JSON
    string textoABuscar = contexto["txtToSearch"];  // Corregir el nombre de la clave

    vector<string> palabrasEncontradas = buscarPalabras(textoABuscar, UmapIDX);
    //CantidadPalabrasArchivo(UmapIDX,palabrasEncontradas, 5);
    json resultadoJson = obtenerJsonCantidadPalabrasArchivo(UmapIDX, palabrasEncontradas, topk);
    // cout << resultadoJson.dump(2) << endl;
    sendJsonToMemcache(clientSocket, resultadoJson);
}

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
    

    //crear socket del servidor
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Error al crear el socket del servidor");
        exit(EXIT_FAILURE);
    }

    // Configurar la dirección del servidor
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(3002); // Puerto del servidor
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

        char buffer[1024];
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead == -1) {
            perror("Error al recibir datos del cliente");
        } else if (bytesRead == 0) {
            cout << "Cliente desconectado" << endl;
            close(clientSocket);
            continue; // Continuar esperando conexiones
        } else {
            buffer[bytesRead] = '\0';

            // Llamar a la función para procesar el mensaje
            procesarMensaje(clientSocket, buffer, UmapIDX);
    }

   
    return 0;
    }   
}
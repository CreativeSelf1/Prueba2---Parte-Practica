#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <map>
#include "include/laserpants/dotenv/dotenv.h"

using namespace std;

struct SearchResult
{
    string archivo;
    int repeticion;
};


void procesarMensaje(int clientSocket, const char *mensaje, int clientSocketToOtherServer, map<string, vector<SearchResult>>& memoriaCache) {
    cout << "Datos recibidos del cliente: " << mensaje << endl;
    string mensajeStr(mensaje);

    size_t origenInicio = mensajeStr.find("origen:\"") + 8;
    size_t origenFin = mensajeStr.find("\"", origenInicio);
    string origen = mensajeStr.substr(origenInicio, origenFin - origenInicio);

    size_t destinoInicio = mensajeStr.find("destino:\"") + 9;
    size_t destinoFin = mensajeStr.find("\"", destinoInicio);
    string destino = mensajeStr.substr(destinoInicio, destinoFin - destinoInicio);

    size_t textoInicio = mensajeStr.find("txtToSerarch:\"") + 14;
    size_t textoFin = mensajeStr.find("\"", textoInicio);
    string textoABuscar = mensajeStr.substr(textoInicio, textoFin - textoInicio);

    cout << origen << " " << destino << " " << textoABuscar << endl;

    auto it = memoriaCache.find(textoABuscar);
    if (it != memoriaCache.end()) {
        // Enviar la respuesta al frontend
        // enviarRespuestaAlFrontend(clientSocket, it->second, true);
        cout << "encontrado" << endl;
    } else {
        cout << "no encontrado" << endl;
    }
}

bool VerificarCondicion(const char *mensaje){
    return true;
}

void enviarBackend(int backend, const char *mensaje){
    send(backend, mensaje, strlen(mensaje), 0);
}

int main(){
    dotenv::init();
    //deficion de variables
    int serverSocket, clientSocket;
    map<string, vector<SearchResult>> memoriaCache;

    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    SearchResult resultadoEjemplo;
    resultadoEjemplo.archivo = "ejemplo.txt";
    resultadoEjemplo.repeticion = 3;     
    memoriaCache["hola"].push_back(resultadoEjemplo);
    
    // Lógica del server 
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

     // Lógica del cliente (conectarse al otro servidor bakend)
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

        // Leer datos del cliente
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
            procesarMensaje(clientSocket, buffer, clientSocketToOtherServer, memoriaCache);
            
            // if(VerificarCondicion(buffer)){
            //     const char *mensaje = "mensaje para el backend";
            //     enviarBackend(clientSocketToOtherServer, mensaje);
            // }
        
        }

    }
    return 0;
}   
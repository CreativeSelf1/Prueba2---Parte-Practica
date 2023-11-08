#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <map>
#include "include/laserpants/dotenv/dotenv.h"
#include "include/json.hpp"

using namespace std;
using json = nlohmann::json;

struct SearchResult
{
    string archivo;
    int repeticion;
};

map<string, vector<SearchResult>> memoriaCache;

void agregarResultadosAlMapa(const json& resultadosJson, string textoABuscar) {
    vector<SearchResult> nuevosResultados;
    const json& resultados = resultadosJson["contexto"]["resultados"];
    for (const auto& resultado : resultados) {
        SearchResult nuevoResultado;
        nuevoResultado.archivo = resultado["archivo"];
        nuevoResultado.repeticion = resultado["repeticion"];
        nuevosResultados.push_back(nuevoResultado);
    }
    memoriaCache[textoABuscar] = nuevosResultados;
}

void sendMemcacheToFrontend(int clientSocket, const vector<SearchResult>& resultados, bool isFound) {
    json jsonResponse;
    jsonResponse["origen"] = string(getenv("HOST"));
    jsonResponse["destino"] = string(getenv("FRONT"));
    jsonResponse["contexto"]["tiempo"] = "100ns";
    jsonResponse["contexto"]["ori"] = "CACHE";
    jsonResponse["contexto"]["isFound"] = isFound;
    
    json resultadosJSON = json::array();
    for (const auto& resultado : resultados) {
        json resultadoJSON;
        resultadoJSON["archivo"] = resultado.archivo;
        resultadoJSON["repeticion"] = resultado.repeticion;
        resultadosJSON.push_back(resultadoJSON);
    }
    jsonResponse["contexto"]["resultados"] = resultadosJSON;
    string serializedResponse = jsonResponse.dump();
    send(clientSocket, serializedResponse.c_str(), serializedResponse.length(), 0);
}

void SendBackendToFrontend(int frontend, const string& mensaje) {
    send(frontend, mensaje.c_str(), mensaje.length(), 0);
}

void sendBackend(int backend, const json& message){
    string serializedMessage = message.dump();
    send(backend, serializedMessage.c_str(),serializedMessage.length(), 0);
}

string receiveMessageToBackend(int backendSocket) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    int bytesRead = recv(backendSocket, buffer, sizeof(buffer), 0);

    if (bytesRead == -1) {
        perror("Error al recibir datos del backend");
        return "";
    } else if (bytesRead == 0) {
        cout << "El backend ha cerrado la conexión" << endl;
        return "";
    } else {
        buffer[bytesRead] = '\0';
        return string(buffer);
    }
}
void procesarMensaje(int clientSocket, const char *mensaje, int clientSocketToOtherServer, map<string, vector<SearchResult>>& memoriaCache) {
    cout << "Datos recibidos del cliente: " << mensaje << endl;
    string mensajeStr(mensaje);
    try {
        json j = json::parse(mensajeStr);
        json contexto = j["contexto"];  // Obtener "contexto" como un objeto JSON
        string textoABuscar = contexto["txtToSearch"];  // Corregir el nombre de la clave


        auto it = memoriaCache.find(textoABuscar);
        if (it != memoriaCache.end()) {
            vector<SearchResult>& resultados = it->second;
            sendMemcacheToFrontend(clientSocket, resultados, true);
        } else {
            cout << "Palabra no encontrada en memoria cache, enviando mensaje al backend" << endl;
            json mensajeBackend = {
                {"origen", string(getenv("HOST"))},
                {"destino", string(getenv("BACK"))},
                {"contexto", {{"txtToSearch", textoABuscar}
                }}
            };

            sendBackend(clientSocketToOtherServer, mensajeBackend);
            string respuestaBackend = receiveMessageToBackend(clientSocketToOtherServer);
            //agrega a memoria cache
            json resultadosJson = json::parse(respuestaBackend);
            agregarResultadosAlMapa(resultadosJson, textoABuscar);

            // Después de recibir y procesar los resultados del memcache
            cout <<"respuesta del backend:"<<respuestaBackend<<endl;
            SendBackendToFrontend(clientSocket, respuestaBackend);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error al analizar el mensaje JSON: " << e.what() << std::endl;
    }
}



int main(){
    dotenv::init();
    //deficion de variables
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    SearchResult resultadoEjemplo;
    SearchResult nuevoResultado;
    resultadoEjemplo.archivo = "ejemplo.txt";
    resultadoEjemplo.repeticion = 3;     
    memoriaCache["hola"].push_back(resultadoEjemplo);
    nuevoResultado.archivo = "otro_ejemplo.txt";
    nuevoResultado.repeticion = 5;
    memoriaCache["hola"].push_back(nuevoResultado);
    
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
        }
    }
    return 0;
}   
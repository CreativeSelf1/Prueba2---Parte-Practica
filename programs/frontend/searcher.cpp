#include <iostream>
#include <limits>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "include/laserpants/dotenv/dotenv.h"
#include "include/json.hpp"

using namespace std;
using json = nlohmann::json;

void receiveMessage(int clientSocket) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));  // Limpiar el buffer antes de recibir
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);

    if (bytesRead == -1) {
        perror("Error al recibir datos del servidor");
        return;
    } else if (bytesRead == 0) {
        std::cerr << "El servidor ha cerrado la conexión" << std::endl;
        return;
    } else {
        buffer[bytesRead] = '\0';

        json jsonResponse = json::parse(buffer);

        // Imprimir la información del archivo y repetición
        cout << "\nRespuesta(tiempo="<<", origen="<< jsonResponse["contexto"]["ori"]<<"):\n" << endl;
        for (const auto& resultado : jsonResponse["contexto"]["resultados"]) {
            cout << "    "<<resultado["archivo"] << "\", " << resultado["repeticion"] << endl;
        }
        
    }
}


// Función para conectar al servidor
int connectToServer(const string& serverIP, int serverPort) {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Error al crear el socket del cliente");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = inet_addr(serverIP.c_str());

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Error al conectar al servidor");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }
    cout << "Conectado al servidor." << endl;
    return clientSocket;
}

void sendMessage(int clientSocket, const json& message){
    string serializedMessage = message.dump();
    send(clientSocket, serializedMessage.c_str(),serializedMessage.length(), 0);
}
int main(){
    dotenv::init();
    string serverIP = "127.0.0.1"; // Dirección IP del servidor
    int serverPort = 3001;       // Puerto del servidor
    int clientSocket = connectToServer(serverIP, serverPort);
    //interfazBuscador(clientSocket);
    while (true){
        string textoBuscar;
        char salida;
        int systemResult = system("clear"); 
        if (systemResult == -1) {
            perror("Error al ejecutar el comando 'clear'");
        }
        cout << "BUSCADOR BASADO EN INDICE INVERTIDO\n";
        cout << "\nLos top K documentos serán: "<<endl;
        cout << "\nEscriba texto a buscar: ";
        getline(cin, textoBuscar);
        json message = {
            {"origen", string(getenv("FROM"))},
            {"destino", string(getenv("TO"))},
            {"contexto", {{"txtToSearch", textoBuscar}}}
        };
        sendMessage(clientSocket, message);

        //respuestas memcache
        receiveMessage(clientSocket);
        //cout << "Respuesta " << respuestaMemcache << endl;

        cout << "\n DESEA SALIR (S/N): ";
        cin >> salida;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (salida == 'S' || salida == 's'){
            close(clientSocket);
            break;
        }
    }
    close(clientSocket);

  
    return 0;
}   
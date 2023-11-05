#include <iostream>
#include <limits>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "include/laserpants/dotenv/dotenv.h"


using namespace std;

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

void interfazBuscador(int clientSocket){
    size_t pid = getpid();
    string textoBuscar;
    char salida;

    int systemResult = system("clear"); 
    if (systemResult == -1) {
        perror("Error al ejecutar el comando 'clear'");
    }
    cout << "BUSCADOR BASADO EN INDICE INVERTIDO "<<"("<<pid<<")\n";
    cout << "\nLos top K documentos serán: "<<endl;
    cout << "\nEscriba texto a buscar: ";
    getline(cin, textoBuscar);


    //funciones para envio de mensaje a al servidor memcache



    //salida
    cout << "\n   DESEA SALIR (S/N): ";
    cin >> salida;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (salida == 'S' || salida == 's') {
       return;
    }

    if (salida == 'N' || salida == 'n') {
        interfazBuscador(clientSocket);
    }
    else{
        cout << "Ingrese una opcion Correcta"<<endl;
        interfazBuscador(clientSocket);
    }  
}

int main(){
    dotenv::init();
    string serverIP = "127.0.0.1"; // Dirección IP del servidor
    int serverPort = 3001;       // Puerto del servidor
    int clientSocket = connectToServer(serverIP, serverPort);
    interfazBuscador(clientSocket);
    
   
    return 0;
}   
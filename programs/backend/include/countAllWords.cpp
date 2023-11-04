#include <fstream>
#include <iostream>
#include <unordered_map>
#include <string> 
#include <algorithm>
#include <iomanip>
#include <unistd.h> 
#include <filesystem> 

#include "countAllWords.h"

namespace fs = std::filesystem;
using namespace std;

void contarTodasLasPalabras(string archivo, string salida, string extension) {

    ifstream inputFile(archivo);
    if (!inputFile.is_open()) {
        cerr << "No se pudo abrir el archivo: " << archivo << endl;
        return;
    }
    unordered_map<string, int> conteo;
    string palabra;
    while (inputFile >> palabra) {
        palabra.erase(remove_if(palabra.begin(), palabra.end(), ::ispunct), palabra.end());
        transform(palabra.begin(), palabra.end(), palabra.begin(), ::tolower);
        conteo[palabra]++;
    }
    inputFile.close();

    size_t lastDotPos = archivo.find_last_of(".");
    string nombreArchivoSinExtension = archivo.substr(archivo.find_last_of("/\\") + 1, lastDotPos - archivo.find_last_of("/\\") - 1);

    string nombreArchivoSalida = salida+"/" + nombreArchivoSinExtension + "."+ extension;
    ofstream outputFile(nombreArchivoSalida);
    
    for (const auto& palabra : conteo) {
        outputFile << palabra.first << "," << palabra.second << endl;
    }
    outputFile.close();
}

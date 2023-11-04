#include "loads.h"
#include <mutex>
#include <thread>
#include <vector>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>
#include "countAllWords.h"

using namespace std;
namespace fs = std::filesystem;

mutex mtx;

void procesarArchivo(string ruta_entrada, string ruta_salida, queue<string>& archivos, int id, string extension) {
    while (true) {
        mtx.lock();
        if (archivos.empty()) {
            mtx.unlock();
            return;
        }
    
        string archivo = archivos.front();
        archivos.pop();
        mtx.unlock();
        contarTodasLasPalabras(archivo,ruta_salida,extension );
    }
}

void write_document(string idx, unordered_map<string, vector<pair<string, int>>>& indice){
    ofstream archivoSalida(idx);
    if (!archivoSalida.is_open()) {
        cerr << "Error: No se pudo abrir el archivo de salida '" << idx << "'" << endl;
        return;
    }
    for (const auto& entrada : indice) {
        archivoSalida << entrada.first << ":";
        size_t numDocumentos = entrada.second.size();
        for (size_t i = 0; i < numDocumentos; ++i) {
            archivoSalida <<"(" << entrada.second[i].first << ";" << entrada.second[i].second << ")";
            if (i < numDocumentos - 1) {
            archivoSalida << ";";
        }
        } 
        archivoSalida << endl;
    }
} 

void writeIDX(string idx, string files_out){
    unordered_map<string, vector<pair<string, int>>> indice;

    for (const auto& archivo : fs::directory_iterator(files_out)) {
        string nombreArchivo = archivo.path().filename().string();
        ifstream archivoEntrada(archivo.path());
        string linea;
        while (getline(archivoEntrada, linea)) {
            istringstream iss(linea);
            string palabra;
            int cantidad;

            if (getline(iss, palabra, ',') && (iss >> cantidad)) {
             indice[palabra].push_back({nombreArchivo, cantidad});
            } 
        }
        archivoEntrada.close();
    }
    write_document(idx, indice);
}

void generateFILEIDX(string extention, string files_in, string files_out, int amount_threads, string idx){
    queue<string> archivos;
    vector<thread> threads;
    mutex mtx;

    for (const auto& archivo : fs::directory_iterator(files_in)) {
        if (archivo.is_regular_file() && archivo.path().extension() == "."+extention) {
            archivos.push(archivo.path().string());
        }
    }
   
    if (archivos.size() <20){
        cout << "Error, la cantidad de archivo es menor a 20" << endl;
        return;
    }

     for (int i = 0; i < amount_threads; ++i) {
        threads.emplace_back(procesarArchivo, files_in, files_out, ref(archivos), i,extention);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    threads.clear();

    writeIDX(idx, files_out);


    
}

unordered_map<string, vector<pair<string, int>>> mapIDX(string idx) {
    unordered_map<string, vector<pair<string, int>>> mapIDX;
    ifstream archivo(idx);
    string linea;
    while (getline(archivo, linea)) {
        linea.erase(remove_if(linea.begin(), linea.end(), ::isspace), linea.end());
        istringstream iss(linea);
        string palabra;
        string restoDeLaLinea;
        getline(iss, palabra, ':');
        getline(iss, restoDeLaLinea);

        size_t found = restoDeLaLinea.find_first_of("()");
        istringstream pairStream(restoDeLaLinea);
        string file;
        int cantidad;
        vector<pair<string, int>> pares;

        found = restoDeLaLinea.find_first_of("()");
        while (found != string::npos) {
            size_t end = restoDeLaLinea.find_first_of(")", found + 1);
            string subPair = restoDeLaLinea.substr(found + 1, end - found - 1);
            istringstream subPairStream(subPair);
            getline(subPairStream, file, ';');
            subPairStream >> cantidad;
            pares.emplace_back(file, cantidad);
            found = restoDeLaLinea.find_first_of("()", end + 1);
        }
        mapIDX[palabra]=pares;
    }
    archivo.close();
    return mapIDX;
}
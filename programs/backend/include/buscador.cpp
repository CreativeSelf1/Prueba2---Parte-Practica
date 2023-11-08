#include "buscador.h"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <iostream>
#include <map>
using namespace std;

vector<string> buscarPalabras(string textoBuscar, unordered_map<string, vector<pair<string, int>>>& UmapIDX) {
    vector<string> palabrasEncontradas;
    unordered_set<string> palabrasEncontradasSet; 

    istringstream palabraStream(textoBuscar);
    string palabra;
    while (palabraStream >> palabra) {
        // Buscar la palabra en el índice invertido
        auto it = UmapIDX.find(palabra);
        if (it != UmapIDX.end() ) {
            palabrasEncontradas.push_back(palabra);
           // palabrasEncontradasSet.insert(palabra);
        }
    }
    for (const auto& palabra : palabrasEncontradas) {
        cout << palabra << endl;
    }
    return palabrasEncontradas;
}

void CantidadPalabrasArchivo(unordered_map<string, vector<pair<string, int>>>& UmapIDX, vector<string>& palabrasEncotradas, int TOPK) {
    vector<pair<string, int>> cantidadPalabrasArchivo;
    unordered_map<string, int> sumaPorArchivo;

    for (const auto& palabra : palabrasEncotradas) {
        if (UmapIDX.find(palabra) != UmapIDX.end()) {
            cantidadPalabrasArchivo.insert(cantidadPalabrasArchivo.end(),UmapIDX[palabra].begin(),UmapIDX[palabra].end());
        }
    }

    for (const auto& par : cantidadPalabrasArchivo) {
        sumaPorArchivo[par.first] += par.second;
    }

    multimap<int, string, greater<int>> multimapOrdenado;
    for (const auto& par : sumaPorArchivo) {
        multimapOrdenado.insert({par.second, par.first});
    }

    int count = 0;
    for (const auto& par : multimapOrdenado) {
        cout <<"   "<<count + 1<<") "<< par.second << ": " << par.first << endl;
        ++count;
        if (count == TOPK) {
            break;
        }
    }
}

json obtenerJsonCantidadPalabrasArchivo(unordered_map<string, vector<pair<string, int>>>& UmapIDX, vector<string>& palabrasEncontradas, int TOPK) {
    json jsonResponse;
    jsonResponse["origen"] = "./backend";
    jsonResponse["destino"] = "./memcache";
    jsonResponse["contexto"]["tiempo"] = "100ns";
    jsonResponse["contexto"]["ori"] = "BACKEND";

    vector<json> archivosConRepeticiones;

    for (const auto& palabra : palabrasEncontradas) {
        if (UmapIDX.find(palabra) != UmapIDX.end()) {
            for (const auto& par : UmapIDX[palabra]) {
                json archivoRepeticion;
                archivoRepeticion["archivo"] = par.first;
                archivoRepeticion["repeticion"] = par.second;
                archivosConRepeticiones.push_back(archivoRepeticion);
            }
        }
    }

    // Ordenar el vector de archivosConRepeticiones por repeticiones en orden descendente
    sort(archivosConRepeticiones.begin(), archivosConRepeticiones.end(),
         [](const json& a, const json& b) {
             return a["repeticion"].get<int>() > b["repeticion"].get<int>();
         });

    // Limitar a TOPK resultados
    // Limitar a TOPK resultados
    if (archivosConRepeticiones.size() > static_cast<size_t>(TOPK)) {
        archivosConRepeticiones.resize(static_cast<size_t>(TOPK));
    }

    jsonResponse["contexto"]["resultados"] = archivosConRepeticiones;

    return jsonResponse;
}


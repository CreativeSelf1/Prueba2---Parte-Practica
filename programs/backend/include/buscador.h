#ifndef BUSCADOR_H
#define BUSCADOR_H

#include <vector>
#include <unordered_map>
#include <string>
#include "json.hpp"
using namespace std;
using json = nlohmann::json;

vector<string> buscarPalabras(string textoBuscar, unordered_map<string, vector<pair<string, int>>>& UmapIDX);
void CantidadPalabrasArchivo(unordered_map<string, vector<pair<string, int>>>& UmapIDX, vector<string>& palabrasEncotradas, int TOPK);
json obtenerJsonCantidadPalabrasArchivo(unordered_map<string, vector<pair<string, int>>>& UmapIDX, vector<string>& palabrasEncontradas, int TOPK);
#endif 
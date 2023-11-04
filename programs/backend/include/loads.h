#ifndef LOADS_H
#define LOADS_H
#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

void generateFILEIDX(string extention, string files_in, string files_out, int amount_threads, string idx);
unordered_map<string, vector<pair<string, int>>> mapIDX(string idx);


#endif 
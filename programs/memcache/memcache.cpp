#include <iostream>
#include "include/laserpants/dotenv/dotenv.h"

using namespace std;

int main(){
    dotenv::init();
    string ruta_entrada = string(getenv("BACK"));
    cout << ruta_entrada<<endl;
    return 0;
}   
#include <iostream>
#include <string>
#include "server.h"
using namespace::std;
int main(int argc, char * argv[]){
    cout << "server starts" << endl;
    Server new_server(8099,200);
    new_server.init();
    new_server.starts();
    cout << "server ends" << endl;
    return 0;
}

#include "DataBuffer.hpp"

int main(int argc, char* argv[]){

    if(argc != 2){
        return -1;
    }

    DataBuffer db(argv[1]);

    return 0;
}

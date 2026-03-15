#include "DataBuffer.hpp"

int main(int argc, char* argv[]){

    if(argc != 2){
        return -1;
    }

    DataBuffer db(argv[1]);

//    db.dump_bits(std::cout);
//    std::cout << std::endl;

    return 0;
}

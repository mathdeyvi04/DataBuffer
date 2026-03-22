#include "DataBuffer.hpp"

int main(int argc, char* argv[]){

    if(argc != 2){
        return -1;
    }

    DataBuffer db(argv[1]);
    db.dump_bits<true>(std::cout);
    std::cout << "----" << std::endl;
    for(size_t i = 0; i < 25; ++i){ // Faremos os 5 primeiros serem 1
        db.set_bit(i, true);
    }
    db.dump_bits<true>(std::cout);
    std::cout << std::endl;

    return 0;
}

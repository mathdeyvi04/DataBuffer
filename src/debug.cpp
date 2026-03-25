#include "DataBuffer.hpp"

int main(int argc, char* argv[]){

    if(argc != 2){
        return -1;
    }

    DataBuffer db(argv[1]);
    DataBuffer db_(argv[1]);
    db_.set_bit(0, 1);
    db_.set_bit(1, 0);

    std::cout << DataBuffer::get_hamming_distance(db, db_) << std::endl;

    return 0;
}

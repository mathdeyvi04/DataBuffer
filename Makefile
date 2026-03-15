all:
	g++ -g3 -O0 -Wall -fno-inline -march=native -DENABLE_BINARY_LOOKTABLE src/debug.cpp -o src/debug; \
	gdb --args ./src/debug src/filetest.txt;

real:
	g++ -O3 -march=native src/debug.cpp -o src/main; \
	./src/main src/filetest.txt

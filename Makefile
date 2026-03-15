all:
	g++ -g3 -O0 -Wall -fno-inline -march=native src/debug.cpp -o src/debug; \
	gdb --args ./src/debug src/filetest.txt; \
	rm src/debug;

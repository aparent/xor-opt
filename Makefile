all: xor-opt

xor-opt: main.cpp model.h
	g++ -g -o2 -std=c++14 -Wall -Wl,-rpath -Wl,/usr/local/lib -I/usr/local/include -L/usr/local/lib main.cpp -o xor-opt -ligraph

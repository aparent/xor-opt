all: xor-opt

xor-opt: main.cpp model.h
	clang++ -std=c++0x -I/opt/local/include -L/opt/local/lib main.cpp -o xor-opt

all: xor-opt

xor-opt: main.cpp model.h
	clang++ -o2 -std=c++14 -I/opt/local/include -L/opt/local/lib main.cpp -o xor-opt -ligraph

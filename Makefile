all: main run
main: main.cpp
	g++ -o main -fopenmp -std=c++11 -O3 main.cpp

run:
	./main
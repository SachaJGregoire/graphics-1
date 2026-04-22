all: main run
main: main.cpp
	g++ -o main -fopenmp -std=c++11 -o3 main.cpp

run:
	./main
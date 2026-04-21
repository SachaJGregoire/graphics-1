all: main run
main: main.cpp
	g++ -o main -fopenmp main.cpp

run:
	./main
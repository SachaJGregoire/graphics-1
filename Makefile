all: main run
main: main.cpp
	g++ -o main main.cpp

run:
	./main
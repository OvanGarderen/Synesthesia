
undulate: undulate.cpp imgtools.a
	g++ -g -o undulate glad.c undulate.cpp imgtools.a -O2 -ldl -lglfw `pkg-config --libs opencv`

imgtools.a: imgtools.o
	ar rvs imgtools.a imgtools.o

imgtools.o: imgtools.cpp
	g++ -c imgtools.cpp -o imgtools.o -Wall

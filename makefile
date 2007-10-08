prog:	calc.o xbmath.o
	g++ calc.o xbmath.o -o calc
    


calc.o: calc.cpp
	g++ -c calc.cpp -o calc.o

xbmath.o: xbmath.cpp
	g++ -c xbmath.cpp -o xbmath.o

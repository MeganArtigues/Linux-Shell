all: cd 1730sh pipe2

run: cd 1730sh pipe2
	./cd ./1730sh ./pipe2

cd: cd.o
	g++ -Wall -std=c++14 -g -O0 -pedantic-errors -o cd cd.o
cd.o: cd.cpp
	g++ -Wall -std=c++14 -c -g -O0 -pedantic-errors cd.cpp


1730sh: 1730sh.o
	g++ -Wall -std=c++14 -g -O0 -pedantic-errors -o 1730sh 1730sh.o
1730sh.o: 1730sh.cpp
	g++ -Wall -std=c++14 -c -g -O0 -pedantic-errors 1730sh.cpp

pipe2: pipe2.o
	g++ -Wall -std=c++14 -g -O0 -pedantic-errors -o pipe2 pipe2.o
pipe2.o: pipe2.cpp
	g++ -Wall -std=c++14 -c -g -O0 -pedantic-errors pipe2.cpp

clean:
	rm -f cd
	rm -f 1730sh
	rm -f *.o

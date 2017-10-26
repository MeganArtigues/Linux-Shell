all: cd shell pipe2

run: cd shell pipe2
	./cd ./shell ./pipe2

cd: cd.o
	g++ -Wall -std=c++14 -g -O0 -pedantic-errors -o cd cd.o
cd.o: cd.cpp
	g++ -Wall -std=c++14 -c -g -O0 -pedantic-errors cd.cpp


shell: shell.o
	g++ -Wall -std=c++14 -g -O0 -pedantic-errors -o shell shell.o
shell.o: shell.cpp
	g++ -Wall -std=c++14 -c -g -O0 -pedantic-errors shell.cpp

pipe2: pipe2.o
	g++ -Wall -std=c++14 -g -O0 -pedantic-errors -o pipe2 pipe2.o
pipe2.o: pipe2.cpp
	g++ -Wall -std=c++14 -c -g -O0 -pedantic-errors pipe2.cpp

clean:
	rm -f cd
	rm -f shell
	rm -f *.o

OPT = -O3 -I. -ITdZdd/include

main: main.cpp tdzdd_converter.hpp
	g++ $(OPT) main.cpp -o converter

clean:
	rm -rf *.o

all: test.e

test.e:
	flex -o generated_scanner.out.c flex-scanner.l
	bison tm-parser.yy -o tm-parser.out.cc
	g++ -o test.e test.cc generated_scanner.out.c tm-parser.out.cc tm-driver.cc -std=c++11

clean:
	rm *.e *.out.c *.out.cc *.out.hh location.hh position.hh stack.hh 2> /dev/null

all: run.e

run.e:
	flex -o generated_scanner.out.c flex-scanner.l
	bison mm4-parser.yy -o mm4-parser.out.cc
	g++ -o run.e test.cc ast.cc translate.cc generated_scanner.out.c mm4-parser.out.cc mm4-driver.cc ../shared_files/bignum.cc -std=c++11

clean:
	rm *.e *.out.c *.out.cc *.out.hh location.hh position.hh stack.hh 2> /dev/null

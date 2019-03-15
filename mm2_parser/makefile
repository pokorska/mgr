all: test.e

test.e:
	flex -o generated_scanner.out.c flex-scanner.l
	bison mm2-parser.yy -o mm2-parser.out.cc
	g++ -o test.e test.cc ast.cc generated_scanner.out.c mm2-parser.out.cc mm2-driver.cc ../shared_files/bignum.cc -std=c++11

clean:
	rm *.e *.out.c *.out.cc *.out.hh location.hh position.hh stack.hh 2> /dev/null

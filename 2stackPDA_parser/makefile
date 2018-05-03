all:
	flex -o generated_scanner.out.c flex-scanner.l
	bison 2stackPDA-parser.yy -o 2stackPDA-parser.out.cc
	g++ -o test.e test.cc generated_scanner.out.c 2stackPDA-parser.out.cc 2stackPDA-driver.cc ast.cc -std=c++11

clean:
	rm *.e *.out.c *.out.cc *.out.hh location.hh position.hh stack.hh

all:
	flex -o generated_scanner.out.c flex-scanner.l
	bison cm-parser.yy -o cm-parser.out.cc
	g++ -o test.e test.cc generated_scanner.out.c cm-parser.out.cc cm-driver.cc -std=c++11

clean:
	rm *.e *.out.c *.out.cc *.out.hh location.hh position.hh stack.hh

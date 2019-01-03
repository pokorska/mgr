all: test.e

test.e: names_manip.o translation.o ast.o parser.o lexer.o driver.o test.o
	g++ -o test.e names_manip.o translation.o ast.o lexer.o parser.o driver.o test.o -std=c++11

names_manip.o:
	g++ -o names_manip.o -c names_manip.cc -std=c++11

translation.o:
	g++ -o translation.o -c translation.cc -std=c++11

ast.o:
	g++ -o ast.o -c ast.cc -std=c++11

lexer.o: parser.o
	flex -o generated_scanner.out.c flex-scanner.l
	g++ -o lexer.o -c generated_scanner.out.c -std=c++11

parser.o:
	bison 2stackPDA-parser.yy -o 2stackPDA-parser.out.cc
	g++ -o parser.o -c 2stackPDA-parser.out.cc -std=c++11

driver.o:
	g++ -o driver.o -c 2stackPDA-driver.cc -std=c++11

test.o:
	g++ -o test.o -c test.cc -std=c++11

clean:
	rm *.e *.out.c *.out.cc *.out.hh *.o location.hh position.hh stack.hh 2> /dev/null

all: run.e

run.e:
	flex -o generated_scanner.out.c flex-scanner.l
	bison bf-parser.yy -o bf-parser.out.cc
	g++ -o run.e test.cc generated_scanner.out.c bf-parser.out.cc bf-driver.cc -std=c++11

clean:
	rm *.e *.out.c *.out.cc *.out.hh location.hh position.hh stack.hh 2> /dev/null

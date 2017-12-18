all:
	flex -o generated_scanner.c flex-scanner.l
	bison calc++-parser.yy
	g++ -o test test.cc generated_scanner.c calc++-parser.tab.cc calc++-driver.cc -std=c++11

clean:
	rm test generated_scanner.c location.hh position.hh stack.hh calc++-parser.tab.cc calc++-parser.tab.hh

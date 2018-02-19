all:
	flex -o generated_scanner.c flex-scanner.l
	bison bf-parser.yy
	g++ -o test test.cc generated_scanner.c bf-parser.tab.cc bf-driver.cc -std=c++11

clean:
	rm test generated_scanner.c location.hh position.hh stack.hh bf-parser.tab.cc bf-parser.tab.hh

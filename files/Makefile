CXX   = g++-8
EXE = iplC
CXXDEBUG = -g -Wall
CXXSTD = -std=c++11


.PHONY: all
all: parser lexer 	
	$(CXX) $(CXXSTD) -o iplC driver.cpp parser.o scanner.o 

parser: parser.yy scanner.hh
	bison -d -v $<
	$(CXX) $(CXXSTD) -c parser.tab.cc -o parser.o 

lexer: scanner.l scanner.hh parser.tab.hh parser.tab.cc	
	flex++ --outfile=scanner.yy.cc  $<
	$(CXX) $(CXXSTD) -c scanner.yy.cc -o scanner.o

clean: 
	rm *.o location.hh iplC parser.output parser.tab.cc parser.tab.hh position.hh scanner.yy.cc stack.hh

CC=gcc-4.4
CXX=g++-4.4
LIDIA=/home/varun/git/lidia-2.3.0/
test:	test.cpp Key.o
	$(CXX) -o test test.cpp Key.o -L$(LIDIA)/library/ -lm -lLiDIA -lgmp
Key.o:	Key.cpp Key.h Invert.cpp
	$(CXX) -c -o Key.o Key.cpp


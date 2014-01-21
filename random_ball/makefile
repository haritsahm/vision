all: basic

basic: basic.cpp
	g++ -o basic basic.cpp `pkg-config --libs opencv cvblob` -lueye_api

clean:
	rm -rf basic
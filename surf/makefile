all: file1

file1: 
	g++ -o surf surf.cpp `pkg-config --libs opencv cvblob` -lueye_api

clean: 
	rm -f surf
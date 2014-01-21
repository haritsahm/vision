all: basic ellipse

basic: basic.cpp
	g++ -o basic basic.cpp `pkg-config --libs opencv cvblob` -lueye_api

ellipse: ellipse.cpp
	g++ -o ellipse ellipse.cpp `pkg-config --libs opencv cvblob` -lueye_api

clean:
	rm -rf basic ellipse
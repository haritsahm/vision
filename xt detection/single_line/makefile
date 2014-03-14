all: single_line

single_line: single_line.cpp
	g++ -fpermissive -o single_line single_line.cpp `pkg-config --libs opencv cvblob` -lueye_api -lm

clean:
	rm -rf single_line

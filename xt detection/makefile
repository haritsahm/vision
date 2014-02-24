all: pgm_converter

pgm_converter: pgm_converter.cpp
	g++ -o pgm_converter pgm_converter.cpp `pkg-config --libs opencv`

clean:
	rm -rf pgm_converter
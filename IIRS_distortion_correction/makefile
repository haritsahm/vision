all: driver color lines

driver: driver.cpp
	g++ -o driver driver.cpp `pkg-config --libs opencv cvblob`

color: color.cpp
	g++ -o color color.cpp `pkg-config --libs opencv`

lines: lines.cpp
	g++ -o lines lines.cpp `pkg-config --libs opencv`

clean:
	rm -rf driver
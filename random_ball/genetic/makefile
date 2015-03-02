all: balldetector driver

balldetector: balldetector.h balldetector.cpp
	g++ -c balldetector.cpp -lueye_api

driver: balldetector driver.cpp
	g++ -o driver driver.cpp balldetector.o `pkg-config --libs opencv` -lueye_api

clean:
	rm -rf balldetector
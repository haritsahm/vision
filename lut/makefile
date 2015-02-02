all: lut_hsv lutle

lutle:
	g++ -o lutle lutle.cpp

lut_hsv: lut_hsv.cpp
	g++ -o lut_hsv lut_hsv.cpp `pkg-config --libs opencv` -lueye_api

clean:
	rm -rf lut_hsv
	rm -rf lutle
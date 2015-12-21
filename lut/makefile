all: lut_hsv lut_maker camcapture lutle

lut_maker: camcapture lut_maker.cpp
	g++ -o lut_maker lut_maker.cpp camcapture.o `pkg-config --libs opencv` -lueye_api

camcapture: camcapture.h camcapture.cpp
	g++ -c camcapture.cpp

lut_hsv: lut_hsv.cpp
	g++ -o lut_hsv lut_hsv.cpp `pkg-config --libs opencv` -lueye_api

lutle: lutle.cpp
	g++ -o lutle lutle.cpp

clean:
	rm -rf lut_hsv
	rm -rf lut_maker
	rm -rf lutle
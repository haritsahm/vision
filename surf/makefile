all: surf surf_oldcamera

surf: 
	g++ -o surf surf.cpp `pkg-config --libs opencv cvblob` -lueye_api

surf_oldcamera:
	g++ -o surf_oldcamera surf_oldcamera.cpp `pkg-config --libs opencv cvblob` -lflycapture

clean: 
	rm -f surf surf_oldcamera
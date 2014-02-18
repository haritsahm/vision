all: calculate_alpha picture_crosshairs pixel_value
	
calculate_alpha: calculate_alpha.cpp
	g++ -o calculate_alpha calculate_alpha.cpp `pkg-config --libs opencv cvblob` -lm

picture_crosshairs: picture_crosshairs.cpp
	g++ -o picture_crosshairs picture_crosshairs.cpp `pkg-config --libs opencv cvblob` -lueye_api

pixel_value: pixel_value.cpp
	g++ -o pixel_value pixel_value.cpp `pkg-config --libs opencv cvblob`

clean:
	rm -rf calculate_alpha
	rm -rf picture_crosshairs
	rm -rf pixel_value
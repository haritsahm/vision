all: coords take_picture undistort

coords: coords.cpp
	@echo building coords
	@g++ -o coords coords.cpp `pkg-config --libs opencv cvblob` -lueye_api

take_picture: take_picture.cpp
	@echo building take_picture
	@g++ -o take_picture take_picture.cpp `pkg-config --libs opencv cvblob` -lueye_api

undistort: undistort.cpp
	@echo building undistort
	@g++ -o undistort undistort.cpp `pkg-config --libs opencv cvblob` -lueye_api

clean:
	@echo cleaning
	@rm -rf coords
	@rm -rf take_picture
	@rm -rf undistort
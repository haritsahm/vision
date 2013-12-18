all: take_pic facedetect

take_pic: 
	g++ -o take_pic take_pic.cpp `pkg-config --libs opencv cvblob` -lueye_api

facedetect: 
	g++ -o facedetect facedetect.cpp `pkg-config --libs opencv` -lueye_api

clean:
	@echo -removing take_pic
	rm -f take_pic facedetect
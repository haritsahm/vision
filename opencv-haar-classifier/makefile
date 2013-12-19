all: take_pic facedetect detect_xt

take_pic: 
	g++ -o take_pic take_pic.cpp `pkg-config --libs opencv cvblob` -lueye_api

facedetect: 
	g++ -o facedetect facedetect.cpp `pkg-config --libs opencv` -lueye_api

detect_xt:
	g++ -o detect_xt detect_xt.cpp `pkg-config --libs opencv` -lueye_api

clean:
	rm -f take_pic facedetect detect_xt
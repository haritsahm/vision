all: take_pic

take_pic: 
	g++ -o take_pic take_pic.cpp `pkg-config --libs opencv cvblob` -lueye_api

clean:
	rm -f take_pic
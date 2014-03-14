all: calculate_constants_objdis calculate_constants_perpend take_picture
# g++ -o average average_constants.cpp -lm
calculate_constants_objdis: calculate_constants_objdis.cpp
	g++ -o calculate_constants_objdis calculate_constants_objdis.cpp `pkg-config --libs opencv cvblob` -lm
calculate_constants_perpend: calculate_constants_perpend.cpp
	g++ -o calculate_constants_perpend calculate_constants_perpend.cpp `pkg-config --libs opencv cvblob` -lm
# g++ -o linear_interpolation_objdis linear_interpolation_objdis.cpp -lm
# g++ -o linear_interpolation_perpend linear_interpolation_perpend.cpp -lm
take_picture: take_picture.cpp
	g++ -o take_picture take_picture.cpp `pkg-config --libs opencv cvblob` -lueye_api -lftdi
clean:
	rm -rf calculate_constants_perpend
	rm -rf calculate_constants_objdis
	rm -rf take_picture
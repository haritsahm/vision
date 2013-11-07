all:
g++ -o average average_constants.cpp -lm
g++ -o calculate_constants_objdis calculate_constants_objdis.cpp `pkg-config --libs opencv cvblob` -lm
g++ -o calculate_constants_perpend calculate_constants_perpend.cpp `pkg-config --libs opencv cvblob` -lm
g++ -o linear_interpolation_objdis linear_interpolation_objdis.cpp -lm
g++ -o linear_interpolation_perpend linear_interpolation_perpend.cpp -lm
g++ -o take_picture take_picture.cpp `pkg-config --libs opencv cvblob` -lueye_api -lftdi
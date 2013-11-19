all: imu.cpp imu.h totalxsens.o calibrate_yaw yaw_values test
	g++ -c imu.cpp
calibrate_yaw: calibrate_yaw.cpp
	g++ imu.o totalxsens.o calibrate_yaw.cpp -o calibrate_yaw -lrt -lftdi -Wformat
yaw_values: yaw_values.cpp
	g++ imu.o totalxsens.o yaw_values.cpp -o yaw_values -lrt -lftdi -Wformat
test: test.cpp
	g++ imu.o totalxsens.o test.cpp -o test -lrt -Wformat
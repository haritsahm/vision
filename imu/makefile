all: imu.cpp imu.h totalxsens.o calibrate_yaw yaw_values
	g++ -c imu.cpp
calibrate_yaw: calibrate_yaw.cpp
	g++ xsens/imu.o xsens/totalxsens.o calibrate_yaw.cpp -o calibrate_yaw -lrt -lftdi
yaw_values: yaw_values.cpp
	g++ xsens/imu.o xsens/totalxsens.o yaw_values.cpp -o yaw_values -lrt -lftdi
all: guohallthinning_test zhangsuenthinning_test image_zhangsuenthinning

guohallthinning_test: guohallthinning_test.cpp
	g++ -o guohallthinning_test guohallthinning_test.cpp `pkg-config --libs opencv`

zhangsuenthinning_test: zhangsuenthinning_test.cpp
	g++ -o zhangsuenthinning_test zhangsuenthinning_test.cpp `pkg-config --libs opencv`

image_zhangsuenthinning: image_zhangsuenthinning.cpp
	g++ -o image_zhangsuenthinning image_zhangsuenthinning.cpp `pkg-config --libs opencv`

clean:
	rm -rf guohallthinning_test
	rm -rf zhangsuenthinning_test
	rm -rf image_zhangsuenthinning
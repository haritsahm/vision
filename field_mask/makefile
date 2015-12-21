all: generate_mask

generate_mask: generate_mask.cpp
	g++ -o generate_mask generate_mask.cpp `pkg-config --libs opencv` -lueye_api

clean:
	rm -rf generate_mask
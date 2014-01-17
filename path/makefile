all: ../Build/path.a

../Build/path.a: ../Build/path.o
	@echo $@
	@ar rc $@ ../Build/path.o
	@echo
	@ranlib $@

../Build/path.o: path.cpp path.hpp graph.hpp
	@echo $@
	@if !(test -d ../Build); then mkdir ../Build; fi
	@g++ -c path.cpp  -o $@

path: path.o path.hpp path.cpp graph.hpp
	g++ path.o testpath.cpp -o testpath -lopencv_highgui -lopencv_core -lopencv_imgproc

path.o:path.hpp path.cpp graph.hpp
	g++ -c path.cpp

clean:
	@rm -f path.o 
	@rm -f testpath
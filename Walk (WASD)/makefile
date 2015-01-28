all: gui.o gui_thread.o
	g++ -o run -w run.cpp gui.o gui_thread.o -lopencv_highgui -lpthread -lopencv_core -lopencv_imgproc

gui.o: gui.h gui.cpp common.h
	g++ -c gui.cpp

gui_thread.o: gui_thread.h gui_thread.cpp common.h
	g++ -c gui_thread.cpp

clean:
	@rm -rf gui.o
	@rm -rf gui_thread.o
	@rm -rf run
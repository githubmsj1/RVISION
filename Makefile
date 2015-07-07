vision:vision.o serial.o track.o
	g++ -o vision vision.o serial.o track.o `pkg-config --cflags --libs opencv`
vision.o:vision.cpp vision.h serial.h
	g++ -c vision.cpp 
track.o:track.cpp vision.h
	g++ -c track.cpp
serial.o:serial.cpp serial.h
	g++ -c serial.cpp
clear:
	rm *.o

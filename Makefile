vision:vision.o
	g++ -o vision vision.o `pkg-config --cflags --libs opencv`
vision.o:vision.cpp vision.h
	g++ -c vision.cpp 

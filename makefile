all: opencv0 opencv1 squares polys demhist houghlines

INCLUDE=/usr/local/include/opencv
LIBS=-lcv -lhighgui

opencv0: opencv0.cc
	g++ -I$(INCLUDE) $(LIBS) opencv0.cc -o opencv0

opencv1: opencv1.cc
	g++ -I$(INCLUDE) $(LIBS) opencv1.cc -o opencv1

squares: squares.cc
	g++ -I$(INCLUDE) $(LIBS) squares.cc -o squares

polys.o: polys.cc polys.h
	g++ -O3 -c -I$(INCLUDE)  polys.cc 
	
main.o: main.cc
	g++ -O3 -c  -I$(INCLUDE)  main.cc 
	
polys: main.o polys.o
	g++ -O3 $(LIBS) -o polys main.o polys.o

demhist: demhist.cc
	g++ -I$(INCLUDE) $(LIBS) demhist.cc -o demhist

houghlines: houghlines.cc
	g++ -I$(INCLUDE) $(LIBS) houghlines.cc -o houghlines

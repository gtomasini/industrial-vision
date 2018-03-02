all: polys nn

INCLUDE=/usr/local/include/opencv
LIBS=-lcv -lhighgui

polys.o: polys.cc polys.h
	g++ -O3 -c -I$(INCLUDE)  polys.cc 
	
main.o: main.cc
	g++ -O3 -c  -I$(INCLUDE)  main.cc 
	
polys: main.o polys.o
	g++ -O3 $(LIBS) -o polys main.o polys.o
	
dataReader.o:	dataReader.cc
	g++ -c dataReader.cc
	
neuralNetwork.o:	neuralNetwork.cc
	g++ -c neuralNetwork.cc		

neuralNetworkTrainer.o: 	neuralNetworkTrainer.cc
	g++ -c neuralNetworkTrainer.cc
	
nn: dataReader.o NNmain.o neuralNetwork.o neuralNetworkTrainer.o
	g++ -o nn dataReader.o NNmain.o neuralNetwork.o neuralNetworkTrainer.o

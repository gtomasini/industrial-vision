/*******************************************************************
 * Neural Network Training Example
 * ------------------------------------------------------------------
 * Bobby Anguelov - takinginitiative.wordpress.com (2008)
 * MSN & email: banguelov@cs.up.ac.za
 *********************************************************************/

//standard libraries
#include <iostream>
#include <ctime>
#include <stdlib.h>

//custom includes
#include "neuralNetwork.h"
#include "neuralNetworkTrainer.h"

//use standard namespace
using namespace std;

int
main(int argc, char *argv[]) {
    //seed random number generator
    srand((unsigned int) time(0));
    int in = 16, on = 3;
    int layers(1);
    int hiddenNeurons(5);
    int inputsNum(2);
    string icsv("input.csv");

    int opt;
    while ((opt = getopt(argc, argv, "i:o:l:h:")) != -1) {
        switch (opt) {
            case 'i':
                in = atoi(optarg);
                break;

            case 'o':
                on = atoi(optarg);
                break;

            case 'l':
                layers = atoi(optarg);
                break;

            case 'h':
                hiddenNeurons = atoi(optarg);
                break;

            default: /* '?' */
                cerr << "Usage: " << argv[0] << "[-i inputNum] [-o ooutputNum] "
                        << "[-l layersNum] [-h hiddenNeuronsNum]" << endl;
                exit(EXIT_FAILURE);
        }
    }
    if (optind >= argc) {
        cerr << "Expected argument after options" << endl;
        exit(EXIT_FAILURE);
    }

    //create data set reader and load data file
    dataReader d;
    d.loadDataFile(argv[optind], in, on);
    d.setCreationApproach(STATIC, 10);

    //create neural network
    neuralNetwork nn(in, hiddenNeurons, on, layers);

    //create neural network trainer
    neuralNetworkTrainer nT(nn);
    nT.setTrainingParameters(0.001, 0.9, false);
    nT.setStoppingConditions(1000, 90);
    nT.enableLogging("log.txt", 1);

    //train neural network on data sets
    for (int i = 0; i < d.getNumTrainingSets(); i++)
        nT.trainNetwork(d.getTrainingDataSet());

    //save the weights
    nn.saveWeights("weights.csv");

    cout << endl << endl << "-- END OF PROGRAM --" << endl;
    //char c; cin >> c;
}

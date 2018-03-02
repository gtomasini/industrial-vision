//standard includes
#include <iostream>
#include <vector>
#include <fstream>
#include <math.h>
#include <string.h>
#include <stdlib.h>
//include definition file
#include "neuralNetwork.h"

using namespace std;

/*******************************************************************
 * Constructor
 ********************************************************************/
neuralNetwork::neuralNetwork(int nI, int nH, int nO, int nHL) : nInput_(nI),
nHidden_(nH), nOutput_(nO), nHiddenLayers_(nHL),
inputNeurons_(new( double[nI + 1])),
hiddenNeurons_(new( double*[nHL])),
outputNeurons_(new( double[nO])),
wInputHidden_(new( double*[nI + 1])),
wHiddenHidden_(new ( double **[nHL - 1])),
wHiddenOutput_(new( double*[nHidden_ + 1])) {
    //create neuron lists
    //--------------------------------------------------------------------------------------------------------

    for (int i = 0; i < nInput_; i++) inputNeurons_[i] = 0;

    inputNeurons_[nInput_] = -1; //input bias neuron

    for (int layer = 0; layer < nHiddenLayers_; layer++) {
        hiddenNeurons_[layer] = new(double[nHidden_ + 1]);
        for (int i = 0; i < nHidden_; i++) hiddenNeurons_[layer][i] = 0;
        hiddenNeurons_[layer][nHidden_] = -1; //hidden bias neuron
    }

    for (int i = 0; i < nOutput_; i++) outputNeurons_[i] = 0;

    //create weight lists (include bias neuron weights)
    //--------------------------------------------------------------------------------------------------------
    for (int i = 0; i <= nInput_; i++) {
        wInputHidden_[i] = new (double[nHidden_]);
        for (int j = 0; j < nHidden_; j++) wInputHidden_[i][j] = 0;
    }

    for (int layer = 0; layer < nHiddenLayers_ - 1; layer++) {
        wHiddenHidden_[layer] = new ( double *[nHidden_]);
        for (int i = 0; i <= nHidden_; i++) {
            wHiddenHidden_[layer][i] = new (double[nHidden_]);
            for (int j = 0; j < nHidden_; j++) wHiddenHidden_[layer][i][j] = 0;
        }
    }

    for (int i = 0; i <= nHidden_; i++) {
        wHiddenOutput_[i] = new (double[nOutput_]);
        for (int j = 0; j < nOutput_; j++) wHiddenOutput_[i][j] = 0;
    }

    //initialize weights
    //--------------------------------------------------------------------------------------------------------
    initializeWeights();
}

/*******************************************************************
 * Destructor
 ********************************************************************/
neuralNetwork::~neuralNetwork() {
    //delete neurons
    delete[] inputNeurons_;

    for (int i = 0; i < nHiddenLayers_; i++) delete[] hiddenNeurons_[i];
    delete[] outputNeurons_;

    //delete weight storage
    for (int i = 0; i <= nInput_; i++) delete[] wInputHidden_[i];
    delete[] wInputHidden_;

    for (int layer = 0; layer < nHiddenLayers_ - 1; layer++) {
        for (int i = 0; i <= nHidden_; i++) delete[] wHiddenHidden_[layer][i];
        delete[] wHiddenHidden_[layer];
    }

    delete[] wHiddenHidden_;

    for (int j = 0; j <= nHidden_; j++) delete[] wHiddenOutput_[j];
    delete[] wHiddenOutput_;
}

/*******************************************************************
 * Load Neuron Weights
 ********************************************************************/
bool
neuralNetwork::loadWeights(const char* filename) {
    //open file for reading
    fstream inputFile;
    inputFile.open(filename, ios::in);

    if (inputFile.is_open()) {
        vector<double> weights;
        string line;

        //read data
        while (!inputFile.eof()) {
            getline(inputFile, line);

            //process line
            if (line.length() > 2 && line[0] != '#') {
                //store inputs
                //tokenise
                char aux[1024];
                strncpy(aux, line.c_str(), sizeof (aux));
                int i = 0;
                for (char *t = strtok(aux, ","); t != NULL; t = strtok(NULL, ",")) {
                    weights.push_back(atof(t));
                    i++; //move token onwards
                }
            }
        }

        //check if sufficient weights were loaded
        if (weights.size() != ((nInput_ + 1) * nHidden_
                + (nHidden_ + 1) * (nHiddenLayers_ - 1) * nHidden_
                + (nHidden_ + 1) * nOutput_)) {
            cout << endl << "Error - Incorrect number of weights in input file: " << filename << endl;

            //close file
            inputFile.close();
            return false;
        } else {
            //set weights
            int pos = 0;
            for (int i = 0; i <= nInput_; i++)
                for (int j = 0; j < nHidden_; j++)
                    wInputHidden_[i][j] = weights[pos++];

            for (int layer = 0; layer < nHiddenLayers_ - 1; layer++)
                for (int i = 0; i <= nHidden_; i++)
                    for (int j = 0; j < nHidden_; j++)
                        wHiddenHidden_[layer][i][j] = weights[pos++];

            for (int i = 0; i <= nHidden_; i++)
                for (int j = 0; j < nOutput_; j++)
                    wHiddenOutput_[i][j] = weights[pos++];
            //print success
            cout << endl << "Neuron weights loaded successfuly from '" << filename << "'" << endl;

            //close file
            inputFile.close();
            return true;
        }
    } else {
        cout << endl << "Error - Weight input file '" << filename << "' could not be opened: " << endl;
        return false;
    }
    return true;
}

/*******************************************************************
 * Save Neuron Weights
 ********************************************************************/
bool
neuralNetwork::saveWeights(const char* filename) {
    fstream outputFile;
    //open file for writing
    outputFile.open(filename, ios::out);

    if (outputFile.is_open()) {
        outputFile.precision(50);

        outputFile << "#nInput,nHidden,nHiddenLayers,nOutput" << endl;
        outputFile << nInput_ << "," << nHidden_ << "," << nHiddenLayers_
                << "," << nOutput_ << "," << endl;

        //input->hidden weights
        outputFile << "#input->hidden weights" << endl;
        for (int i = 0; i <= nInput_; i++)
            for (int j = 0; j < nHidden_; j++)
                outputFile << wInputHidden_[i][j] << ",";

        outputFile << "#hidden->hidden weights" << endl;

        for (int layer = 0; layer < nHiddenLayers_ - 1; layer++) {
            for (int i = 0; i <= nHidden_; i++)
                for (int j = 0; j < nHidden_; j++)
                    outputFile << wHiddenHidden_[layer][i][j] << ",";
            outputFile << endl;
        }

        outputFile << "#hidden->output weights" << endl;
        for (int i = 0; i <= nHidden_; i++)
            for (int j = 0; j < nOutput_; j++) {
                outputFile << wHiddenOutput_[i][j];
                if (i * nOutput_ + j + 1 != (nHidden_ + 1) * nOutput_)
                    outputFile << ",";
            }
        outputFile << "#endl" << endl;
        //print success
        cout << endl << "Neuron weights saved to '" << filename << "'" << endl;

        //close file
        outputFile.close();

        return true;
    } else {
        cout << endl << "Error - Weight output file '" << filename << "' could not be created: " << endl;
        return false;
    }
}

/*******************************************************************
 * Feed pattern through network and return results
 ********************************************************************/
int*
neuralNetwork::feedForwardPattern(const double *pattern) {
    feedForward(pattern);

    //create copy of output results
    int* results = new int[nOutput_];
    for (int i = 0; i < nOutput_; i++)
        results[i] = clampOutput(outputNeurons_[i]);

    return results;
}

/*******************************************************************
 * Return the NN accuracy on the set
 *****************************************************************	outputFile<<endl;	***/
double
neuralNetwork::getSetAccuracy(vector<dataEntry*>& set) {
    double incorrectResults = 0;

    //for every training input array
    for (int tp = 0; tp < (int) set.size(); tp++) {
        //feed inputs through network and backpropagate errors
        feedForward(set[tp]->pattern());

        //correct pattern flag
        bool correctResult(true);

        //check all outputs against desired output values
        for (int k = 0; k < nOutput_; k++)
            //set flag to false if desired and output differ
            if (clampOutput(outputNeurons_[k]) != set[tp]->target()[k]) {
                correctResult = false;
                break;
            }
        //inc training error for a incorrect result
        if (!correctResult) incorrectResults++;
    }//end for

    //calculate error and return as percentage
    return 100 - (incorrectResults / set.size() * 100);
}

/*******************************************************************
 * Return the NN mean squared error on the set
 ********************************************************************/
double
neuralNetwork::getSetMSE(vector<dataEntry*>& set) {
    double mse = 0;

    //for every training input array
    for (int tp = 0; tp < (int) set.size(); tp++) {
        //feed inputs through network and backpropagate errors
        feedForward(set[tp]->pattern());

        //check all outputs against desired output values
        for (int k = 0; k < nOutput_; k++)
            //sum all the MSEs together
            mse += pow((outputNeurons_[k] - set[tp]->target()[k]), 2);
    }//end for

    //calculate error and return as percentage
    return mse / (nOutput_ * set.size());
}

/*******************************************************************
 * Initialize Neuron Weights
 ********************************************************************/
void
neuralNetwork::initializeWeights() {
    //set range
    double rH = 1 / sqrt((double) nInput_);
    double rO = 1 / sqrt((double) nHidden_);

    //set weights between input and hidden
    //--------------------------------------------------------------------------------------------------------
    for (int i = 0; i <= nInput_; i++)
        for (int j = 0; j < nHidden_; j++) {
            //set weights to random values
            wInputHidden_[i][j] = (((double) (rand() % 100) + 1) / 100 * 2 * rH) - rH;
        }

    //set weights between hidden and hidden
    //-------------------------------------------------------------------------------------------------------

    for (int layer = 0; layer < nHiddenLayers_ - 1; layer++)
        for (int i = 0; i <= nHidden_; i++)
            for (int j = 0; j < nHidden_; j++)
                wHiddenHidden_[layer][i][j] = (((double) (rand() % 100) + 1) / 100 * 2 * rO) - rO;

    //set weights between output and hidden
    //--------------------------------------------------------------------------------------------------------
    for (int i = 0; i <= nHidden_; i++)
        for (int j = 0; j < nOutput_; j++)
            //set weights to random values
            wHiddenOutput_[i][j] = (((double) (rand() % 100) + 1) / 100 * 2 * rO) - rO;

}

/*******************************************************************
 * Activation Function
 ********************************************************************/
inline double
neuralNetwork::activationFunction(double x) {
    //sigmoid function
    return 1 / (1 + exp(-x));
}

/*******************************************************************
 * Output Clamping
 ********************************************************************/
int
neuralNetwork::clampOutput(double x) {
    if (x < 0.1) return 0;
    else if (x > 0.9) return 1;
    else return -1;
}

/*******************************************************************
 * Feed Forward Operation
 ********************************************************************/
void
neuralNetwork::feedForward(const double* pattern) {
    //set input neurons to input values
    for (int i = 0; i < nInput_; i++)
        inputNeurons_[i] = pattern[i];

    //Calculate Hidden Layer values - include bias neuron
    //--------------------------------------------------------------------------------------------------------

    for (int layer = 0; layer < nHiddenLayers_; layer++)
        for (int i = 0; i < nHidden_; i++) {
            hiddenNeurons_[layer][i] = 0; //clear value

            //get weighted sum of pattern and bias neuron
            if (layer == 0)
                for (int j = 0; j <= nInput_; j++)
                    hiddenNeurons_[0][i] += inputNeurons_[j] * wInputHidden_[j][i];
            else
                for (int j = 0; j <= nHidden_; j++)
                    hiddenNeurons_[layer][i] += hiddenNeurons_[layer - 1][j] * wHiddenHidden_[layer - 1][j][i];

            //set to result of sigmoid
            hiddenNeurons_[layer][i] = activationFunction(hiddenNeurons_[layer][i]);
        }

    //Calculating Output Layer values - include bias neuron
    //--------------------------------------------------------------------------------------------------------

    for (int i = 0; i < nOutput_; i++) {
        outputNeurons_[i] = 0; //clear value

        //get weighted sum of pattern and bias neuron
        for (int j = 0; j <= nHidden_; j++)
            outputNeurons_[i] += hiddenNeurons_[nHiddenLayers_ - 1][j] * wHiddenOutput_[j][i];

        //set to result of sigmoid
        outputNeurons_[i] = activationFunction(outputNeurons_[i]);
    }

}



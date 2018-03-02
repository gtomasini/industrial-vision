//standard includes
#include <iostream>
#include <fstream>
#include <math.h>
#include <stdlib.h>

//include definition file
#include "neuralNetworkTrainer.h"

using namespace std;


/*******************************************************************
* constructor
********************************************************************/
neuralNetworkTrainer::neuralNetworkTrainer( neuralNetwork &nn )	:	NN_(&nn),
																	epoch_(0),
																	learningRate_(LEARNING_RATE),
																	momentum_(MOMENTUM),
																	maxEpochs_(MAX_EPOCHS),
																	desiredAccuracy_(DESIRED_ACCURACY),																	
																	useBatch_(false),
																	trainingSetAccuracy_(0),
																	validationSetAccuracy_(0),
																	generalizationSetAccuracy_(0),
																	trainingSetMSE_(0),
																	validationSetMSE_(0),
																	generalizationSetMSE_(0){
	//create delta lists
	//--------------------------------------------------------------------------------------------------------
	deltaInputHidden_ = new( double*[NN_->nInput_ + 1] );
	for ( int i=0; i <= NN_->nInput_; i++ ){
		deltaInputHidden_[i] = new (double[NN_->nHidden_]);
		for ( int j=0; j < NN_->nHidden_; j++ ) deltaInputHidden_[i][j] = 0;		
	}

	deltaHiddenHidden_ = new ( double **[NN_->nHiddenLayers_-1] );
	for ( int layer=0; layer < NN_->nHiddenLayers_-1; layer++ ){
		deltaHiddenHidden_[layer] = new ( double *[NN_->nHidden_+1]);
		for ( int i=0; i <= NN_->nHidden_; i++ ){
			deltaHiddenHidden_[layer][i] = new (double[NN_->nHidden_]);
			for ( int j=0; j < NN_->nHidden_; j++ ){ 	
				deltaHiddenHidden_[layer][i][j] = 0;
			}
		}		
	}
	
	deltaHiddenOutput_ = new( double *[NN_->nHidden_ + 1] );
	for ( int i=0; i <= NN_->nHidden_; i++ ){
		deltaHiddenOutput_[i] = new (double[NN_->nOutput_]);			
		for ( int j=0; j < NN_->nOutput_; j++ ) deltaHiddenOutput_[i][j] = 0;		
	}

	//create error gradient storage
	//--------------------------------------------------------------------------------------------------------
	hiddenErrorGradients_ = new( double *[NN_->nHiddenLayers_-1] );
	for ( int layer=0; layer <= NN_->nHiddenLayers_-1; layer++ ) {
		hiddenErrorGradients_[layer]= new( double [NN_->nHidden_ + 1] );
		for ( int i=0; i <= NN_->nHidden_; i++ ) 
			hiddenErrorGradients_[layer][i] = 0;
	}
	outputErrorGradients_ = new( double[NN_->nOutput_ + 1] );
	for ( int i=0; i <= NN_->nOutput_; i++ ) outputErrorGradients_[i] = 0;
}


/*******************************************************************
* Set training parameters
********************************************************************/
void 
neuralNetworkTrainer::setTrainingParameters( double lR, double m, bool batch ){
	learningRate_	= lR;
	momentum_		= m;
	useBatch_		= batch;
}


/*******************************************************************
* Set stopping parameters
********************************************************************/
void 
neuralNetworkTrainer::setStoppingConditions( int mEpochs, double dAccuracy ){
	maxEpochs_ = mEpochs;
	desiredAccuracy_ = dAccuracy;	
}


/*******************************************************************
* Enable training logging
********************************************************************/
void 
neuralNetworkTrainer::enableLogging(const char* filename, int resolution = 1){
	//create log file 
	if ( ! logFile_.is_open() ){
		logFile_.open(filename, ios::out);

		if ( logFile_.is_open() ){
			//write log file header
			logFile_ << "Epoch,Training Set Accuracy, Generalization Set Accuracy,Training Set MSE, Generalization Set MSE" << endl;
			
			//enable logging
			loggingEnabled_ = true;
			
			//resolution setting;
			logResolution_ = resolution;
			lastEpochLogged_ = -resolution;
		}
	}
}


/*******************************************************************
* calculate output error gradient
********************************************************************/
inline double 
neuralNetworkTrainer::getOutputErrorGradient( double desiredValue, double outputValue){
	//return error gradient
	return outputValue * ( 1 - outputValue ) * ( desiredValue - outputValue );
}



/*******************************************************************
* calculate hidden error gradient
********************************************************************/

double 
neuralNetworkTrainer::getHiddenErrorGradient( int layer, int j ){
	//get sum of hidden->output weights * output error gradients
	if ( layer>0 && NN_->nHiddenLayers_==1 ){
		cerr<<"getHiddenErrorGradient(): Error insalvable!!!"<<endl;
		exit(1); 
	}
	double weightedSum = 0;
	
	if (layer==NN_->nHiddenLayers_-1){
		for( int k = 0; k < NN_->nOutput_; k++ ) {
			weightedSum += NN_->wHiddenOutput_[j][k] * outputErrorGradients_[k];
		}
		//return error gradient
	}
	else{
		for( int k = 0; k < NN_->nHidden_; k++ ) {
			weightedSum += NN_->wHiddenHidden_[layer][j][k] * hiddenErrorGradients_[layer][k];
		}

	//return error gradient
	}
	return NN_->hiddenNeurons_[layer][j] * ( 1 - NN_->hiddenNeurons_[layer][j] ) * weightedSum;
}



/*******************************************************************
* Train the NN using gradient descent
********************************************************************/
void 
neuralNetworkTrainer::trainNetwork(  trainingDataSet* tSet ){

	cout	<< endl << " Neural Network Training Starting: " << endl
			<< "==========================================================================" << endl
			<< " LR: " << learningRate_ << ", Momentum: " << momentum_ << ", Max Epochs: " << maxEpochs_ << endl
			<< " " << NN_->nInput_ << " Input Neurons, " << NN_->nHidden_ << " Hidden Neurons, " 
			<< NN_->nHiddenLayers_<<" Hidden Layers, "<<NN_->nOutput_ << " Output Neurons" <<endl
			<< "==========================================================================" << endl << endl;

	//reset epoch and log counters
	epoch_ = 0;
	lastEpochLogged_ = -logResolution_;
		
	//train network using training dataset for training and generalization dataset for testing
	//--------------------------------------------------------------------------------------------------------
	while (	( trainingSetAccuracy_ < desiredAccuracy_ || 
		generalizationSetAccuracy_ < desiredAccuracy_ ) && epoch_ < maxEpochs_ ){			

		//store previous accuracy
		double previousTAccuracy (trainingSetAccuracy_);
		double previousGAccuracy (generalizationSetAccuracy_);

		//use training set to train network
		runTrainingEpoch ( tSet->trainingSet_ );

		//get generalization set accuracy and MSE
		generalizationSetAccuracy_ = NN_->getSetAccuracy( tSet->generalizationSet_ );
		generalizationSetMSE_ = NN_->getSetMSE( tSet->generalizationSet_ );

		//Log Training results
		if ( loggingEnabled_ && logFile_.is_open() && 
				( epoch_ - lastEpochLogged_ == logResolution_ ) ) {
			logFile_ << epoch_ << "," << trainingSetAccuracy_ << "," << generalizationSetAccuracy_ << "," << trainingSetMSE_ << "," << generalizationSetMSE_ << endl;
			lastEpochLogged_ = epoch_;
		}
		
		//cout<<epoch_ << "," << trainingSetAccuracy_ << "," << generalizationSetAccuracy_ 
		//	<< "," << trainingSetMSE_ << "," << generalizationSetMSE_ << endl;
			
		//print out change in training /generalization accuracy (only if a change is greater than a percent)
		if ( ceil(previousTAccuracy) != ceil(trainingSetAccuracy_) || 
				ceil(previousGAccuracy) != ceil(generalizationSetAccuracy_) ) {	
			cout << "Epoch :" << epoch_;
			cout << " TSet Acc:" << trainingSetAccuracy_ << "%, MSE: " << trainingSetMSE_ ;
			cout << " GSet Acc:" << generalizationSetAccuracy_ << "%, MSE: " << generalizationSetMSE_ << endl;				
		}
		
		//once training set is complete increment epoch
		epoch_++;
	}//end while

	//get validation set accuracy and MSE
	validationSetAccuracy_ = NN_->getSetAccuracy(tSet->validationSet_);
	validationSetMSE_ = NN_->getSetMSE(tSet->validationSet_);

	//log end
	logFile_ << epoch_ << "," << trainingSetAccuracy_ << "," << generalizationSetAccuracy_ << "," << trainingSetMSE_ << "," << generalizationSetMSE_ << endl << endl;
	logFile_ << "Training Complete!!! - > Elapsed Epochs: " << epoch_ << " Validation Set Accuracy: " << validationSetAccuracy_ << " Validation Set MSE: " << validationSetMSE_ << endl;
			
	//out validation accuracy and MSE
	cout << endl << "Training Complete!!! - > Elapsed Epochs: " << epoch_ << endl;
	cout << " Validation Set Accuracy: " << validationSetAccuracy_ << endl;
	cout << " Validation Set MSE: " << validationSetMSE_ << endl << endl;
}


/*******************************************************************
* Run a single training epoch
********************************************************************/
void 
neuralNetworkTrainer::runTrainingEpoch( vector<dataEntry*> &trainingSet ){
	double incorrectPatterns (0), mse (0);
		
	//for every training pattern
	for ( int tp = 0; tp < (int) trainingSet.size(); tp++){						
		//feed inputs through network and backpropagate errors
		NN_->feedForward ( trainingSet[tp]->pattern() );
		backpropagate ( trainingSet[tp]->target() );	

		//pattern correct flag
		bool patternCorrect (true);

		//check all outputs from neural network against desired values
		for ( int k = 0; k < NN_->nOutput_; k++ ){					
			//pattern incorrect if desired and output differ
			if ( NN_->clampOutput( NN_->outputNeurons_[k] ) != 
						trainingSet[tp]->target()[k] ) 
				patternCorrect = false;
			
			//calculate MSE
			mse += pow(( NN_->outputNeurons_[k] - trainingSet[tp]->target()[k] ), 2);
		}
		
		//if pattern is incorrect add to incorrect count
		if ( !patternCorrect ) incorrectPatterns++;	
		
	}//end for

	//if using batch learning - update the weights
	if ( useBatch_ ) updateWeights();
	
	//update training accuracy and MSE
	trainingSetAccuracy_ = 100 - (incorrectPatterns/trainingSet.size() * 100);
	trainingSetMSE_ = mse / ( NN_->nOutput_ * trainingSet.size() );
	
}


/*******************************************************************
* Propagate errors back through NN and calculate delta values
********************************************************************/
void 
neuralNetworkTrainer::backpropagate ( const double* desiredOutputs ){
		
	//modify deltas between hidden and output layers
	//--------------------------------------------------------------------------------------------------------
	for (int k = 0; k < NN_->nOutput_; k++){
		//get error gradient for every output node
		outputErrorGradients_[k] = getOutputErrorGradient( desiredOutputs[k], NN_->outputNeurons_[k] );
		
		//for all nodes in hidden layers and bias neuron
		for (int j = 0; j <= NN_->nHidden_; j++) 			
			//calculate change in weight
			if ( !useBatch_ ) 
				deltaHiddenOutput_[j][k] = 
					learningRate_ * NN_->hiddenNeurons_[NN_->nHiddenLayers_-1][j] * outputErrorGradients_[k] 
															+ momentum_ * deltaHiddenOutput_[j][k];
			else 
				deltaHiddenOutput_[j][k] += 
					learningRate_ * NN_->hiddenNeurons_[NN_->nHiddenLayers_-1][j] * outputErrorGradients_[k];
	}

	//modify deltas between hidden layers
	//--------------------------------------------------------------------------------------------------------
	for (int layer=NN_->nHiddenLayers_-1; layer>0 ; layer-- ){
		for (int k = 0; k < NN_->nHidden_; k++){
			hiddenErrorGradients_[layer][k] = getHiddenErrorGradient( layer, k );

			//for all nodes in input layer and bias neuron
			for (int j = 0; j <= NN_->nHidden_; j++){
				//calculate change in weight 
				if ( !useBatch_ ){
					deltaHiddenHidden_[layer-1][j][k] = 
						learningRate_ * NN_->hiddenNeurons_[layer-1][j] * hiddenErrorGradients_[layer][k] 
									+ momentum_ * deltaHiddenHidden_[layer-1][j][k];
				}
				else 
					deltaHiddenHidden_[layer-1][j][k] += 
						learningRate_ * NN_->hiddenNeurons_[layer-1][j] * hiddenErrorGradients_[layer][k];			
			}
		}
	}
	
	//modify deltas between input and hidden layers
	//--------------------------------------------------------------------------------------------------------

	for (int j = 0; j < NN_->nHidden_; j++){
		//get error gradient for every hidden node
		hiddenErrorGradients_[0][j] = getHiddenErrorGradient( 0, j );

		//for all nodes in input layer and bias neuron
		for (int i = 0; i <= NN_->nInput_; i++){
			//calculate change in weight 
			if ( !useBatch_ ) 
				deltaInputHidden_[i][j] = 
						learningRate_ * NN_->inputNeurons_[i] * hiddenErrorGradients_[0][j] 
									+ momentum_ * deltaInputHidden_[i][j];
			else 
				deltaInputHidden_[i][j] += learningRate_ * NN_->inputNeurons_[i] * hiddenErrorGradients_[0][j]; 
		}
	}
	//if using stochastic learning update the weights immediately
	if ( !useBatch_ ) updateWeights();
}

/*******************************************************************
* Update weights using delta values
********************************************************************/
void 
neuralNetworkTrainer::updateWeights(){
	//input -> hidden weights
	//--------------------------------------------------------------------------------------------------------
	for (int i = 0; i <= NN_->nInput_; i++)
		for (int j = 0; j < NN_->nHidden_; j++) {
			//update weight
			NN_->wInputHidden_[i][j] += deltaInputHidden_[i][j];	
			
			//clear delta only if using batch (previous delta is needed for momentum
			if (useBatch_) deltaInputHidden_[i][j] = 0;				
		}
		
	//hidden -> hidden weights
	//--------------------------------------------------------------------------------------------------------
	for (int layer=0; layer < NN_->nHiddenLayers_-1; layer++ )
		for (int j = 0; j <= NN_->nHidden_; j++)
			for (int k = 0; k < NN_->nHidden_; k++) {						
				//update weight
				NN_->wHiddenHidden_[layer][j][k] += deltaHiddenHidden_[layer][j][k];
			
			//clear delta only if using batch (previous delta is needed for momentum)
			if (useBatch_)deltaHiddenHidden_[layer][j][k] = 0;
		}	
	
	//hidden -> output weights
	//--------------------------------------------------------------------------------------------------------
	for (int j = 0; j <= NN_->nHidden_; j++)
		for (int k = 0; k < NN_->nOutput_; k++) {					
			//update weight
			NN_->wHiddenOutput_[j][k] += deltaHiddenOutput_[j][k];
			
			//clear delta only if using batch (previous delta is needed for momentum)
			if (useBatch_)deltaHiddenOutput_[j][k] = 0;
		}
}

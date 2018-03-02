/*******************************************************************
* Neural Network Training Class
* ------------------------------------------------------------------
* Bobby Anguelov - takinginitiative.wordpress.com (2008)
* MSN & email: banguelov@cs.up.ac.za
*********************************************************************/

#ifndef NNetworkTrainer
#define NNetworkTrainer

//standard includes
#include <fstream>
#include <vector>

//neural network header
#include "neuralNetwork.h"

//Constant Defaults!
#define LEARNING_RATE 0.001
#define MOMENTUM 0.9
#define MAX_EPOCHS 1500
#define DESIRED_ACCURACY 90  
#define DESIRED_MSE 0.001 

/*******************************************************************
* Basic Gradient Descent Trainer with Momentum and Batch Learning
********************************************************************/
class neuralNetworkTrainer{
	//class members
	//--------------------------------------------------------------------------------------------

private:

	neuralNetwork *NN_;	//network to be trained

	//learning parameters
	double learningRate_;		// adjusts the step size of the weight update	
	double momentum_;			// improves performance of stochastic learning (don't use for batch)

	long epoch_;//epoch counter
	long maxEpochs_;
	
	//accuracy/MSE required
	double desiredAccuracy_;
	
	//change to weights
	double **deltaInputHidden_;
	double ***deltaHiddenHidden_;
	double **deltaHiddenOutput_;

	//error gradients
	double **hiddenErrorGradients_;
	double *outputErrorGradients_;

	//accuracy stats per epoch
	double trainingSetAccuracy_;
	double validationSetAccuracy_;
	double generalizationSetAccuracy_;
	double trainingSetMSE_;
	double validationSetMSE_;
	double generalizationSetMSE_;

	//batch learning flag
	bool useBatch_;

	//log file handle
	bool loggingEnabled_;
	std::fstream logFile_;
	int logResolution_;
	int lastEpochLogged_;

	//public methods
	//--------------------------------------------------------------------------------------------
public:	
	
	neuralNetworkTrainer( neuralNetwork &untrainedNetwork );
	void setTrainingParameters( double lR, double m, bool batch );
	void setStoppingConditions( int mEpochs, double dAccuracy);
	void useBatchLearning( bool flag ){ useBatch_ = flag; }
	void enableLogging( const char* filename, int resolution );

	void trainNetwork(  trainingDataSet* tSet );

	//private methods
	//--------------------------------------------------------------------------------------------
private:
	inline double getOutputErrorGradient( double desiredValue, double outputValue );
	double getHiddenErrorGradient( int layer, int row );
	void runTrainingEpoch( std::vector<dataEntry*> &trainingSet );
	void backpropagate(const double* desiredOutputs);
	void updateWeights();
};


#endif

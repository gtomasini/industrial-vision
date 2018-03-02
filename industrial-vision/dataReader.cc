//include definition file
#include "dataReader.h"

#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <math.h>
#include <algorithm>

using namespace std;

/*******************************************************************
 * Destructor
 ********************************************************************/
dataReader::~dataReader() {
    //clear data
    for (int i = 0; i < (int) data_.size(); i++) delete data_[i];
    data_.clear();
}

/*******************************************************************
 * Loads a csv file of input data
 ********************************************************************/
bool
dataReader::loadDataFile(const char* filename, int nI, int nT) {

    //clear any previous data
    for (int i = 0; i < (int) data_.size(); i++) delete data_[i];
    data_.clear();
    tSet_.clear();

    //set number of inputs and outputs
    nInputs_ = nI;
    nTargets_ = nT;

    //open file for reading
    fstream inputFile;
    inputFile.open(filename, ios::in);

    if (inputFile.is_open()) {
        string line;

        //read data
        for( int i=0; !inputFile.eof(); i++) {
            getline(inputFile, line);

            //process line
            if (line.length() > 2 && line[0]!='#')
                if (processLine(line) == false)
                    cerr<<"error in line "<<i<<endl;
        }

        //shuffle data
        //random_shuffle(data.begin(), data.end());

        //split data set
        trainingDataEndIndex_ = (int) (0.6 * data_.size());
        int gSize = (int) (ceil(0.2 * data_.size()));
        int vSize = (int) (data_.size() - trainingDataEndIndex_ - gSize);

        //generalization set
        for (int i = trainingDataEndIndex_; i < trainingDataEndIndex_ + gSize; i++)
            tSet_.generalizationSet_.push_back(data_[i]);

        //validation set
        for (int i = trainingDataEndIndex_ + gSize; i < (int) data_.size(); i++)
            tSet_.validationSet_.push_back(data_[i]);

        //print success
        cout << "Input File: " << filename << "\nRead Complete: " << data_.size() << " Patterns Loaded" << endl;

        //close file
        inputFile.close();

        return true;
    } else {
        cout << "Error Opening Input File: " << filename << endl;
        return false;
    }
}

/*******************************************************************
 * Processes a single line from the data file
 ********************************************************************/
bool dataReader::processLine(const string &line) {
    //create new pattern and target
    double* pattern(new double[nInputs_]);
    double* target(new double[nTargets_]);

    //for(int i=0;i<nTargets_;++i) target[i]=0;

    //store inputs
    //char* cstr ( new char[line.size()+1] );
    //char cstr[1024];
    //strcpy(cstr, line.c_str());

    //tokenise

    //store inputs
    string::size_type pos(0), t;
    int i = 0;

    do {
        t = line.find_first_of(",; |", pos);

        if (t == string::npos) t = line.size();

        
        string auxs(line.substr(pos, t - pos));

        int c=auxs.at(0);
        if ( c>='A' && c<='Z' ){
            int k=c-'A';
            if (k>=0 && k<nTargets_)
                target[k]=1;
            else
                cerr<<"character item \""<<c<<"\"out of range!!";
            i=nInputs_+nTargets_;
            break;
        }
        
        double auxf = atof(auxs.c_str());
        if (i < nInputs_) pattern[i]=auxf;
        else target[i-nInputs_]=auxf;

        pos = t + 1;
        i++;
    }
    while (t < line.size());

    if ( i != nInputs_+ nTargets_ ){
        cerr<<"incorrect number of items!"<<endl;
        delete pattern;
        delete target;
        return false;
    }

    //add to records
    data_.push_back(new dataEntry(pattern, target));
    return true;
}

/*******************************************************************
 * Selects the data set creation approach
 ********************************************************************/
void
dataReader::setCreationApproach(int approach, double param1, double param2) {
    //static

    if (approach == STATIC) {
        creationApproach_ = STATIC;

        //only 1 data set
        numTrainingSets_ = 1;
    }        //growing
    else if (approach == GROWING) {
        if (param1 <= 100.0 && param1 > 0) {
            creationApproach_ = GROWING;

            //step size
            growingStepSize_ = param1 / 100;
            growingLastDataIndex_ = 0;

            //number of sets
            numTrainingSets_ = (int) ceil(1 / growingStepSize_);
        }
    }        //windowing
    else if (approach == WINDOWING) {
        //if initial size smaller than total entries and step size smaller than set size
        if (param1 < data_.size() && param2 <= param1) {
            creationApproach_ = WINDOWING;

            //params
            windowingSetSize_ = (int) param1;
            windowingStepSize_ = (int) param2;
            windowingStartIndex_ = 0;

            //number of sets
            numTrainingSets_ = (int) ceil((double) (trainingDataEndIndex_ - windowingSetSize_) / windowingStepSize_) + 1;
        }
    }
}

/*******************************************************************
 * Returns number of data sets created by creation approach
 ********************************************************************/
int
dataReader::getNumTrainingSets() {
    return numTrainingSets_;
}

/*******************************************************************
 * Get data set created by creation approach
 ********************************************************************/
trainingDataSet* dataReader::getTrainingDataSet() {
    switch (creationApproach_) {
        case STATIC: createStaticDataSet();
            break;
        case GROWING: createGrowingDataSet();
            break;
        case WINDOWING: createWindowingDataSet();
            break;
    }

    return &tSet_;
}

/*******************************************************************
 * Get all data entries loaded
 ********************************************************************/
vector<dataEntry*>& dataReader::getAllDataEntries() {
    return data_;
}

/*******************************************************************
 * Create a static data set (all the entries)
 ********************************************************************/
void dataReader::createStaticDataSet() {
    //training set
    for (int i = 0; i < trainingDataEndIndex_; i++)
        tSet_.trainingSet_.push_back(data_[i]);
}

/*******************************************************************
 * Create a growing data set (contains only a percentage of entries
 * and slowly grows till it contains all entries)
 ********************************************************************/
void
dataReader::createGrowingDataSet() {
    //increase data set by step percentage
    growingLastDataIndex_ += (int) ceil(growingStepSize_ * trainingDataEndIndex_);
    if (growingLastDataIndex_ > (int) trainingDataEndIndex_)
        growingLastDataIndex_ = trainingDataEndIndex_;

    //clear sets
    tSet_.trainingSet_.clear();

    //training set
    for (int i = 0; i < growingLastDataIndex_; i++)
        tSet_.trainingSet_.push_back(data_[i]);
}

/*******************************************************************
 * Create a windowed data set ( creates a window over a part of the data
 * set and moves it along until it reaches the end of the date set )
 ********************************************************************/
void
dataReader::createWindowingDataSet() {
    //create end point
    int endIndex = windowingStartIndex_ + windowingSetSize_;
    if (endIndex > trainingDataEndIndex_)
        endIndex = trainingDataEndIndex_;

    //clear sets
    tSet_.trainingSet_.clear();

    //training set
    for (int i = windowingStartIndex_; i < endIndex; i++)
        tSet_.trainingSet_.push_back(data_[i]);

    //increase start index
    windowingStartIndex_ += windowingStepSize_;
}


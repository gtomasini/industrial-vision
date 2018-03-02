/*******************************************************************
 * CSV Data File Reader and Training Set Creator
 * ------------------------------------------------------------------
 * Bobby Anguelov - takinginitiative.wordpress.com (2008)
 * MSN & email: banguelov@cs.up.ac.za
 ********************************************************************/

#ifndef _DATAREADER
#define _DATAREADER

//include standard header files
#include <vector>
#include <string>

/*******************************************************************
 * stores a data item
 ********************************************************************/
class dataEntry {
private:
    double* pattern_; //input patterns
    double* target_; //target (output) result

public:

    dataEntry(double* p, double* t) : pattern_(p), target_(t) {
    }

    double *pattern() {
        return pattern_;
    }

    double *target() {
        return target_;
    }

    ~dataEntry() {
        delete[] pattern_;
        delete[] target_;
    }
};

/*******************************************************************
 * Training Sets Storage - stores shortcuts to data items
 ********************************************************************/
class trainingDataSet {
public:

    std::vector<dataEntry*> trainingSet_;
    std::vector<dataEntry*> generalizationSet_;
    std::vector<dataEntry*> validationSet_;

    trainingDataSet() {
    }

    void clear() {
        trainingSet_.clear();
        generalizationSet_.clear();
        validationSet_.clear();
    }
};

//dataset retrieval approach enum

enum {
    NONE, STATIC, GROWING, WINDOWING
};

//data reader class

class dataReader {
public:

    dataReader() : creationApproach_(NONE), numTrainingSets_(-1) {
    }
    ~dataReader();
    bool loadDataFile(const char* filename, int nI, int nT);
    void setCreationApproach(int approach, double param1 = -1, double param2 = -1);
    int getNumTrainingSets();

    trainingDataSet* getTrainingDataSet();
    std::vector<dataEntry*>& getAllDataEntries();

private:

    void createStaticDataSet();
    void createGrowingDataSet();
    void createWindowingDataSet();
    bool processLine(const std::string &line);

    //data storage
    std::vector<dataEntry*> data_;
    int nInputs_;
    int nTargets_;

    //current data set
    trainingDataSet tSet_;

    //data set creation approach and total number of dataSets
    int creationApproach_;
    int numTrainingSets_;
    int trainingDataEndIndex_;

    //creation approach variables
    double growingStepSize_; //step size - percentage of total set
    int growingLastDataIndex_; //last index added to current dataSet
    int windowingSetSize_; //initial size of set
    int windowingStepSize_; //how many entries to move window by
    int windowingStartIndex_; //window start index

};

#endif

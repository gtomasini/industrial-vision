#ifndef POLYS_H
#define POLYS_H

#include <vector>
#include <stdexcept>
#include "cv.h"
#include "highgui.h"



class polysError:public std::runtime_error {
    char    str[128];
    
public:
    polysError(const std::string& msg = ""):runtime_error(msg){
    }
};

class polys{

public:
	/**
	* Constructor
	*
	* @param K sigma, se usa para filtrar el nivel de intensidad de color
	* @parame sides deprecated
	*/
	polys( const char *fname, const CvPoint &p0, const CvPoint &p1, int K=4, 
		int delay=1000, int sides=4 );
		
	CvSeq *findObject ( double area );
	
	/**
	* obsoleta
	*
	*/
	CvSeq* findSquares4( const int thresh = 50, const int N=11 );
	
	/**
	* obsoleta
	*
	*/
	void drawPolys( ) throw (polysError);
	
	/**
	* obsoleta
	*
	*/	
	void matrix(  );
	
	void getSign ( const std::vector <CvPoint *> &ptV, 
		std::vector< std::vector<float> > &deltaVV );
		
	void matrixPaint ( const std::vector< std::vector<float> > &deltaVV,
		const std::string &filename );
		
	void matrix2csv ( const std::vector< std::vector<float> > &deltaVV,
		const std::string &filename );
	

	void findPoints ( const CvSeq* contorno, int n, std::vector <CvPoint *> &ptV );		

	~polys();
	
private:

	double 	winLevel ( IplImage* img, unsigned int x0, unsigned int y0, 
		unsigned int x1, unsigned int y1, double &sigma );
		
	float distance ( const CvPoint &p0, const CvPoint &pf );
	
	static void adjustXY (int arg);
	
	double angle( CvPoint* pt1, CvPoint* pt2, CvPoint* pt0 );
	
	CvMemStorage* storage_; 
	const char *wndname_;
	static char *Wndname_;
	static IplImage* img0_; 
	CvPoint p0_, p1_;
	int K_;
	int sides_;
	int apLevel_;
	// create empty sequence that will contain points -
	// 4 points per square (the square's vertices)
	CvSeq* polys_;
	static int x_, y_, a_, b_;
	int delay_;
	//std::vector <CvPoint> ptV_;
	
	const int ptArraySize_;
	CvPoint ptArray_[128];
	//std::vector <CvPoint *> ptV_;
};


#endif

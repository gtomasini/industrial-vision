//
// The full "Square Detector" program.
// It loads several images subsequentally and tries to find squares in
// each image
//

/*
12-03-2008

Sistemas de bloques
etiquetado

tarea:
ventana de la mesa para sacar media de colores:
ej:
   (300,200) a (400,400)
   
sigma, estadìsticamente
operaciòn de filtrado (morfologico) erosiòn?
reveer segmentaciòn: canny (detecccion bordes), 


*/

#ifdef _CH_
#pragma package <opencv>
#endif

#include <fstream>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <time.h>
#include "polys.h"

// try several threshold levels

#define DEBUG
using namespace std;

int polys::x_;
int polys::y_;
int polys::a_;
int polys::b_;
char *polys::Wndname_("main");
IplImage* polys::img0_; 


// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2 
double polys::angle( CvPoint* pt1, CvPoint* pt2, CvPoint* pt0 ){
    double dx1 = pt1->x - pt0->x;
   	double dy1 = pt1->y - pt0->y;
    double dx2 = pt2->x - pt0->x;
    double dy2 = pt2->y - pt0->y;
   	return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}
	
void 
polys::adjustXY( int arg ){
	IplImage* img=cvCloneImage( img0_);
	
	CvPoint pt1, pt2;
	
	pt1.x=x_; pt1.y=y_; pt2.x=x_; pt2.y=y_+b_;
	cvLine( img, pt1, pt2, CV_RGB(255,0,0), 3, 8, 0 );
	
	pt2.x=x_+a_; pt2.y=y_;
	cvLine( img, pt1, pt2, CV_RGB(255,0,0), 3, 8, 0 );
	
	pt1.x=x_+a_;pt2.y=y_+b_;
	cvLine( img, pt1, pt2, CV_RGB(255,0,0), 3, 8, 0 );
	
	pt1.x=x_;pt1.y=y_+b_;pt2.x=x_+a_;
	cvLine( img, pt1, pt2, CV_RGB(255,0,0), 3, 8, 0 );
				
	cvShowImage( Wndname_, img );
	cvReleaseImage ( &img );
}

polys::polys( const char *fname, const CvPoint &p0, const CvPoint &p1, int K, 
	int delay, int sides)
	:storage_(cvCreateMemStorage(0)),wndname_(fname), 
	K_(K),p0_(p0),p1_(p1), sides_(sides), delay_(delay), 
	polys_(cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvPoint), storage_ )),
	ptArraySize_ (sizeof(ptArray_)/sizeof(ptArray_[0])){
	
	// create memory storage that will contain all the dynamic data
		
	cvNamedWindow( Wndname_, 1 );
	cvNamedWindow( wndname_, 1 );
	
	// load i-th image
	img0_ = cvLoadImage( fname, 1 );
		
	if( !img0_ ){
	  	cerr<<"polys::polys(): couldn't load "<<fname<<endl;
	    return;
	}
		
	//muestro ventana
	IplImage *img( cvCloneImage( img0_ ));
	CvPoint pt1,pt2;
	pt1=p0;
	pt2.x=p1.x;
	pt2.y=p0.y;
	cvLine( img, pt1, pt2, CV_RGB(255,0,0), 3, 8, 0 );
	pt1.x=p1.x;
	pt1.y=p0.y;
	pt2.y=p1.y;
	cvLine( img, pt1, pt2, CV_RGB(255,0,0), 3, 8, 0 );
	pt1.x=p0.x;
	pt1.y=p1.y;
	cvLine( img, pt1, pt2, CV_RGB(255,0,0), 3, 8, 0 );
	pt2.x=p0.x;
	pt2.y=p0.y;
	cvLine( img, pt1, pt2, CV_RGB(255,0,0), 3, 8, 0 );
	x_=p0.x;
	y_=p0.y;
	a_=p1.x-x_;
	b_=p1.y-y_;
	
	cvCreateTrackbar("x", Wndname_, & x_, img0_->width, adjustXY );
	cvCreateTrackbar("y", Wndname_, & y_, img0_->height, adjustXY );
	cvCreateTrackbar("a", Wndname_, & a_, img0_->width/2, adjustXY );
	cvCreateTrackbar("b", Wndname_, & b_, img0_->height/2, adjustXY );
		
	cvShowImage( Wndname_, img );
	cvWaitKey(0);
	cvReleaseImage( &img );
	p0_.x=x_;p0_.y=y_;
	p1_.x=x_+a_;p1_.y=y_+b_;
}

polys::~polys(){
	cvDestroyWindow( wndname_ );
	cvDestroyWindow( Wndname_ );
    cvReleaseImage( &img0_ );
	cvClearMemStorage( storage_ );
}
	
double 
polys::winLevel ( IplImage* img, 
	unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1,
	double &sigma ){
	if ( x0>x1 || y0>y1 || x0> img->width || x1 > img->width ||
		 y0 > img->height, y1 > img->height ){
		cerr<<"winLevel: error en coordenadas ventana"<<endl;
		return -1;
	}
		
	int n=(x1-x0)*(y1-y0);
	
	double media(0);
	for (int i=x0; i<x1; ++i )
		for (int j=y0; j<y1; ++j) 
			media+=cvGetReal2D( img, j, i );
				
	media/=n;
	sigma=0;
	for (int i=x0; i<x1; ++i )
		for (int j=y0; j<y1; ++j) {
			double aux(media-cvGetReal2D( img, j, i ));
			sigma+=aux*aux;
		}
			
	sigma/=n;
	sigma=sqrt(sigma);
	cout<< "winLevel("<<x0<<","<<y0<<","<<x1<<","<<y1<<"): media="<<media
		<<", sigma="<<sigma<<endl;
		
	return media;
}
    

void 
polys::findPoints ( const CvSeq* contorno, int n, vector <CvPoint *> &ptV ){
	const float perim = cvContourPerimeter( contorno );
	const float delta=perim/n;
	float perimParcial(0);
	CvPoint *anterior((CvPoint *)cvGetSeqElem( contorno, 0 ));
	ptV.clear();
	int k(0);
	cout<<"polys::findPoints(), perimetro:"<<perim<<", delta:"<<delta<<endl;
	for (int i=0; ;++i){
		CvPoint *p=(CvPoint*)cvGetSeqElem( contorno, i );
		if (p==NULL) break;
		
		perimParcial += distance ( *anterior, *p);
		anterior=p;
		if (perimParcial > k*delta){
			++k;
			cout<<k<<". perim parcial acumulado="<<perimParcial
				<<", nuevo limite="<<delta*k;
			cout<<"  ("<<p->x<<", "<< p->y <<") "<<endl;
			ptV.push_back ( p );
			if ( k==n ){
				cout<<"polys::findPoints() loop ends!"<<endl;
				break;
			}
		}
	}
}



CvSeq *
polys::findObject ( double Area){
	CvSize sz = cvSize( img0_->width & -2, img0_->height & -2);
	     
   	IplImage* timg = cvCloneImage( img0_ ); // make a copy of input image
   	cvSetImageROI( timg, cvRect( 0, 0, sz.width, sz.height ));
		
	//IplImage* pyr = cvCreateImage( cvSize(sz.width/2, sz.height/2), 8, 3 );
		
	//cvPyrDown( timg, pyr, 7 );
	//cvPyrUp( pyr, timg, 7 );
		
	IplImage* tgray = cvCreateImage( sz, 8, 1 );
	IplImage* gray = cvCreateImage( sz, 8, 1 ); 
	IplImage* result = cvCloneImage(tgray );
	IplImage* result2 = cvCloneImage(tgray );
	cvZero ( result2 );

	//la pongo en blanco
	cvSet( result,  cvScalarAll(255), 0 );
		
	for( int c = 0; c < 3; c++ ){
		// extract the c-th color plane
    	cvSetImageCOI( timg, c+1 );
		cvCopy( timg, tgray, 0 );   
		double sigma;
		double media=winLevel ( tgray, p0_.x, p0_.y, p1_.x, p1_.y, sigma );
		sigma*=K_;
							
		for ( int i=0;i<sz.height; ++i)
			for ( int j=0;j<sz.width ; ++j){
				double z=cvGetReal2D( tgray, i, j );
				//hago el and con el anterior
				if ( z < ( media-sigma) || z> ( media+sigma) )
					cvSetReal2D ( result, i ,j, 0 );		 
    		}
					
		cvShowImage( wndname_, result );
		cvWaitKey(delay_);
   	}
	 
	//dilato, salida en result2
	cvDilate( result, result2, NULL, 1);
		
	cvShowImage( wndname_, result2 );
	cvWaitKey(delay_);
	
	//erosiono, salida en result
	cvErode ( result2, result, NULL, 1);
		
	cvShowImage( wndname_, result );
	
	cvWaitKey( delay_ );
	//cvReleaseImage( &result2 );
		
	//canny, salida en gray	
    cvCanny( result, gray, 0, 10 );

	cvZero( result2 );
	cvShowImage( wndname_, gray );
		
	cvWaitKey( delay_ );
		
    CvSeq* contour;//busco contornos, resultado en contour	
	int n=cvFindContours( gray, storage_, &contour, 
		sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE );

	cout<<"polys::findObject, #finded contours="<<n<<endl;
	cvZero( result2 );
	cvShowImage( wndname_, result2 );	
	
	CvSeq* c;
	//buscamos el contorno que cumpla la condiciòn de area, y dibujamos todos.
	for(  c=contour; c != 0; c = c->h_next ){
        CvScalar color = CV_RGB( rand()&255, rand()&255, rand()&255 );
        cvDrawContours( result2, c, color, color, 0, CV_AA, 8 );
		double area=cvContourArea(c);
		cout<<" area:"<<area<<endl;
		if ( abs ( (long long)area ) > Area ){
			//findPoints ( c, 10 );
			cout<<"se cumple: area >"<<Area<<"!"<<endl;
			break;
		}
	}
	if (c==0) 
		cerr<<"polys::findObject: no hay contorno con area >"<<Area<<" !!"<<endl;
		
	cvWaitKey( delay_ );
	
	return c;
} 
	


float 
polys::distance ( const CvPoint &p0, const CvPoint &pf ){
	float x (p0.x-pf.x);
	float y (p0.y-pf.y);
	
	return sqrtf (x*x +y*y);
}

	
//Algoritmo Bruckstein (cocientes de triangulos entre puntos)
void
polys::getSign ( const vector <CvPoint *> &ptV, vector< vector<float> > &deltaVV ){
	
	vector< vector<float> > areaVV;
	for( int i=0; i < ptV.size(); ++i )  {
		vector<float> areaV;		
		//for (int k=1; k<( lados_)/2; ++k){
		int K=(ptV.size()-1)/2;
		
#ifdef DEBUG
		cout<<"p["<<i<<"]: "<<ptV[i]->x<<","<<ptV[i]->y<<", K:"<<K<<endl;
#endif		
		
		for (int k=1; k<= K; ++k){
			int m(i-k);
			int n(i+k);
				
			m=(m<0)?ptV.size()+m:m;
			n=(n>=ptV.size())?n-ptV.size():n;
							
			// aplicamos la formula de Heron para el calculo de area de triangulo
			// si a
			// p=(a+b+c)/2
			// area=sqrt( p(p-a)(p-b)(p-c) )
			
			float a( distance( *ptV[i], *ptV[n]) );
			float b( distance( *ptV[i], *ptV[m]) );
			float c( distance( *ptV[m], *ptV[n]) );

			float p( (a+b+c)/2);
			float aux(p*(p-a)*(p-b)*(p-c));
			aux=(aux<0)?(-aux):aux;
			
			float area( sqrtf(aux ) );
			
#ifdef DEBUG
			cout<<area<<" ";
#endif
			areaV.push_back(area);
		}
		areaVV.push_back (areaV);  
	
	}//for (i );
		
	//vector < vector < float > > deltaVV; 
	for (int ii=0; ii<areaVV.size(); ++ii){
		vector <float> areaV=areaVV[ii], deltaV;
		for ( int jj=0; jj<areaV.size() ; ++jj )
			for ( int kk=0; kk<areaV.size() ; ++kk )
				if (jj != kk )
					deltaV.push_back (areaV[jj]/areaV[kk]);
		deltaVV.push_back (deltaV);
	}
}

void polys::matrixPaint ( const std::vector< std::vector<float> > &deltaVV,
	const string &filename ){
	
	float mayor(0);		
	for (int ii=0; ii<deltaVV.size(); ++ii){
		vector <float> deltaV=deltaVV[ii];
		for ( int jj=0; jj<deltaV.size() ; ++jj )
			if ( deltaV[jj]>mayor )	mayor=deltaV[jj];
	}
							
	CvSize sz = cvSize (  deltaVV.size(), deltaVV[0].size()  );
	IplImage  *tgray(cvCreateImage(sz,  8, 1 ));
		
	cout<<endl<<"mayor="<<mayor<<", deltaV.size():"<<deltaVV.size()<<endl;		
		
	for (int ii=0; ii<deltaVV.size(); ++ii ){
		vector <float>	deltaV(deltaVV[ii]);
		for( int jj=0; jj< deltaV.size(); ++jj){
			cvSetReal2D ( tgray, jj, ii, deltaV[jj]*255/mayor );
		}
	}
		
	sz = cvSize ( 640, 480 );
	cvNamedWindow( filename.c_str(), 1 );	
	IplImage  *tgray2(cvCreateImage(sz, 8, 1 ));
		
	cvResize ( tgray, tgray2 );	
	cvShowImage( filename.c_str(), tgray2 );
		
	cvSaveImage( filename.c_str(), tgray2 );
	cvWaitKey( 0 );
		
	cvReleaseImage ( &tgray );
	cvReleaseImage ( &tgray2 );
}

void polys::matrix2csv ( const std::vector< std::vector<float> > &deltaVV,
	const string &filename ){
	
	ofstream myfile;
	myfile.open ( filename.c_str() );
	//myfile << "Writing this to a file.\n";
 
	for (int ii=0; ii<deltaVV.size(); ++ii){
		vector <float> deltaV=deltaVV[ii];
		for ( int jj=0; jj<deltaV.size() ; ++jj ){
			myfile << deltaV[jj]<<",";
		}	
	
	}
	myfile<<"1"<<endl;
	myfile.close();						
	
}


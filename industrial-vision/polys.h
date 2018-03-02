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

/**
* polys
*
* Con esta clase se logra todo el procesamiento de la imagen (jpg)
* para identificar el objeto geometricamente y generar la firma (matriz) 
* caracteristica. Dicha matriz sera la entrada de la red neuronal.
*
*/


class polys{

public:
	/**
	* Constructor
	*
	* @param fname archivo jpg conteniendo el objeto
	* @param p0 junto con p1 forma un rectangulo del cual se muestrea el tono del objeto
	* @param p1 junto con p0 forma un rectandulo del cual se muestrea el tono del objeto
	* @param K sigma se usa para filtrar el nivel de intensidad de color
	* @param delay tiempo que se demora mostrando una imagen 
	* @param sides depreciado
	*/
	polys( const char *fname, const CvPoint &p0, const CvPoint &p1, int K=4, 
		int delay=1000, int sides=4 );
		
	/**
	* retorna el objeto del archivo como una CvSeq
	*
	* @param area area aproximada minima del objeto, con esto se pueden discriminar contornos
	* 
	* @return la secuencia del contorno	del objeto
	*/
	CvSeq *findObject ( double area );
	
	/**
	* depreciada
	*
	*/
	CvSeq* findSquares4( const int thresh = 50, const int N=11 );
	
	/**
	* depreciada
	*
	*/
	void drawPolys( ) throw (polysError);
	
	/**
	* depreciada
	*
	*/	
	void matrix( );
	
	/**
	* obtiene la firma del objeto.
	*
	* @param ptv vector de puntos representando el objeto
	* @param deltaVV vector devuelto representando la matriz (firma)
	*
	*/
	void getSign ( const std::vector <CvPoint *> &ptV, 
		std::vector< std::vector<float> > &deltaVV );
		
	/**
	* pinta la matriz firma en un jpg.
	*
	* @param deltaVV vector matriz firma
	* @param filename nombre de archivo generado jpg.
	*
	*/
	void matrixPaint ( const std::vector< std::vector<float> > &deltaVV,
		const std::string &filename );
		
	/**
	* convierte la matriz a formato csv para ser recogida luego por la red neuronal
	* 
	* @param deltaVV matriz
	* @param filename nombre del archivo .csv generado
	*/
	void matrix2csv ( const std::vector< std::vector<float> > &deltaVV,
		const std::string &filename );
	
	/**
	* encuentra n puntos del contorno de la figura contorno y devuelve n puntos "muestreados"
	* equidistantes
	* 
	* @param contorno CvSeq del contorno del objeto de marras
	* @param ptV vector de CvPoint con el contorno "muestreado"
	*/
	void findPoints ( const CvSeq* contorno, int n, std::vector <CvPoint *> &ptV );		

	~polys();
	
private:
	/**
	* calcula el promedio de nivel de img en la ventana definida por x0,yo,x1,y1
	* y lo retorna en sigma.
	*
	*/
	double 	winLevel ( IplImage* img, unsigned int x0, unsigned int y0, 
		unsigned int x1, unsigned int y1, double &sigma );
		
	/**
	* distancia de p0 a pf
	*
	*/
	float distance ( const CvPoint &p0, const CvPoint &pf );
	
	/**
	* adjustXY
	*
	*/
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

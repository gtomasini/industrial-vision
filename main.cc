#include "polys.h"
#include <iostream>
#include <stdlib.h>

using namespace std;

int 
main(int argc, char** argv){
   
    if (argc==1){
    	cout<< "Escriba "<<argv[0]<<" pic1.jpg\n"<<endl;
		return -1;
	}

	int x(250),y(350),a(230),b(150),k(4),sides(6),delay(1000),area(30);
 	int nroMuestras(10), opt;      
   	while ((opt = getopt(argc, argv, "x:y:a:A:b:k:s:d:n:")) != -1) {
    	switch (opt) {
            case 'x':
                x = atoi(optarg);
                break;
				
			case 'y':
            	y = atoi(optarg);
                break;
				
			case 'a':
				a= atoi(optarg);
				break;
				
			case 'A':
				area=atoi(optarg);
				break;
				
			case 'b':
				b = atoi(optarg);
				break;

			case 'k':
				k = atoi(optarg);
				break;
				
			case 'd':
				delay = atoi(optarg);
				break;
				
			case 'n':
				nroMuestras = atoi(optarg);
				break;
				
			default: /* '?' */
                cerr<<"Usage: "<<argv[0]<<"[-x x0] [-y yo] [-a a0] [-b b0]"
					<<" [-k k0] [-s	sides] [-d	delay] file.bmp."<<endl;
                exit(EXIT_FAILURE);
        }
    }
	if (optind >= argc) {
       cerr<< "Expected argument after options"<<endl;
       exit(EXIT_FAILURE);
    }

	//ventana
	CvPoint p0, p1;
	p0.x=x;
	p0.y=y;
	p1.x=x+a;
	p1.y=y+b;
	
	polys myPolys ( argv[optind], p0, p1, k, delay );
	
	CvSeq *secuencia(myPolys.findObject( area ));
	if (secuencia==0) return -1;
	
	vector <CvPoint *> ptV;//acà devuelve el vector de contorno
	myPolys.findPoints ( secuencia, nroMuestras, ptV ); 
	
	time_t t0, tf;
	t0 = time( NULL );
	
	vector< vector<float> > deltaVV;
	myPolys.getSign ( ptV, deltaVV );
	tf = time( NULL );
	
	cout<<"tf-t0="<<tf-t0<<endl;
	
	myPolys.matrixPaint ( deltaVV, "matrix.bmp" );
	myPolys.matrix2csv ( deltaVV, "salida.csv" );
	return 0;
}

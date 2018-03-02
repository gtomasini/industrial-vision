#include "cv.h"
#include "highgui.h"
#include <stdio.h>

char name0[]="images/example.bmp";
char name1[]="images/example.jpg";

int main(){
	IplImage *img0=cvLoadImage ( name0, -1 );
	IplImage *img1=cvLoadImage ( name1, -1 );
	
	cvNamedWindow ("image0", 1 );
	cvNamedWindow ("image1", 1 );

	cvShowImage ( "image0", img0 );
	cvShowImage ( "image1", img1 );
	
	cvWaitKey(0);
	
	cvReleaseImage ( &img0 );
	cvReleaseImage ( &img1 );

	return 0;
	
}

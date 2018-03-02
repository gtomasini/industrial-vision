#include "cv.h"
#include "highgui.h"
#include <stdio.h>

int main(){

	CvSize imgSize;
	imgSize.width = 640;
	imgSize.height = 480;
	IplImage *cvImg( cvCreateImage ( imgSize, 8, 1) );	
	
	for (int i=0; i<imgSize.width; ++i)
		for (int j=0; j<imgSize.height; ++j)
			((uchar *)(cvImg->imageData + cvImg->widthStep*j))[i]=
				(char) ((i*j)%256);

	cvNamedWindow ( "Testing OpenCV...", 1 );
	cvShowImage ( "Testing OpenCV...", cvImg );
	
	cvWaitKey ( 0);
	cvDestroyWindow ("image");
	cvReleaseImage( &cvImg );
	
	return (0);	
}

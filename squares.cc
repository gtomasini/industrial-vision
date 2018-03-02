//
// The full "Square Detector" program.
// It loads several images subsequentally and tries to find squares in
// each image
//
#ifdef _CH_
#pragma package <opencv>
#endif

#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

#include <unistd.h>

const int thresh = 50;
// try several threshold levels
const int N=11;

const char* wndname = "Square Detection Demo";

int _brightness = 100;
int _contrast = 100;

int hist_size = 64;
float range_0[]={0,256};
float* ranges[] = { range_0 };
IplImage *src_image = 0, *dst_image = 0, *hist_image = 0;
CvHistogram *hist;
uchar lut[256];
CvMat* lut_mat;

void update_brightcont( int arg );
// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2 
double 
angle( CvPoint* pt1, CvPoint* pt2, CvPoint* pt0 ){
    double dx1 = pt1->x - pt0->x;
    double dy1 = pt1->y - pt0->y;
    double dx2 = pt2->x - pt0->x;
    double dy2 = pt2->y - pt0->y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

// returns sequence of squares detected on the image.
// the sequence is stored in the specified memory storage
CvSeq* 
findSquares4( IplImage* img, CvMemStorage* storage ){

    CvSize sz = cvSize( img->width & -2, img->height & -2 );
    IplImage* timg = cvCloneImage( img ); // make a copy of input image
    IplImage* gray = cvCreateImage( sz, 8, 1 ); 
    IplImage* pyr = cvCreateImage( cvSize(sz.width/2, sz.height/2), 8, 3 );
	
    // create empty sequence that will contain points -
    // 4 points per square (the square's vertices)
    CvSeq* squares = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvPoint), storage );
    
    // select the maximum ROI in the image
    // with the width and height divisible by 2
    cvSetImageROI( timg, cvRect( 0, 0, sz.width, sz.height ));
    
    // down-scale and upscale the image to filter out the noise
    cvPyrDown( timg, pyr, 7 );
    cvPyrUp( pyr, timg, 7 );
    IplImage* tgray = cvCreateImage( sz, 8, 1 );
    
   
	
// find squares in every color plane of the image
    for( int c = 0; c < 3; c++ ){
        // extract the c-th color plane
        cvSetImageCOI( timg, c+1 );
        cvCopy( timg, tgray, 0 );
        
//		cvShowImage( wndname, tgray );
		 
//		cvWaitKey(0);

	src_image=cvCloneImage(tgray);
	dst_image = cvCloneImage(src_image);
    hist_image = cvCreateImage(cvSize(320,200), 8, 1);
    hist = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);
    lut_mat = cvCreateMatHeader( 1, 256, CV_8UC1 );
    cvSetData( lut_mat, lut, 0 );

    cvNamedWindow("image", 0);
//    cvNamedWindow("histogram", 0);

    cvCreateTrackbar("brightness", "image", &_brightness, 200, update_brightcont);
    cvCreateTrackbar("contrast", "image", &_contrast, 200, update_brightcont);

    update_brightcont(0);
    cvWaitKey(0);
    
//    cvReleaseImage(&src_image);

	cvReleaseHist(&hist);
	cvReleaseImage (&tgray);
	tgray=cvCloneImage(dst_image);
	
    cvReleaseImage(&dst_image);
		
	//  cvShowImage( wndname, dst_image );
	//  cvWaitKey(0);
        for( int l = 0; l < N; l++ ){
            // hack: use Canny instead of zero threshold level.
            // Canny helps to catch squares with gradient shading   
			
            if( l == 0 ) {
                // apply Canny. Take the upper threshold from slider
                // and set the lower to 0 (which forces edges merging) 
                cvCanny( tgray, gray, 0, thresh, 5 );
                // dilate canny output to remove potential
                // holes between edge segments 
                cvDilate( gray, gray, 0, 1 );
            }
            else {
                // apply threshold if l!=0:
                //     tgray(x,y) = gray(x,y) < (l+1)*255/N ? 255 : 0
                cvThreshold( tgray, gray, (l+1)*255/N, 255, CV_THRESH_BINARY );
            }
            
            // find contours and store them all as a list
			CvSeq* contours;
            cvFindContours( gray, storage, &contours, sizeof(CvContour),
            CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );
            
            // test each contour
            while( contours ) {
                // approximate contour with accuracy proportional
                // to the contour perimeter
                CvSeq *result = cvApproxPoly( contours, sizeof(CvContour), storage,
                    CV_POLY_APPROX_DP, cvContourPerimeter(contours)*0.02, 0 );
                // square contours should have 4 vertices after approximation
                // relatively large area (to filter out noisy contours)
                // and be convex.
                // Note: absolute value of an area is used because
                // area may be positive or negative - in accordance with the
                // contour orientation
                if( result->total == 4 &&
                    fabs(cvContourArea(result,CV_WHOLE_SEQ)) > 1000 &&
                    cvCheckContourConvexity(result) )  {
                    double s (0);
                    
					
                    for( int i = 0; i < 5; i++ ){
                        // find minimum angle between joint
                        // edges (maximum of cosine)
                        if( i >= 2 ){
                            double t = fabs(angle(
                            (CvPoint*)cvGetSeqElem( result, i ),
                            (CvPoint*)cvGetSeqElem( result, i-2 ),
                            (CvPoint*)cvGetSeqElem( result, i-1 )));
                            s = s > t ? s : t;
                        }
                    }
                    
                    // if cosines of all angles are small
                    // (all angles are ~90 degree) then write quandrange
                    // vertices to resultant sequence 
                    if( s < 0.3 )
                        for( int i = 0; i < result->total; i++ )
                            cvSeqPush( squares,
                                (CvPoint*)cvGetSeqElem( result, i ));
                }
                
                // take the next contour
                contours = contours->h_next;
            }
        }
    }
    
    // release all the temporary images
    cvReleaseImage( &gray );
    cvReleaseImage( &pyr );
    cvReleaseImage( &tgray );
    cvReleaseImage( &timg );
    
    return squares;
}


// the function draws all the squares in the image
void
drawSquares( IplImage* img, CvSeq* squares ){

    IplImage* cpy = cvCloneImage( img );
    
    // initialize reader of the sequence
    CvSeqReader reader;
    cvStartReadSeq( squares, &reader, 0 );
    
    // read 4 sequence elements at a time (all vertices of a square)
    for( int i = 0; i < squares->total; i += 4 )  {
        CvPoint pt[4];
        
        // read 4 vertices
        CV_READ_SEQ_ELEM( pt[0], reader );
        CV_READ_SEQ_ELEM( pt[1], reader );
        CV_READ_SEQ_ELEM( pt[2], reader );
        CV_READ_SEQ_ELEM( pt[3], reader );
        
		int count = 4;
	    CvPoint *rect = pt;
        // draw the square as a closed polyline 
        cvPolyLine( cpy, &rect, &count, 1, 1, CV_RGB(0,255,0), 3, CV_AA, 0 );
    }
    
    // show the resultant image
    cvShowImage( wndname, cpy );
    cvReleaseImage( &cpy );
}

/* brightness/contrast callback function */
void update_brightcont( int arg ){

    int brightness = _brightness - 100;
    int contrast = _contrast - 100;
    int i, bin_w;
    float max_value = 0;

    /*
     * The algorithm is by Werner D. Streidt
     * (http://visca.com/ffactory/archives/5-99/msg00021.html)
     */
    if( contrast > 0 ){
        double delta = 127.*contrast/100;
        double a = 255./(255. - delta*2);
        double b = a*(brightness - delta);
        for( i = 0; i < 256; i++ ){
            int v = cvRound(a*i + b);
            if( v < 0 )  v = 0;
            if( v > 255 )  v = 255;
            lut[i] = (uchar)v;
        }
    }
    else  {
        double delta = -128.*contrast/100;
        double a = (256.-delta*2)/255.;
        double b = a*brightness + delta;
        for( i = 0; i < 256; i++ ) {
            int v = cvRound(a*i + b);
            if( v < 0 )  v = 0;
            if( v > 255 )  v = 255;
            lut[i] = (uchar)v;
        }
    }

    cvLUT( src_image, dst_image, lut_mat );
    cvShowImage( "image", dst_image );
#if 0
    cvCalcHist( &dst_image, hist, 0, NULL );
    cvZero( dst_image );
    cvGetMinMaxHistValue( hist, 0, &max_value, 0, 0 );
    cvScale( hist->bins, hist->bins, ((double)hist_image->height)/max_value, 0 );
    /*cvNormalizeHist( hist, 1000 );*/

    cvSet( hist_image, cvScalarAll(255), 0 );
    bin_w = cvRound((double)hist_image->width/hist_size);

    for( i = 0; i < hist_size; i++ )
        cvRectangle( hist_image, cvPoint(i*bin_w, hist_image->height),
                     cvPoint((i+1)*bin_w, hist_image->height - cvRound(cvGetReal1D(hist->bins,i))),
                     cvScalarAll(0), -1, 8, 0 );
   
    cvShowImage( "histogram", hist_image );
#endif
}


int 
main(int argc, char** argv){
//    int i, c;
   
    if (argc==1){
    	printf ( "Escriba %s pic1, pic2...\n", argv[0] );
		return -1;
	}

    // create memory storage that will contain all the dynamic data
    CvMemStorage* storage = cvCreateMemStorage(0);

    // load i-th image
    IplImage* img0 = cvLoadImage( argv[1], 1 );
		
    if( !img0 ){
            printf( "Couldn't load %s\n", argv[1] );
		return -1;
    }
		
	IplImage* img = cvCloneImage( img0 );
      
    // create window and a trackbar (slider) with parent "image" and set callback
    // (the slider regulates upper threshold, passed to Canny edge detector) 
    cvNamedWindow( wndname, 1 );
        // find and draw the squares
        drawSquares( img, findSquares4( img, storage ) );
        
        // wait for key.
        // Also the function cvWaitKey takes care of event processing
    int c = cvWaitKey(0);
        // release both images
        cvReleaseImage( &img );
        cvReleaseImage( &img0 );
        // clear memory storage - reset free space position
        cvClearMemStorage( storage );
    
 
    cvDestroyWindow( wndname );
    
    return 0;
}

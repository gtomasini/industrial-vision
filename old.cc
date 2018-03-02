 
// returns sequence of polys detected on the image.
// the sequence is stored in the specified memory storage
CvSeq* 
polys::findSquares4( const int thresh, const int N ){
    CvSize sz = cvSize( img0_->width & -2, img0_->height & -2 );
    IplImage* timg = cvCloneImage( img0_ ); // make a copy of input image
    IplImage* gray = cvCreateImage( sz, 8, 1 ); 
    IplImage* pyr = cvCreateImage( cvSize(sz.width/2, sz.height/2), 8, 3 );
	
    // create empty sequence that will contain points -
    // 4 points per square (the square's vertices)
    CvSeq* squares = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvPoint), storage_ );
    
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
        
		cvShowImage( wndname_, tgray );
		cvWaitKey( delay_ );
	
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
            cvFindContours( gray, storage_, &contours, sizeof(CvContour),
	            CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );
            
            // test each contour
            while( contours ) {
                // approximate contour with accuracy proportional
                // to the contour perimeter
                CvSeq *result = cvApproxPoly( contours, sizeof(CvContour), storage_,
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
polys::drawPolys( ) throw (polysError){
    IplImage* cpy = cvCloneImage( img0_ );
    
    // initialize reader of the sequence
    CvSeqReader reader;
    cvStartReadSeq( polys_, &reader, 0 );
	//CvPoint pt[64];
	
	int i=0;
	
	for( int j = 0; j < (polys_->total)/sides_; j++  ){
		cout<<"drawPolys(), j="<<j<<endl;
		if ( polys_->total > ptArraySize_ )
			throw polysError ( "array limit reached" );
		for( ;i < polys_->total; ++i )  {
		    CV_READ_SEQ_ELEM( ptArray_[i], reader );
			cout<<"p["<<i<<"]: "<<ptArray_[i].x<<","<<ptArray_[i].y<<" ";
		}
		cout<<endl;	
		CvPoint *pol = ptArray_;
		// draw the square as a closed polyline 
   		cvPolyLine( cpy, &pol, &sides_, 1, 1, CV_RGB(0,255,0), 3, CV_AA, 0 );
		
		cvShowImage( wndname_, cpy );
		cvSaveImage ( "salida.bmp", cpy );
		cvWaitKey( delay_ );
	}
		
	cvReleaseImage( &cpy );	
}

void
polys::matrix( ){

	//int N=polys_->total;
	/*for( int j = 0; j < polys_->total/sides_; j++  )*/
	
	cout<<"matrix(): sides_="<<sides_<<", polys_->total="
			<<polys_->total<<endl;
		
	vector< vector<float> > areaVV;
		
	for( int i=0; i < sides_; ++i )  {
		cout<<"i="<<i<<endl;
		vector<float> areaV;		
		//for (int k=1; k<( lados_)/2; ++k){
		int K=(sides_-1)/2;
		cout<<"p["<<i<<"]: "<<ptArray_[i].x<<","<<ptArray_[i].y<<", K:"<<K<<endl;
		for (int k=1; k<= K; ++k){
			int m(i-k);
			int n(i+k);
				
			m=(m<0)?sides_+m:m;
			n=(n>=sides_)?n-sides_:n;
							
			float a(distance( ptArray_[i],ptArray_[n]));
				float b(distance( ptArray_[n],ptArray_[m]));
				float c(distance( ptArray_[i],ptArray_[m]));

				float p((a+b+c)/2);
				float area( sqrtf(p*(p-a)*(p-b)*(p-c) ) );
				
				areaV.push_back(area);
				
				cout<<"i="<<i<<", m="<<m<<", n="<<n<<" area:"<<area<<"; ";
		}
			areaVV.push_back (areaV);  
			cout<<endl;
	
	}//for (i );
		
	vector < vector < float > > deltaVV; 
	float mayor(0);		
	for (int ii=0; ii<areaVV.size(); ++ii){
			vector <float> areaV=areaVV[ii];
			vector <float>	deltaV;
			for ( int jj=0; jj<areaV.size() ; ++jj )
				for ( int kk=0; kk<areaV.size() ; ++kk )
					if (jj != kk ){
						float d(areaV[jj]/areaV[kk]);
						if (d>mayor) mayor=d;
						deltaV.push_back (d);
						cout<<d<<" ";
					}
			
			deltaVV.push_back (deltaV);
			cout<<endl;
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
	cvNamedWindow( "matrix", 1 );	
	IplImage  *tgray2(cvCreateImage(sz, 8, 1 ));
		
	cvResize ( tgray, tgray2 );	
	cvShowImage( "matrix", tgray2 );
	char filename[128];
//		sprintf ( filename, "matrix.bmp", j );
		
	cvSaveImage( "matrix.bmp", tgray2 );
	cvWaitKey( delay_ );
		
	cvReleaseImage ( &tgray );
	cvReleaseImage ( &tgray2 );
	
}

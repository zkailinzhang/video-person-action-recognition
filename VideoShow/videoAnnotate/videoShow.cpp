// videoAnnotate.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "cv.h"
#include "highgui.h"
#include <cmath>
#include <vector>
#include "DepthMapBinFileIO.h"


typedef struct{
	int objClass;	// 0: drink, 1: food, 2: laptop, 3: cellphone, 4: cellphone, 5: book, 6: remoter, 101: head...
	int status;		// -1: without any label,   0: indicates this is a direct label;		1: indicates this is a interpolation result
	CvPoint p1;		// top-left corner
	CvPoint p2;		// bottom-right corner
}
RectperFrame;


typedef struct{
	float x, y, z;
} Point3D;


 


using namespace std;

CvCapture* capture = NULL;
IplImage* frame = NULL;
IplImage *depthFrame = NULL;
IplImage *skeletonFrame = NULL;

int curFrameInd = -1;
int totalNumFrames;
int increFrame = 0;		// an indicator
int mouseDown = 0;
int objectClass = -1;
const int NUM_SKELETON_POINTS = 20;
double averageDepth = 0;

char *trackerBar = "Frame";
char *window = "RGB Frames";
char *windowDepth = "Depth Frames";
char *windowSkeleton = "Skeleton Frames";

char videoName[500];
char videoFullName[500];
char depthFullName[500];
char skeletonFullName[500];
char labelFullName[500];

vector< IplImage *> imageBuffer;
vector< RectperFrame > recBuffer;
vector< IplImage *> depthBuffer;
vector< vector< Point3D > > skeletonBuffer;

CvScalar color = cvScalar( 255, 0, 0 );

int skeletonDrawRelation[2][19] ={ 3, 2, 1, 0, 12, 13, 14,  0, 16, 17, 18, 2, 4, 5, 6, 2, 8, 9,  10,  
	                               2, 1, 0,12, 13, 14, 15, 16, 17, 18, 19, 4, 5, 6, 7, 8, 9, 10, 11};

void drawSkeletonPoints();

void drawFrame( IplImage *curFrame, int frameInd )
{
	if( frame != NULL )
		cvReleaseImage( &frame );
	
	frame = cvCreateImage( cvGetSize( curFrame ), IPL_DEPTH_8U, 3 );
	cvCopy( curFrame, frame );

	// draw rectangle if it is available
	if( recBuffer[ frameInd ].status != -1 ){
		cvRectangle( frame, recBuffer[ frameInd ].p1, recBuffer[ frameInd ].p2, color, 2 );
	}


	// draw depth frame
	if( depthBuffer.size() > 0 ){
		depthFrame = depthBuffer[ frameInd ];
	}

	curFrameInd = frameInd;

	if( skeletonBuffer.size() > 0 )
		drawSkeletonPoints();
}


void callBackForTrackerBar( int pos )
{
	if( increFrame == 1 ){
		increFrame = 0;
		return;
	}

	int curFrameNum = cvGetTrackbarPos( trackerBar, window );

	if( curFrameNum < totalNumFrames )		
		drawFrame( imageBuffer[curFrameNum], curFrameNum );		
							
}



void loadLabelResultsVersion1( FILE *flabel )
{
	int nFrames;
	fscanf( flabel, "version1 %d\n", &nFrames );
	if( nFrames != totalNumFrames ){
		printf( "Wrong input label format\n" );
		exit(-1);
	}

	int i;	
	for( i = 0; i < totalNumFrames; ++i ){
		fscanf( flabel, "%d %d %d %d %d %d\n", &(recBuffer[i].status), &(recBuffer[i].objClass), &(recBuffer[i].p1.x), &(recBuffer[i].p1.y), &(recBuffer[i].p2.x), &(recBuffer[i].p2.y ) );
	}
	fclose( flabel );

	
}


void drawSkeletonPoints()
{
		//curFrameInd
	if( skeletonFrame == NULL ){
		skeletonFrame = cvCreateImage( cvSize( 320, 240 ), IPL_DEPTH_8U, 3 );		
	}
	cvSetZero( skeletonFrame );

	int i;
	for( i = 0; i < NUM_SKELETON_POINTS-1; ++i ){
		CvPoint p1, p2;

		if( skeletonBuffer[curFrameInd][ skeletonDrawRelation[0][i] ].x < 2.0 ){
			p1.x = cvRound( 240 * skeletonBuffer[curFrameInd][ skeletonDrawRelation[0][i] ].x );
			p1.y = cvRound( 320 * skeletonBuffer[curFrameInd][ skeletonDrawRelation[0][i] ].y );
			p2.x = cvRound( 240 * skeletonBuffer[curFrameInd][ skeletonDrawRelation[1][i] ].x );
			p2.y = cvRound( 320 * skeletonBuffer[curFrameInd][ skeletonDrawRelation[1][i] ].y );
		/*	p1.x = cvRound( 320 * skeletonBuffer[curFrameInd][ skeletonDrawRelation[0][i]-1 ].x );
			p1.y = cvRound( 240 * skeletonBuffer[curFrameInd][ skeletonDrawRelation[0][i]-1 ].y );
			p2.x = cvRound( 320 * skeletonBuffer[curFrameInd][ skeletonDrawRelation[1][i]-1 ].x );
			p2.y = cvRound( 240 * skeletonBuffer[curFrameInd][ skeletonDrawRelation[1][i]-1 ].y );*/
		}
		else{
			p1.x = skeletonBuffer[curFrameInd][ skeletonDrawRelation[0][i] ].x;
			p1.y = skeletonBuffer[curFrameInd][ skeletonDrawRelation[0][i] ].y;
			p2.x = skeletonBuffer[curFrameInd][ skeletonDrawRelation[1][i] ].x;
			p2.y = skeletonBuffer[curFrameInd][ skeletonDrawRelation[1][i] ].y;
		}


		/*p1.x = cvRound( averageDepth * skeletonBuffer[curFrameInd][ skeletonDrawRelation[0][i]-1 ].x / skeletonBuffer[curFrameInd][ skeletonDrawRelation[0][i]-1 ].z );
		p1.y = cvRound( averageDepth * skeletonBuffer[curFrameInd][ skeletonDrawRelation[0][i]-1 ].y / skeletonBuffer[curFrameInd][ skeletonDrawRelation[0][i]-1 ].z );
		p2.x = cvRound( averageDepth * skeletonBuffer[curFrameInd][ skeletonDrawRelation[1][i]-1 ].x / skeletonBuffer[curFrameInd][ skeletonDrawRelation[1][i]-1 ].z );
		p2.y = cvRound( averageDepth * skeletonBuffer[curFrameInd][ skeletonDrawRelation[1][i]-1 ].y / skeletonBuffer[curFrameInd][ skeletonDrawRelation[1][i]-1 ].z );*/
		cvDrawLine( skeletonFrame, p1, p2, cvScalar( 0, 255, 255 ), 2 );
	}

	// draw the skeleton points only
	CvFont font;
	char buffer[100];
	
	cvInitFont( &font, CV_FONT_HERSHEY_SIMPLEX, 0.2, 0.4 );
	
	for( i = 0; i < NUM_SKELETON_POINTS; ++i ){
			CvPoint p1;

			if( skeletonBuffer[curFrameInd][ i ].x < 2.0 ){
				p1.x = cvRound( 240 * skeletonBuffer[curFrameInd][ i ].x-4 );
				p1.y = cvRound( 320 * skeletonBuffer[curFrameInd][ i ].y+1 );			
			}
			else{
				p1.x = skeletonBuffer[curFrameInd][ i ].x-4;
				p1.y = skeletonBuffer[curFrameInd][ i ].y+1;				
			}
			//sprintf( buffer, "%d", i );
			sprintf( buffer, "%c", '+' );
			cvPutText(skeletonFrame, buffer, p1, &font, cvScalar(255, 255, 0));
	}

	// draw the objec label if it is possible
	if( recBuffer[ curFrameInd ].status != -1 ){
		CvPoint p1, p2;
		p1.x = recBuffer[ curFrameInd ].p1.x / 2;
		p1.y = recBuffer[ curFrameInd ].p1.y / 2;
		p2.x = recBuffer[ curFrameInd ].p2.x / 2;
		p2.y = recBuffer[ curFrameInd ].p2.y / 2;
		cvRectangle( skeletonFrame, p1, p2, color, 2 );
	}



}



//int _tmain(int argc, _TCHAR* argv[])
int main(int argc, char* argv[])
{
	int sub_id;
	int cls_id;
	int epi_id;

	int set_id;

	if( argc != 6 ){
		printf( "usage:  sub_id  cls_id   epi_id  set_id  videopath\n" );
		return -1;
	}

	char videoPath[300];
 
	cls_id = atoi( (char*) argv[1] );
	sub_id = atoi( (char*) argv[2] );
	epi_id = atoi( (char*) argv[3] );	
	set_id = atoi( (char*) argv[4] );	
	strcpy( videoPath, (char*) argv[5] );
		 
	sprintf( videoName, "%s\\S%d\\a%02d_s%02d_e%02d", videoPath, set_id, cls_id, sub_id, epi_id );		 		 
	sprintf( videoFullName, "%s_rgb.avi", videoName );
	sprintf( depthFullName, "%s_depth.bin", videoName );
	sprintf( skeletonFullName, "%s_skeleton.txt", videoName );
	sprintf( labelFullName, "%s_rgb.avi.label.txt", videoName );

	//==================================================
	// load the RGB frames
	//==================================================
    cvNamedWindow( window, CV_WINDOW_AUTOSIZE);   	
    capture = cvCreateFileCapture( videoFullName );
	totalNumFrames = cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_COUNT );
	int startPos = 0;
	int imageWidth = cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH );
	int imageHeight = cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT  );

	cvCreateTrackbar( trackerBar, window, &startPos, totalNumFrames-1, callBackForTrackerBar );	
    
	// load all the image into buffer
	int i, j;
	imageBuffer.resize( totalNumFrames );
	for( i = 0; i < totalNumFrames; ++i ){
		if( i < totalNumFrames-2 )
			frame = cvQueryFrame(capture);
		imageBuffer[i] = cvCreateImage( cvSize( imageWidth, imageHeight ), IPL_DEPTH_8U, 3 );
		cvCopy( frame, imageBuffer[i] );
	}
	
	// load the object label if it is avaible
	recBuffer.resize( totalNumFrames );
	for( i = 0; i < totalNumFrames; ++i )
		recBuffer[i].status = -1;

	FILE *flabel = fopen( labelFullName, "r" );
	if( flabel != NULL ){			
		loadLabelResultsVersion1( flabel );
	}

	frame = NULL;
	drawFrame( imageBuffer[0], 0 );	


	

	//==================================================
	// load the Skeleton frames
	//==================================================
	cvNamedWindow( windowSkeleton, CV_WINDOW_AUTOSIZE);   	
	FILE * fskeleton = fopen( skeletonFullName, "r");
	if( fskeleton != NULL ){
		int nSkeletonFrames;
		int nSkeletonPoints;
		fscanf( fskeleton, "%d %d", &nSkeletonFrames, &nSkeletonPoints );
		/*if( nSkeletonFrames != totalNumFrames || nSkeletonPoints != NUM_SKELETON_POINTS ){
			printf( "error skeleton file format\n" );
			exit(-3);
		}*/

		skeletonBuffer.resize( totalNumFrames );

		int linesPerFrame;
		int tmp;
		float ftmp;
	
		for( i = 0; i < min( totalNumFrames, nSkeletonFrames ); ++i ){
			fscanf( fskeleton, "%d", &linesPerFrame );		
			skeletonBuffer[i].resize( NUM_SKELETON_POINTS );

			// in case where no skeleton is detected
			if( linesPerFrame == 0 ){
				for( j = 0; j < NUM_SKELETON_POINTS; ++j ){	
					skeletonBuffer[i][j].x = 0;
					skeletonBuffer[i][j].y = 0;
					skeletonBuffer[i][j].z = 0;
				}
				continue;
			}


			for( j = 0; j < min( NUM_SKELETON_POINTS, linesPerFrame ); ++j ){	
				fscanf( fskeleton, "%f %f %f %d", &ftmp, &ftmp, &ftmp, &tmp );			// throw away the world coordinate
				fscanf( fskeleton, "%f %f %f %d\n", &(skeletonBuffer[i][j].x), &(skeletonBuffer[i][j].y), &(skeletonBuffer[i][j].z), &tmp );
				averageDepth+= skeletonBuffer[i][j].z;
				
			}
			for( j = NUM_SKELETON_POINTS; j < linesPerFrame/2; ++j ){
				fscanf( fskeleton, "%f %f %f %d", &ftmp, &ftmp, &ftmp, &tmp );			// throw away the world coordinate
				fscanf( fskeleton, "%f %f %f %d", &ftmp, &ftmp, &ftmp, &tmp );			 
			}		

		}
		fclose( fskeleton );

		for( i = nSkeletonFrames; i < totalNumFrames; ++i ){			
			skeletonBuffer[i].resize( NUM_SKELETON_POINTS );
			for( j = 0; j < NUM_SKELETON_POINTS; ++j ){	
					skeletonBuffer[i][j].x = 0;
					skeletonBuffer[i][j].y = 0;
					skeletonBuffer[i][j].z = 0;
			}
		}

		drawSkeletonPoints();

	}
	

	//==================================================
	// load the Depth frames
	//==================================================
	cvNamedWindow( windowDepth, CV_WINDOW_AUTOSIZE);   	
	FILE * fdepth = fopen( depthFullName, "rb");

	if(fdepth != NULL){	
		int nofs = 0; //number of frames conatined in the file (each file is a video sequence of depth maps)
		int ncols = 0;
		int nrows = 0;
		ReadDepthMapBinFileHeader( fdepth, nofs, ncols, nrows );

		printf("number of frames=%i\n", nofs);
		/*if( nofs != totalNumFrames ){
			printf( "wrong depth file format\n" );
			exit( -2 );
		}*/

		depthBuffer.resize( totalNumFrames );

		//read each frame
		int f; //frame index
		for( f = 0; f < min( totalNumFrames, nofs); f++ )
		{
			depthBuffer[f] = cvCreateImage( cvSize( ncols, nrows ), IPL_DEPTH_8U, 1 );

			CDepthMap depthMap;
			depthMap.SetSize( ncols, nrows ); //it allocates space
			//the data will be stored in <depthMap>
			ReadDepthMapBinFileNextFrame( fdepth, ncols, nrows, depthMap );
			
			for( i = 0; i < nrows; ++i ){
				for( j = 0; j < ncols; ++j ){
				//	printf( "%f ", depthMap.GetItem( i, j ) );
					CV_IMAGE_ELEM( depthBuffer[f], uchar, i, j ) = (int)depthMap.GetItem( i, j );
				}
			}

		}

		fclose( fdepth );
		fdepth = NULL;

		depthFrame = depthBuffer[0];
	}




	
	
    /* Loop until frame ended or ESC is pressed */
    while(1) {       
		cvShowImage( window, frame );

		if( depthFrame ){
			cvShowImage( windowDepth, depthFrame );
		}
		if( skeletonFrame )
			cvShowImage( windowSkeleton, skeletonFrame );

        char c = cvWaitKey(33);
		if( c == 32 ){
			int curFrameNum = cvGetTrackbarPos( trackerBar, window );
			cvSetTrackbarPos( trackerBar, window, curFrameNum+1 );
			if( curFrameNum < totalNumFrames-1 ){
				drawFrame( imageBuffer[curFrameNum+1], curFrameNum+1 );					
				
			}
			else{
				drawFrame( imageBuffer[totalNumFrames-1], totalNumFrames-1 );			
			}
			increFrame = 1;
			
		}		
        else if( c == 27 )
			break;
    }

    cvReleaseCapture(&capture);
    cvDestroyWindow( window );
	cvDestroyWindow( windowDepth );
	cvReleaseImage( &skeletonFrame );

	if( frame )
		cvReleaseImage( &frame );

	for( int i = 0; i < totalNumFrames; ++i ){
		cvReleaseImage( &(imageBuffer[i] ) );
		if( depthBuffer[i] )
			cvReleaseImage( &(depthBuffer[i] ) );
	}
 
	return 0;
}


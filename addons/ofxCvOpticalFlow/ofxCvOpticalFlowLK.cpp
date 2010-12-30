//
// ofxCvOpticalFlowLK.h - a OpenCV cvOpticalFlowLK warpper for openFrameworks
//
// Copyright (C) 2008 Takashi Maekawa <takachin@generative.info> 
//     All rights reserved.
//     This is free software with ABSOLUTELY NO WARRANTY.
//
// You can redistribute it and/or modify it under the terms of 
// the GNU Lesser General Public License.
//

#include "ofxCvOpticalFlowLK.h"

ofxCvOpticalFlowLK::ofxCvOpticalFlowLK(void)
{
	captureWidth = DEFAULT_CAPTURE_WIDTH;
	captureHeight = DEFAULT_CAPTURE_HEIGHT;
	
	captureColsStep = DEFAULT_CAPTURE_COLS_STEP;
	captureRowsStep = DEFAULT_CAPTURE_ROWS_STEP;
}

ofxCvOpticalFlowLK::~ofxCvOpticalFlowLK(void)
{
	cvReleaseImage(&vel_x);
	cvReleaseImage(&vel_y);
}

void ofxCvOpticalFlowLK::allocate(int _w, int _h){
	captureWidth = _w;
	captureHeight = _h; 
	
	vel_x = cvCreateImage( cvSize( captureWidth ,captureHeight ), IPL_DEPTH_32F, 1  );
	vel_y = cvCreateImage( cvSize( captureWidth ,captureHeight ), IPL_DEPTH_32F, 1  );
	
    cvSetZero(vel_x);
    cvSetZero(vel_y);
}

void ofxCvOpticalFlowLK::setCalcStep(int _cols, int _rows){
	captureColsStep = _cols;
	captureRowsStep = _rows;
}

void ofxCvOpticalFlowLK::calc( ofxCvGrayscaleImage & pastImage,
							  ofxCvGrayscaleImage & currentImage,					   
							  int size
							  )
{
	cvCalcOpticalFlowLK( pastImage.getCvImage(), currentImage.getCvImage(),
						cvSize( size, size), vel_x, vel_y );
}


//void ofxCvOpticalFlowLK::getVelAtPixel(int x, int y, float *u, float *v) {
//	*u = cvGetReal2D( opticalFlow.vel_x, y, x );
//	*v = cvGetReal2D( opticalFlow.vel_y, y, x );
//}

ofPoint ofxCvOpticalFlowLK::getVelAtNorm(float x, float y) {
	int ix = x * captureWidth;
	int iy = y * captureHeight;
	if(ix<0) ix = 0; else if(ix>=captureWidth) ix = captureWidth - 1;
	if(iy<0) iy = 0; else if(iy>=captureHeight) iy = captureHeight - 1;
	return ofPoint(cvGetReal2D( vel_x, iy, ix ), cvGetReal2D( vel_y, iy, ix ));
}



void ofxCvOpticalFlowLK::draw(void){
	
	int x, y, dx, dy;
	for ( y = 0; y < captureHeight; y+=captureRowsStep ){
		for ( x = 0; x < captureWidth; x+=captureColsStep ){
			
			dx = (int)cvGetReal2D( vel_x, y, x );
			dy = (int)cvGetReal2D( vel_y, y, x );
			
			ofLine(x,y,x+dx*2,y+dy*2);
			
		}
	}
}

void ofxCvOpticalFlowLK::draw(float x, float y, float w, float h) {
	glPushMatrix();
	glTranslatef(x, y, 0);
	glScalef(w/captureWidth, h/captureHeight, 1.0);
	draw();
	glPopMatrix();
}

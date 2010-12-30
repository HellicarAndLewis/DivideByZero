#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"

#define _USE_FIREWIRE_		// uncomment this to use a live firewire camera
// otherwise, we'll the built in video device.... (iSight)

#ifdef _USE_FIREWIRE_
#include "ofxVideoGrabber.h"
#endif

#include "ofxControlPanel.h"
#include "renderManager.h"
#include "AppSettings.h"

#include "ofCvBlobTracker.h"

#include "ofxCvOpticalFlowLK.h"
#include "vectorField.h"

#include "ofxContourUtil.h"

#include "ofxMidi.h"


class testApp : public ofBaseApp, public ofCvBlobListener, public ofxMidiListener{

	public:

		void setup();
		void update();
		void draw();
	
		void eventsIn(guiCallbackData & data);
#ifdef _USE_FIREWIRE_	
		void cameraEventsIn(guiCallbackData & data);
#endif
		void calculateCaptureFramerate();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
	
		//from White Heat
		void randomiseGuiValueSafely(string whichGuiValue);
		void drawContourFinder(float xPos, float yPos, float drawWidth, float drawHeight);
	
		//from Augramp / ofxBlobTracker
	void blobOn( int x, int y, int bid, int order );
    void blobMoved( int x, int y, int bid, int order );    
    void blobOff( int x, int y, int bid, int order );
	
		//from julapy contour util

	void simplifyDP_openCV	( const vector<ofPoint>& contourIn, vector<ofPoint>& contourOut, float tolerance = 0.004 );
	void drawCurve			( const vector<ofPoint>& points, float xPos, float yPos, float drawWidth, float drawHeight, float drawLineWidth, float overallAlpha);
	void drawFittedCurve	( const vector<ofPoint>& points, float xPos, float yPos, float drawWidth, float drawHeight);	
	
		//midi messages
	void newMidiMessage(ofxMidiEventArgs& eventArgs);	
	
	void drawAura(float noneSmoothAlpha, float simplifyCVAlpha);
	void drawAim(float overallAlpha);
	void drawPassage(float overallAlpha);
	void drawField(float overallAlpha);
	void drawCycle(float overallAlpha);	
	
	void resetPassageWalls();

	
		//ofxvideo grabber

#ifdef _USE_FIREWIRE_
	ofxVideoGrabber 	vidGrabber;
	Libdc1394Grabber*	sdk;	
#else
	ofVideoGrabber 		vidGrabber;
#endif	
	
		int 				camWidth;
		int 				camHeight;
		int                 appWidth;
		int                 appHeight;

        /*framerate display*/
		char                buf[255];
		char                buf2[255];
		float 	            mytimeNow, mytimeThen, myframeRate;
		float 	            myfps;
		float               myframes;
	
		ofxControlPanel gui;
		
		float elapsedTime;
		int appFrameCount;
		float appFrameRate;
		
		ofTrueTypeFont TTF;
		
	
		//warp speed
		ofxCvGrayscaleImage		videoGrayscaleCvImage;
		ofxCvGrayscaleImage		videoBgImage;
		
		ofxCvGrayscaleImage		videoGrayscaleCvImagePreWarp;
		ofPoint		srcPts[4];
		ofPoint		dstPts[4];
		
		
		ofxCvGrayscaleImage		videoDiffImage;
		ofxCvContourFinder		videoContourFinder;	
	
		//mouse in my house
	bool bShowMouse;	
	
		//for warping out with the right debug look
	ofxCvGrayscaleImage allWhiteImage;
	ofxCvGrayscaleImage allBlackImage;
	bool bShowWhiteRect;
	
		//FBO warping now
	renderManager rm;
	
	ofRectangle guiIn;
	ofRectangle guiOut;	
	
		//progressive relearn background
	ofxCvFloatImage videoFloatBgImage;
	
		//from ofxBlobTracker
	ofCvBlobTracker		blobTracker;
	
	ofxCvGrayscaleImage stripToStretch;
	
		//optical flow and field
		// optival flow
	int						stepSize; //unused at the moment
	ofxCvOpticalFlowLK		flowImage;
//	vectorField				field; //unused at the moment
	
	ofxCvGrayscaleImage		prevImage;

		//contour analysis from julapy
	ofxContourUtil		cu;
	
	vector<ofxCvBlob>		curve;
	vector<ofxCvBlob>		curveSmooth;
	vector<ofxCvBlob>		curveSimplify;
	vector<ofxCvBlob>		curveCvSimplify;
	
	float				scale1;
	float				scale2;
	float				scale3;	
	
	ofxMidiIn midiIn;
//	MyMidiListener midiListener;	
	
	float timeOfSpace;
	
		//for bouncy walls
	float blobLeftPosition; //mouse x
	float blobLeftPreviousPosition;
	float leftPassage; //current vertical line position
	float velocityLeftPassage;
	
	float blobRightPosition; //mouse x	+ a constant
	float blobRightPreviousPosition;
	float rightPassage; //current vertical line position
	float velocityRightPassage;
	
	float blobTopPosition; //mouse y
	float blobTopPreviousPosition;
	float topPassage; //horizontal line position
	float velocityTopPassage; //velocity of the top passage
	
//	float gravity;
//	float coefficientOfRestitution;
//	float coefficientOfVelocity;	
};

#endif

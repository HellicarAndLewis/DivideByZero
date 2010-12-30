#include "testApp.h"

inline void spaceReplace(std::string & myStr)
{
	/* replace all spaces in the name to satisfy XML*/
	size_t found;
	found=myStr.find_first_of(" ");
	while (found!=string::npos)
	{
            //myStr.erase(found,1);
		myStr[found]='_';
		
		found=myStr.find_first_of(" ",found+1);
	}
}

//--------------------------------------------------------------
void testApp::setup(){
	
		// load the settings
	settings.loadSettings("AppSettings.xml");
	
		//mouse house
	
	bShowMouse = false;
	
		//ofxVideoGrabber
	
	ofSetFrameRate(30);
	ofEnableAlphaBlending();

	camWidth 		= 640;	// try to grab at this size.
	camHeight 		= 480;
	appWidth        = ofGetWidth();
	appHeight       = ofGetHeight();
	mytimeThen		= 0.0f;
	myframeRate     = 0.0f;
	myframes        = 0.0f;
	timeOfSpace		= ofGetElapsedTimef();

	ofSetVerticalSync(true);
	ofSetLogLevel(OF_LOG_VERBOSE);

#ifdef _USE_FIREWIRE_	
    vidGrabber.setDeviceID(0);

    sdk = new Libdc1394Grabber;
	//sdk->setFormat7(VID_FORMAT7_0);
	sdk->listDevices();
	sdk->setDiscardFrames(true);
	sdk->set1394bMode(false);
	//sdk->setROI(0,0,320,200);
	//sdk->setDeviceID("b09d01008bc69e:0");

	vidGrabber.setVerbose(true);
		//bool result = vidGrabber.initGrabber( camWidth, camHeight, VID_FORMAT_YUV422, VID_FORMAT_RGB, 30, true, sdk, settings );

    //bool result = vidGrabber.initGrabber( camWidth, camHeight, VID_FORMAT_YUV422, VID_FORMAT_RGB, 30 );
	// or like this:
	//Libsdk* dc1394Grabber = new Libsdk(); //stupid already above!
	bool result = vidGrabber.initGrabber( camWidth, camHeight, VID_FORMAT_GREYSCALE, VID_FORMAT_GREYSCALE, 30, true, sdk);
	// or like this:
	//bool result = vidGrabber.initGrabber( camWidth, camHeight, VID_FORMAT_YUV411, VID_FORMAT_RGB, 30, true, new Libsdk, new ofxIIDCSettings);



	if(result) {
	    ofLog(OF_LOG_VERBOSE,"Firewire Camera successfully initialized.");
	} else {
	    ofLog(OF_LOG_FATAL_ERROR,"Firewire Camera failed to initialize.");
	}
	
#else	
	
		//opencvexample
	vidGrabber.setVerbose(true);
	bool result = vidGrabber.initGrabber(camWidth,camHeight);
	
	if(result) {
		ofLog(OF_LOG_VERBOSE,"iSight or Native Camera successfully initialized.");
	} else {
		ofLog(OF_LOG_FATAL_ERROR,"iSight or Native Camera failed to initialize.");
	}
	
#endif		
	
		//warping
	
	videoGrayscaleCvImage.allocate(camWidth, camHeight);
	videoBgImage.allocate(camWidth, camHeight);
	videoDiffImage.allocate(camWidth, camHeight);
	videoGrayscaleCvImagePreWarp.allocate(camWidth, camHeight);
	videoFloatBgImage.allocate(camWidth, camHeight);	
	
	stripToStretch.allocate(1, camHeight);
		// set background color to be white: 
		//ofBackground(127, 127, 127);
	
	
	srcPts[0].set(0,0);
	srcPts[1].set(camWidth,0);
	srcPts[2].set(camWidth,camHeight);
	srcPts[3].set(0,camHeight);
	
	dstPts[0].set(0,0);
	dstPts[1].set(camWidth,0);
	dstPts[2].set(camWidth,camHeight);
	dstPts[3].set(0,camHeight);	
	
	
	
		//white image for warp debugging
	allWhiteImage.allocate(camWidth, camHeight);
	cvZero(allWhiteImage.getCvImage());
	allWhiteImage.invert();
		//black image for clearing
	allBlackImage.allocate(camWidth, camHeight);
	cvZero(allBlackImage.getCvImage());
	
		//FBO
	ofBackground(0,0,0);
    rm.allocateForNScreens(1, settings.PROJECTION_W, settings.PROJECTION_H);
    rm.loadFromXml("FBOSettings.xml");
	
    guiIn   = ofRectangle(10, 40, 640, 480);
    guiOut  = ofRectangle(guiIn.x + guiIn.width + 30, 40, 640, 480);
	
	bShowWhiteRect = false;		
	
		//optical flow
	
	stepSize  = 5;
	flowImage.setCalcStep(stepSize, stepSize);
	flowImage.allocate(camWidth/2, camHeight/2);	
//	field.setupField(20, 15, camWidth, camHeight);	//internal dim, external dims
	prevImage.allocate(camWidth/2, camHeight/2);


	
	
	//ofxControlPanel
	
	ofxControlPanel::setBackgroundColor(simpleColor(30, 30, 60, 200));
	ofxControlPanel::setTextColor(simpleColor(240, 50, 50, 255));
	
	gui.loadFont("MONACO.TTF", 8);		
	gui.setup("Divide by Zero by Hellicar&Lewis and Nina Umniakov", 0, 0, settings.GUI_W, settings.GUI_H);
	gui.addPanel("see", 4, false);			
	gui.addPanel("crop", 4, false);			
	gui.addPanel("background", 4, false);	
	gui.addPanel("blob", 4, false);			
	gui.addPanel("aura", 4, false);	
	gui.addPanel("aim", 4, false);		
	gui.addPanel("passage", 4, false);	
	gui.addPanel("field", 4, false);			

		//--------- PANEL See
	gui.setWhichPanel("see");
	
	gui.setWhichColumn(0);	
	
	gui.addSlider("SCENE 1: NOT USED Aura Fade", "AURA_FADE", 0, 0, 127, true);
	gui.addSlider("Aura Fade: NONE/SMOOTH", "AURA_FADE_NONE_SMOOTH", 0, 0, 127, true);
	gui.addSlider("Aura Fade: SIMPLIFY/CV", "AURA_FADE_SIMPLIFY_CV", 0, 0, 127, true);		
	gui.addSlider("SCENE 2: Aim Fade", "AIM_FADE", 0, 0, 127, true);
	gui.addSlider("SCENE 3: Passage Fade", "PASSAGE_FADE", 0, 0, 127, true);
	gui.addSlider("SCENE 4: Field Fade", "FIELD_FADE", 0, 0, 127, true);
	
		//some dummy vars we will update to show the variable lister object
	elapsedTime		= ofGetElapsedTimef();
	appFrameCount	= ofGetFrameNum();	
	appFrameRate	= ofGetFrameRate();
	
	vector <guiVariablePointer> vars;
	vars.push_back( guiVariablePointer("elapsed time", &elapsedTime, GUI_VAR_FLOAT, 2) );
	vars.push_back( guiVariablePointer("elapsed frames", &appFrameCount, GUI_VAR_INT) );
	vars.push_back( guiVariablePointer("app fps", &appFrameRate, GUI_VAR_FLOAT, 2) );
	
	vars.push_back( guiVariablePointer("mouse x", &mouseX, GUI_VAR_INT) );
	vars.push_back( guiVariablePointer("mouse y", &mouseY, GUI_VAR_INT) );
	
	gui.addVariableLister("app vars", vars);
	
	gui.addDrawableRect("Camera Image", &videoGrayscaleCvImagePreWarp, 200, 150);	
	
#ifdef _USE_FIREWIRE_	
		//fire wire camera settings
	
    sdk->getAllFeatureValues();
	gui.setWhichColumn(1);
	
	int tmp_index = -1;
	int tmp_featureID = -1;
	bool whitebalance = false;
	int featureID = -1;
	
    /* setup feature gui sliders */
    for (int i=0; i < sdk->availableFeatureAmount; ++i)
    {
        if(sdk->featureVals[i].isPresent) {
            featureID = sdk->featureVals[i].feature;
			
            if( featureID == FEATURE_WHITE_BALANCE) {
                tmp_index = i;
                tmp_featureID = featureID;
                whitebalance = true;
            }
            else if(featureID == FEATURE_TRIGGER) {
					//not using any features above this
                break;
            }
            else {
                string name = sdk->featureVals[i].name;
                string xmlName = name;
                transform(xmlName.begin(), xmlName.end(), xmlName.begin(), ::toupper);
				
                float minVal = sdk->featureVals[i].minVal;
                float maxVal = sdk->featureVals[i].maxVal;
                float currVal = sdk->featureVals[i].currVal;
                gui.addButtonSlider(name,xmlName, currVal, minVal, maxVal, true);
            }
        }
    }
	
    /* setup white balance 2d slider */
    if(whitebalance)
    {
        string name = sdk->featureVals[tmp_index].name;
        string xmlName = name;
        transform(xmlName.begin(), xmlName.end(), xmlName.begin(), ::toupper);
        spaceReplace(xmlName);
		
        float minVal = sdk->featureVals[tmp_index].minVal;
        float maxVal = sdk->featureVals[tmp_index].maxVal;
        float currVal = sdk->featureVals[tmp_index].currVal;
        float currVal2 = sdk->featureVals[tmp_index].currVal2;
        gui.addSlider2D(name,xmlName, currVal, currVal2, minVal, maxVal, minVal, maxVal, false);
		
    }
	
    gui.setWhichColumn(2);
	
    /* setup features modes */
    for (int i=0; i < sdk->availableFeatureAmount; ++i)
    {
        if(sdk->featureVals[i].isPresent) {
            featureID = sdk->featureVals[i].feature;
			
            if (featureID == FEATURE_TRIGGER)
            {
					//not using any features above this
                break;
            }
            if( featureID != FEATURE_WHITE_BALANCE)
            {
                string name = sdk->featureVals[i].name + " mode";
                string xmlName = name;
				
                transform(xmlName.begin(), xmlName.end(), xmlName.begin(), ::toupper);
                spaceReplace(xmlName);
				
                int defaultval = FEATURE_MODE_MANUAL;
				
					//                if(videoGrabber->featureVals[i].hasAutoModeActive) {
					//                    defaultval = FEATURE_MODE_AUTO;
					//					videoGrabber->setFeatureMode(FEATURE_MODE_AUTO, featureID );
					//                } else {
				sdk->setFeatureMode(FEATURE_MODE_MANUAL, featureID );
					//				}
                vector <string> modelist;
                modelist.push_back("MANUAL");
				
				
                if(sdk->featureVals[i].hasAutoMode)
                {
                    modelist.push_back("AUTO");
                    if(sdk->featureVals[i].hasOnePush )
                    {
                        modelist.push_back("ONE PUSH AUTO");
                    }
                }
				
                gui.addTextDropDown(name,xmlName, defaultval, modelist);
            }
        }
    }
	
    /* setup whitebalance modes */
    if(whitebalance)
    {
		featureID = FEATURE_WHITE_BALANCE;
		string name = sdk->featureVals[tmp_index].name + " mode";
		string xmlName = name;
		transform(xmlName.begin(), xmlName.end(), xmlName.begin(), ::toupper);
		spaceReplace(xmlName);
		
		int defaultval = FEATURE_MODE_MANUAL;
			//                if(videoGrabber->featureVals[tmp_index].hasAutoModeActive) {
			//                    defaultval = FEATURE_MODE_AUTO;
			//					videoGrabber->setFeatureMode(FEATURE_MODE_AUTO, featureID );
			//                } else {
		sdk->setFeatureMode(FEATURE_MODE_MANUAL, featureID );
			//				}
		vector <string> modelist;
		modelist.push_back("MANUAL");
		
		if(sdk->featureVals[tmp_index].hasAutoMode)
		{
			modelist.push_back("AUTO");
			if(sdk->featureVals[tmp_index].hasOnePush )
			{
				modelist.push_back("ONE PUSH AUTO");
			}
		}
		
		gui.addTextDropDown(name,xmlName, defaultval, modelist);
		
    }		
	
#endif	
	
		//------- PANEL Crop
	gui.setWhichPanel("crop");
	
	gui.setWhichColumn(0);
	gui.addSlider2D("pta", "WARP_CV_MANAGER_PANEL_VIDEO_PTA", 0, 0, 0, camWidth, 0, camHeight, true);
	gui.addSlider2D("ptd", "WARP_CV_MANAGER_PANEL_VIDEO_PTD", 0, camHeight, 0, camWidth, 0, camHeight, true);	
	
	gui.setWhichColumn(1);
	gui.addSlider2D("ptb", "WARP_CV_MANAGER_PANEL_VIDEO_PTB", camWidth, 0, 0, camWidth, 0, camHeight, true);
	gui.addSlider2D("ptc", "WARP_CV_MANAGER_PANEL_VIDEO_PTC", camWidth, camHeight, 0, camWidth, 0, camHeight, true);
	
	gui.setWhichColumn(2);
	gui.addDrawableRect("grey pre warp", &videoGrayscaleCvImagePreWarp, 200, 150);
	gui.addDrawableRect("grey warped", &videoGrayscaleCvImage, 200, 150);	
	
		//------- PANEL "background"
	gui.setWhichPanel("background");
	
	gui.setWhichColumn(0);
	gui.addToggle("learn background ", "WARP_B_LEARN_BG", true);
	gui.addToggle("progressive learn?", "WARP_B_LEARN_BG_PROGRESSIVE", true);	
	gui.addSlider("learn rate", "WARP_B_LEARN_BG_PROGRESSIVE_RATE", .1f, 0.0f, 1000.0f, false);			  
	gui.addSlider("threshold ", "WARP_THRESHOLD", 127, 0, 255, true);
	vector <string> warpDiffNames;
	warpDiffNames.push_back("abs diff");
	warpDiffNames.push_back("greater than");
	warpDiffNames.push_back("less than");
	gui.addTextDropDown("difference mode", "WARP_DIFF_MODE", 0, warpDiffNames);	
	
	gui.setWhichColumn(1);
	gui.addDrawableRect("grey pre warp", &videoGrayscaleCvImagePreWarp, 200, 150);
	gui.addDrawableRect("grey warped", &videoGrayscaleCvImage, 200, 150);
	gui.addDrawableRect("grey bg", &videoBgImage, 200, 150);	
	
	gui.setWhichColumn(2);
	gui.addDrawableRect("diff", &videoDiffImage, 200, 150);	
	
		//------- PANEL	"blob"
	gui.setWhichPanel("blob");

	gui.addDrawableRect("diff", &videoDiffImage, 200, 150);	
	gui.addDrawableRect("contour finder", &videoContourFinder, 200, 150);		
	
	gui.setWhichColumn(2);
	gui.addSlider("minimum blob size (% of view):", "WARP_MIN_BLOB_SIZE", 1.f, 0.01f, 10.0f, false);
	gui.addSlider("maximum blob size (% of view):", "WARP_MAX_BLOB_SIZE", 50.f, 1.f, 100.f, false);
		//guiTypeSlider * addSlider(string sliderName, string xmlName, float value , float min, float max, bool isInt);
	gui.addToggle("ignore nested blobs", "WARP_FIND_HOLES", false);	
	gui.addSlider("max num blobs to find ", "WARP_N_BLOBS_CONSIDERED", 3, 0, 20, true);	
	
	gui.setWhichColumn(0);

		//------- PANEL	"aura" outline of blob and shivering it...
	gui.setWhichPanel("aura");
	gui.setWhichColumn(0);

	gui.addSlider("aura width", "AURA_LINE_WIDTH", 1.f, 1.f, 20.f, false);
	gui.addToggle("scale from stage?", "AURA_SCALE_FROM_STAGE", false);
	gui.addToggle("aura filled?", "AURA_FILL", false);
	gui.addSlider("aura scale", "AURA_SCALE", 1.f, 1.f, 15.f, false);
	gui.setWhichColumn(2);
	gui.addSlider("aura simplification", "AURA_SIMPLIFICATION", 0.f, 0.f, 1.f, false);
	gui.addToggle("SMOOTH? Else NONE", "AURA_IS_SMOOTH", false);
	gui.addToggle("CV? Else SIMPLIFY", "AURA_IS_CV", false);	
			
	
		//------- PANEL 5 	"passage" bounding box of blob
	gui.setWhichPanel("passage");
	gui.setWhichColumn(0);
	gui.addToggle("Draw to head (Horiztonal Block)", "PASSAGE_HORIZONTAL", true);
	gui.addToggle("Draw to sides (Vertical Blocks)", "PASSAGE_VERTICAL", false);
	gui.addToggle("Invert", "PASSAGE_INVERT", false);
	gui.addToggle("Passage Snap (inertia will be ignored)", "PASSAGE_SNAP", false);
	gui.addToggle("Reset Passage positions", "PASSAGE_RESET", false);
	gui.addSlider("Passage Gravity", "PASSAGE_GRAVITY", 0.2f, 0.f, 1.f, false);
	gui.addSlider("Passage Bounce", "PASSAGE_COEFFICIENT_OF_RESTITUTION", 0.5f, 0.f, 1.f, false);
	gui.addSlider("Passage Strength of Hands", "PASSAGE_COEFFICIENT_OF_VELOCITY", 03.f, 0.f, 1.f, false);	
	gui.addToggle("Blocking Walls?", "PASSAGE_BLOCK_WALLS", false);
	
		//------- PANEL 7 	"aim" / slit stretch slit scan
	gui.setWhichPanel("aim");
	gui.setWhichColumn(0);
	gui.addSlider("Slit Height", "AIM_SLIT_HEIGHT", 0.01f, 0.01f, 1.f, false);	
	gui.addSlider("Slit Width", "AIM_SLIT_WIDTH", 0.01f, 0.01f, 1.f, false);
	gui.addToggle("Invert slit", "AIM_SLIT_INVERT", false);
	gui.addToggle("Slit Redness", "AIM_REDNESS", false);
	
		//------- PANEL 8 	"field" / optical flow
	gui.setWhichPanel("field");
	gui.setWhichColumn(0);
	gui.addSlider("Max Speed", "FIELD_MAX_SPEED", 3.0, 1.0, 20.0, false);
	gui.addSlider("Release Mag", "FIELD_RELEASE_MAG", 1.0, 0.0, 20.0, false);
	gui.addSlider("Vector Fade Rate", "FIELD_FADE_RATE", 0.4, 0.0, 1.0, false);
	gui.addSlider("Vector Limit", "FIELD_LIMIT", 3.0, 0.0, 10.0, false);

	//SETTINGS AND EVENTS
	
		//load from xml!
	gui.loadSettings("ControlPanelSettings.xml");
	
		//if you want to use events call this after you have added all your gui elements
	gui.setupEvents();
	gui.enableEvents();
	
		//  -- SPECIFIC EVENTS -- this approach creates an event group and only sends you events for the elements you describe. 
		//	vector <string> list;
		//	list.push_back("FIELD_DRAW_SCALE");
		//	list.push_back("DIFF_MODE");
		//	gui.createEventGroup("TEST_GROUP", list);
		//	ofAddListener(gui.getEventGroup("TEST_GROUP"), this, &testApp::eventsIn);
#ifdef _USE_FIREWIRE_	
		//  -- PANEL EVENTS -- this approach gives you back an ofEvent for only the events from panel 2, camera
	ofAddListener(gui.getEventsForPanel(0), this, &testApp::cameraEventsIn);
#endif	
		//  -- this gives you back an ofEvent for all events in this control panel object
	ofAddListener(gui.guiEvent, this, &testApp::eventsIn);
	
	
	blobTracker.setListener(this);
	
		//contour simplification
	scale1	= 0;
	scale2	= 0;
	scale3	= 0;	
	
		// This outputs the various ports
		// and their respective IDs.
	midiIn.listPorts();
	
		// Now open a port to whatever 
		// port ID your MIDI controller is on.
	midiIn.openPort(0); 
	
		// Add your MyMidiListener instance
		// as a listener.
	midiIn.addListener(this);	
	
		//ofEnableSmoothing();
	
		//reset the passage positions
	resetPassageWalls();	
}

	//--------------------------------------------------------------
void testApp::update(){

	ofBackground(0,0,0);
	
	vidGrabber.update();

	if (vidGrabber.isFrameNew()){
        calculateCaptureFramerate();
	}

    sprintf(buf,"App framerate : %f",ofGetFrameRate());

		//ofxControlPanel
	elapsedTime		= ofGetElapsedTimef();
	appFrameCount	= ofGetFrameNum();	
	appFrameRate	= ofGetFrameRate();

		//warper
	
	srcPts[0].set( gui.getValueI("WARP_CV_MANAGER_PANEL_VIDEO_PTA", 0), gui.getValueI("WARP_CV_MANAGER_PANEL_VIDEO_PTA", 1));
	srcPts[1].set( gui.getValueI("WARP_CV_MANAGER_PANEL_VIDEO_PTB", 0), gui.getValueI("WARP_CV_MANAGER_PANEL_VIDEO_PTB", 1));
	srcPts[2].set( gui.getValueI("WARP_CV_MANAGER_PANEL_VIDEO_PTC", 0), gui.getValueI("WARP_CV_MANAGER_PANEL_VIDEO_PTC", 1));
	srcPts[3].set( gui.getValueI("WARP_CV_MANAGER_PANEL_VIDEO_PTD", 0), gui.getValueI("WARP_CV_MANAGER_PANEL_VIDEO_PTD", 1));
	
	bool bLearnBg				= gui.getValueB("WARP_B_LEARN_BG");
	bool bLearnBgProgressive	= gui.getValueB("WARP_B_LEARN_BG_PROGRESSIVE");
	int threshold				= gui.getValueI("WARP_THRESHOLD");
	float minBlobSize			= (((float)gui.getValueI("WARP_MIN_BLOB_SIZE"))/100.f)*camWidth*camHeight;
	float maxBlobSize			= (((float)gui.getValueI("WARP_MAX_BLOB_SIZE"))/100.f)*camWidth*camHeight;
	int nBlobsConsidered		= gui.getValueI("WARP_N_BLOBS_CONSIDERED");
	bool bFindHoles				= gui.getValueB("WARP_FIND_HOLES");
	float rateOfProgression		= gui.getValueF("WARP_B_LEARN_BG_PROGRESSIVE_RATE") * .00001; //progressive from opentsps
	int warpDiffMode			= gui.getValueI("WARP_DIFF_MODE");
	
	if (vidGrabber.isFrameNew()){
		
		//videoColorCvImage.setFromPixels(video.getPixels(), width, height);
		
#ifdef _USE_FIREWIRE_			
		videoGrayscaleCvImagePreWarp.setFromPixels(vidGrabber.getPixels(), camWidth, camHeight);
#else
			//image will be in colour if we are using the isight
		
		ofxCvColorImage tempCVImage;
		tempCVImage.allocate(camWidth, camHeight);
		
		tempCVImage.setFromPixels(vidGrabber.getPixels(), camWidth,camHeight);
		
		videoGrayscaleCvImagePreWarp = tempCVImage; //colour -> grayscale conversion is automatic in ofxOpenCV
#endif		
		
		videoGrayscaleCvImage.warpIntoMe(videoGrayscaleCvImagePreWarp, srcPts, dstPts);
		
		if (ofGetElapsedTimef() < 1.5f){
			videoBgImage = videoGrayscaleCvImage;
		}
		
		if (bLearnBg){ 
			videoBgImage = videoGrayscaleCvImage;
			gui.setValueB("WARP_B_LEARN_BG", false);
		}
		
		if (bLearnBgProgressive){
			if (bLearnBg){
				videoFloatBgImage = videoBgImage;
			}
			
			videoFloatBgImage.addWeighted(videoGrayscaleCvImage, rateOfProgression);
			videoBgImage = videoFloatBgImage;		
		}
		
		if( warpDiffMode == 0 ){ //abs
			videoDiffImage.absDiff(videoGrayscaleCvImage, videoBgImage);
		}else if( warpDiffMode == 1 ){ //greater
			videoDiffImage = videoGrayscaleCvImage;
			videoDiffImage -= videoBgImage;
		}else if( warpDiffMode == 2 ){ //less
			videoDiffImage = videoBgImage;
			videoDiffImage -= videoGrayscaleCvImage;	
		}	
		
		
		videoDiffImage.threshold(threshold);
		
		videoContourFinder.findContours(videoDiffImage, minBlobSize, maxBlobSize, nBlobsConsidered, bFindHoles, true);
		
		blobTracker.trackBlobs(videoContourFinder.blobs);
		
			//take the strip of the centre of the image, not any more, from the centre of the biggest blob
		
		if(videoContourFinder.blobs.size() > 0){
				//we have some blobs, 0 is always the biggests
			int blobCentroidX = videoContourFinder.blobs[0].centroid.x;
			
			unsigned char* videoImagePointer = videoGrayscaleCvImage.getPixels();
			unsigned char* stripImagePointer = stripToStretch.getPixels();
			
			for(int i=0; i<camHeight; i++)
			{
				stripImagePointer[i] = videoImagePointer[blobCentroidX + (i*camWidth)];
			}
			
			stripToStretch.setFromPixels(stripImagePointer, 1, camHeight);
			
			stripToStretch.contrastStretch();
			
			if(gui.getValueB("AIM_SLIT_INVERT")){
				stripToStretch.invert();
			}
		}		
		
			//optical flow bits
		
		
		ofxCvGrayscaleImage forTheFlow;
		
		forTheFlow.allocate(camWidth, camHeight);
		
		forTheFlow = videoGrayscaleCvImage;
		
		forTheFlow.resize(camWidth/2, camHeight/2);
		
			// optical flow image
		flowImage.calc(forTheFlow, prevImage, 11);
		
			// pixels
		
			//not doing the field proper yet
//		ofxVec2f		vel;
//		float			len;
//		int				index;
//		
//		
//		for(int x=0; x<camWidth; x+=flowImage.captureRowsStep) {
//			for(int y=0; y<camHeight; y+=flowImage.captureColsStep) {
//				vel = flowImage.getVelAtNorm((float)x/(float)camWidth, (float)y/(float)camHeight);
//				vel.limit(gui.getValueF("FIELD_MAX_SPEED"));
//				len = vel.length();
//				vel *= -len;
//				
//				if(len > gui.getValueF("FIELD_RELEASE_MAG")) {
//					
//					field.addToField(x, y, vel.x, vel.y);
//				}
//			}
//		}
//		
//		field.fadeField(gui.getValueF("FIELD_FADE_RATE"));
//		field.limit(gui.getValueF("FIELD_LIMIT"));
		
		prevImage = forTheFlow;
		
			//countour simplification/manipulation
		
		int numberOfBlobs = videoContourFinder.blobs.size();
		
		if(numberOfBlobs > 0){
				//if we have at least one blob
			
			curve.resize(numberOfBlobs);
			curveSmooth.resize(numberOfBlobs);
			curveSimplify.resize(numberOfBlobs);
			curveCvSimplify.resize(numberOfBlobs);
			
			float mx = gui.getValueF("AURA_SIMPLIFICATION");
			
			float scale1	= mx;
			float scale2	= mx * 140;
			float scale3	= mx * 0.1;
			
			bool noneSmooth = gui.getValueB("AURA_IS_SMOOTH");
			bool simplifyCV = gui.getValueB("AURA_IS_CV");
			
			float auraScale = gui.getValueF("AURA_SCALE");
			
			bool scaleFromStage = gui.getValueB("AURA_SCALE_FROM_STAGE");
			
			for(int i = 0; i< numberOfBlobs; i++){
				
				curve[i] = videoContourFinder.blobs[i];
				
				ofPoint centreOfStage = ofPoint(camWidth/2.f, camHeight);
				
				if(scaleFromStage){
					curve[i].scaleBlob(centreOfStage, auraScale); //scale from the base of stage
				}else {
					curve[i].scaleBlob(curve[i].centroid, auraScale); //else do it from the centroid
				}
				
				if(noneSmooth){ //smooth it
					cu.smooth( curve[i].pts, curveSmooth[i].pts, scale1 );
				}else{
						//do nothing.
				}
				
				if(simplifyCV){
						//cv simplify it
					simplifyDP_openCV( curve[i].pts, curveCvSimplify[i].pts, scale3 );
				}else{
						//just simplify it
					cu.simplify( curve[i].pts, curveSimplify[i].pts, scale2 );
				}					
			}
				//TODO, fit into one curve? so can draw it either way, never mind the mode
		}
	}	
	
		//reseting the passage walls via the gui
	if(gui.getValueB("PASSAGE_RESET")){
		resetPassageWalls(); //do it here
		gui.setValueB("PASSAGE_RESET", FALSE);		
	}	
	
	gui.update();
}

//--------------------------------------------------------------
void testApp::draw(){
    ofSetColor(255, 255, 255, 255);
	
	float scaleX = 0.0f;
	float scaleY = 0.0f;
	
	float drawWidth = settings.PROJECTION_W;
	float drawHeight = settings.PROJECTION_H;
	
	if( camWidth != 0 ) { scaleX = drawWidth/camWidth; } else { scaleX = 1.0f; }
	if( camHeight != 0 ) { scaleY = drawHeight/camHeight; } else { scaleY = 1.0f; }	
	
    rm.startOffscreenDraw();
	
	if(bShowWhiteRect){
		allWhiteImage.draw(0,0,settings.PROJECTION_W,settings.PROJECTION_H);
	}else {
		allBlackImage.draw(0,0,settings.PROJECTION_W,settings.PROJECTION_H); //hacktastic
		
		float auraAlphaNoneSmooth		= (float)gui.getValueI("AURA_FADE_NONE_SMOOTH")/127.f; //scale to 0..1
		float auraAlphaSimplifyCV		= (float)gui.getValueI("AURA_FADE_SIMPLIFY_CV")/127.f; //scale to 0..1
		float aimAlpha		= (float)gui.getValueI("AIM_FADE")/127.f;
		float passageAlpha	= (float)gui.getValueI("PASSAGE_FADE")/127.f;
		float fieldAlpha	= (float)gui.getValueI("FIELD_FADE")/127.f;

		drawAura(auraAlphaNoneSmooth, auraAlphaSimplifyCV);
		drawAim(aimAlpha);
		drawPassage(passageAlpha);
		drawField(fieldAlpha);
	}
	
	
    rm.endOffscreenDraw();
	
    ofSetColor(255, 255, 255, 255);
	
    rm.drawInputDiagnostically(guiIn.x, guiIn.y, guiIn.width, guiIn.height);
    rm.drawOutputDiagnostically(guiOut.x, guiOut.y, guiOut.width, guiOut.height);
	
    glPushMatrix();
	glTranslatef(10, guiIn.y+guiIn.height + 30, 0);
	glScalef(0.5f,0.5f,1.f);
	ofSetColor(255, 255, 255, 255);
	rm.drawScreen(0);
    glPopMatrix();	
	
    glPushMatrix();
	glTranslatef(settings.GUI_W, 0, 0);
	ofSetColor(255, 255, 255, 255);
	rm.drawScreen(0);
    glPopMatrix();	
	
	
    ofDrawBitmapString("internal texture points", 10, 228);
    ofDrawBitmapString("texture warping points", 535, 228);
	
    ofDrawBitmapString("s - to save to xml   l - to reload from xml    r - reset coordinates\n", 10, 16);
	
    /* Framerate display */
	ofDrawBitmapString(buf,30,appHeight - 40);
	ofDrawBitmapString(buf2,30,appHeight - 20);
	
	ofSetColor(0xffffff);
	
	gui.draw();	
	
	ofDrawBitmapString("Time since Spacebar: "+ofToString(ofGetElapsedTimef()-timeOfSpace, 2), 0, ofGetHeight()-20.f);	
}

void testApp::drawAura(float noneSmoothAlpha, float simplifyCVAlpha){
	ofEnableAlphaBlending();


	float lineWidth = gui.getValueF("AURA_LINE_WIDTH");
	
		//			blobTracker.draw(auraOffsetX,auraOffsetY,scaleX*auraScale, scaleY*auraScale, lineWidth);
	
	int numberOfBlobs = videoContourFinder.blobs.size();
	
	bool noneSmooth = gui.getValueB("AURA_IS_SMOOTH");
	bool simplifyCV = gui.getValueB("AURA_IS_CV");	
	
	for(int i = 0; i< numberOfBlobs; i++){	
		
		ofSetColor(255, 255, 255, (int)(noneSmoothAlpha*255.f));
		
		if(noneSmooth){ //smooth it
						//smooth
			drawCurve( curveSmooth[i].pts ,0,0,settings.PROJECTION_W,settings.PROJECTION_H, lineWidth, noneSmoothAlpha);
		}else{
				//none
				//so don't do anything!
			drawCurve( curve[i].pts ,0,0,settings.PROJECTION_W,settings.PROJECTION_H, lineWidth, noneSmoothAlpha);			
		}
		
		ofSetColor(255, 255, 255, (int)(simplifyCVAlpha*255.f));
		
		if(simplifyCV){
				//cv simplify
			drawCurve( curveCvSimplify[i].pts ,0,0,settings.PROJECTION_W,settings.PROJECTION_H, lineWidth, simplifyCVAlpha);
		}else{
				//simplify
			drawCurve( curveSimplify[i].pts ,0,0,settings.PROJECTION_W,settings.PROJECTION_H, lineWidth, simplifyCVAlpha);
		}
	}	
}

void testApp::drawAim(float overallAlpha){
	ofPushStyle();
	ofEnableAlphaBlending();
	ofSetColor(255, 255, 255, (int)(overallAlpha*255.f));
	ofSetRectMode(OF_RECTMODE_CENTER);

	if(videoContourFinder.blobs.size() > 0){
			//we have some blobs, 0 is always the biggests
		int blobCentroidX = videoContourFinder.blobs[0].centroid.x;
		
		float blobCentroidXScaled = ((float)blobCentroidX/(float)camWidth)*settings.PROJECTION_W;
		float blobY = settings.PROJECTION_H;
		float stripWidth = gui.getValueF("AIM_SLIT_WIDTH") * settings.PROJECTION_W * 2.f;
		float stripHeight = gui.getValueF("AIM_SLIT_HEIGHT") * settings.PROJECTION_H * 2.f;		
		
		bool redness = gui.getValueB("AIM_REDNESS");
		
		if(redness){
			ofSetColor(255, 0, 0, (int)(overallAlpha*255.f));
		}
	
		stripToStretch.draw(blobCentroidXScaled, blobY, stripWidth, stripHeight);
	}
	
	ofSetRectMode(OF_RECTMODE_CORNER);
	ofPopStyle();
}

void testApp::drawPassage(float overallAlpha){
	float scaleX = 0.0f;
	float scaleY = 0.0f;
	
	float drawWidth = settings.PROJECTION_W;
	float drawHeight = settings.PROJECTION_H;
	
	if( camWidth != 0 ) { scaleX = drawWidth/camWidth; } else { scaleX = 1.0f; }
	if( camHeight != 0 ) { scaleY = drawHeight/camHeight; } else { scaleY = 1.0f; }		
	
	ofPushStyle();
	glPushMatrix();
	ofFill();
	glTranslatef(0, 0, 0);
	glScalef( scaleX, scaleY, 0.0 );
	
	ofEnableAlphaBlending();
	ofSetColor(255, 255, 255, (int)(overallAlpha*255.f));	
	
	bool bProjectHorizontal = gui.getValueB("PASSAGE_HORIZONTAL");
	bool bProjectVertical = gui.getValueB("PASSAGE_VERTICAL");
	bool bProjectInverse = gui.getValueB("PASSAGE_INVERT");
	bool bBlockWalls = gui.getValueB("PASSAGE_BLOCK_WALLS");
	
	float gravity = gui.getValueF("PASSAGE_GRAVITY");
	float coefficientOfRestitution = gui.getValueF("PASSAGE_COEFFICIENT_OF_RESTITUTION");
	float coefficientOfVelocity = gui.getValueF("PASSAGE_COEFFICIENT_OF_VELOCITY");
	
	if(videoContourFinder.blobs.size() > 0){
			//we have some blobs, 0 is always the biggests
		float blobLeft = videoContourFinder.blobs[0].boundingRect.x;
		float blobRight = blobLeft + videoContourFinder.blobs[0].boundingRect.width;
		float blobTop = videoContourFinder.blobs[0].boundingRect.y;
		
		if(!gui.getValueB("PASSAGE_SNAP")){
				//if we are not snapping, then update the positions of everywhere, according to a very basic system
			
				//first the left
			
			float blobLeftPosition = blobLeft;
			
			leftPassage += velocityLeftPassage; //should be doing this with floats and stuff, proper timing, not frame by frame
			
			velocityLeftPassage += gravity;
			
			if (leftPassage > blobLeftPosition) {
				float blobLeftVelocity = blobLeftPosition - blobLeftPreviousPosition;
				blobLeftVelocity *= coefficientOfVelocity;
				
				velocityLeftPassage = -velocityLeftPassage;
				velocityLeftPassage *= coefficientOfRestitution; //co-eff of restitute!		
				
				velocityLeftPassage += blobLeftVelocity;
				
				leftPassage = blobLeftPosition;
			}
			
			if(bBlockWalls && (leftPassage < 0.f)){
				velocityLeftPassage = -velocityLeftPassage;
				velocityLeftPassage *= coefficientOfRestitution; //co-eff of restitute!		
				
				leftPassage = 0;				
			}
			
			blobLeftPreviousPosition = blobLeftPosition;
			
				//now the right
			
			float blobRightPosition = blobRight;
			
			rightPassage -= velocityRightPassage; //should be doing this with floats and stuff, proper timing, not frame by frame
			
			velocityRightPassage += gravity;
			
			if (rightPassage < blobRightPosition) {
				float blobRightVelocity = blobRightPosition - blobRightPreviousPosition;	
				blobRightVelocity *= coefficientOfVelocity;
				
				velocityRightPassage = -velocityRightPassage;
				velocityRightPassage *= coefficientOfRestitution; //co-eff of restitute!
				
				velocityRightPassage += blobRightVelocity;
				
				rightPassage = blobRightPosition;
			}	
			
			if(bBlockWalls && (rightPassage > camWidth)){
				velocityRightPassage = -velocityRightPassage;
				velocityRightPassage *= coefficientOfRestitution; //co-eff of restitute!		
				
				rightPassage = camWidth;				
			}			
			
			
			blobRightPreviousPosition = blobRightPosition;
			
				//now top
			
			float blobTopPosition = blobTop;
			
			topPassage += velocityTopPassage; //should be doing this with floats and stuff, proper timing, not frame by frame
			
			velocityTopPassage += gravity;
			
			if (topPassage > blobTopPosition) {
				float blobTopVelocity = blobTopPosition - blobTopPreviousPosition;
				blobTopVelocity *= coefficientOfVelocity;
				
				velocityTopPassage = -velocityTopPassage;
				velocityTopPassage *= coefficientOfRestitution; //co-eff of restitute!
				
				velocityTopPassage += blobTopVelocity;
				
				topPassage = blobTopPosition;
			}
			
			if(bBlockWalls && (topPassage < 0)){
				velocityTopPassage = -velocityTopPassage;
				velocityTopPassage *= coefficientOfRestitution; //co-eff of restitute!		
				
				topPassage = 0.f;				
			}				
			
			blobTopPreviousPosition = blobTopPosition;	
			
				//now do the setting
			
			blobLeft = leftPassage;
			blobRight = rightPassage;
			blobTop  = topPassage;
		}
		
		if(bProjectInverse){
			ofRect(0, 0, camWidth, camHeight);
			ofSetColor(0, 0, 0, (int)(overallAlpha*255.f));
		}
		
		if(bProjectHorizontal){
			ofRect(0,0,camWidth, blobTop);
		}
		
		if(bProjectVertical){
				//first from 0 to blobLeft
			ofRect(0,0,blobLeft,camHeight);
				//second from blobRight to camWidth
			ofRect(blobRight,0,camWidth-blobRight, camHeight);
		}
	}
	
	glPopMatrix();
	ofPopStyle();	
}

void testApp::drawField(float overallAlpha){
	ofEnableAlphaBlending();
	ofSetColor(255, 255, 255, (int)(overallAlpha*255.f));	
		
	flowImage.draw(0,0,settings.PROJECTION_W, settings.PROJECTION_H);	
	
//	float scaleX = 0.0f;
//	float scaleY = 0.0f;
//	
//	float drawWidth = settings.PROJECTION_W;
//	float drawHeight = settings.PROJECTION_H;
//	
//	if( camWidth != 0 ) { scaleX = drawWidth/camWidth; } else { scaleX = 1.0f; }
//	if( camHeight != 0 ) { scaleY = drawHeight/camHeight; } else { scaleY = 1.0f; }		
//	
//	ofPushStyle();
//	glPushMatrix();
//	ofFill();
//	glTranslatef(0, 0, 0);
//	glScalef( scaleX, scaleY, 0.0 );
//	
//	ofEnableAlphaBlending();
//	ofSetColor(255, 255, 255, (int)(overallAlpha*255.f));	
//	
//	field.draw();
//	
//	glPopMatrix();
//	ofPopStyle();		
}

	//--------------------------------------------------------------
void testApp::calculateCaptureFramerate()
{
    mytimeNow = ofGetElapsedTimef();
    if( (mytimeNow-mytimeThen) > 0.05f || myframes == 0 ) {
        myfps = myframes / (mytimeNow-mytimeThen);
        mytimeThen = mytimeNow;
        myframes = 0;
        myframeRate = 0.9f * myframeRate + 0.1f * myfps;
        sprintf(buf2,"Capture framerate : %f",myframeRate);
    }
    myframes++;
}

	//--------------------------------------------------------------
void testApp::eventsIn(guiCallbackData & data){
	
//		//lets send all events to our logger
//	if( data.groupName != "events logger"){
//		string logStr = data.groupName;
//		
//		if( data.fVal.size() ){
//			for(int i = 0; i < data.fVal.size(); i++) logStr += " - "+ofToString(data.fVal[i], 4);
//		}
//		if( data.iVal.size() ){
//			for(int i = 0; i < data.iVal.size(); i++) logStr += " - "+ofToString(data.iVal[i]);
//		}	
//		if( data.sVal.size() ){
//			for(int i = 0; i < data.sVal.size(); i++) logStr += " - "+data.sVal[i];
//		}
//		
//		logger.log(OF_LOG_NOTICE, "event - %s", logStr.c_str());
//	}
	
		// print to terminal if you want to 
		//this code prints out the name of the events coming in and all the variables passed
//	printf("testApp::eventsIn - name is %s - \n", data.groupName.c_str());
//	if( data.elementName != "" ){
//		printf(" element name is %s \n", data.elementName.c_str());
//	}
//	if( data.fVal.size() ){
//		for(int i = 0; i < data.fVal.size(); i++){
//			printf(" float value [%i] = %f \n", i, data.fVal[i]);
//		}
//	}
//	if( data.iVal.size() ){
//		for(int i = 0; i < data.iVal.size(); i++){
//			printf(" int value [%i] = %i \n", i, data.iVal[i]);
//		}
//	}	
//	if( data.sVal.size() ){
//		for(int i = 0; i < data.sVal.size(); i++){
//			printf(" string value [%i] = %s \n", i, data.sVal[i].c_str());
//		}
//	}
//	printf("\n");
}

#ifdef _USE_FIREWIRE_
//--------------------------------------------------------------
void testApp::cameraEventsIn(guiCallbackData & data){
	
	cout << "Camera data, group name:" << data.groupName << ", element name: " << data.elementName << endl; 

	string str_modePattern = "_MODE";
	size_t found;
	
	found = data.groupName.find(str_modePattern);
	
	if(found!=string::npos) //if "_MODE" does exist in the string
	{
		
		cout << "_MODE Detected, not dealt with yet." << endl;
		//sdk->setFeatureMode(param_mode, param_id);
	}
	else {
		if(data.groupName == "WHITE_BALANCE") {
			sdk->setFeatureValue(data.fVal[0], data.fVal[1], Libdc1394GrabberFeatureHelper::libdcStringToAVidFeature(data.groupName));
		}
		else {
			sdk->setFeatureValue(data.fVal[0], Libdc1394GrabberFeatureHelper::libdcStringToAVidFeature(data.groupName));
		}
	}	
	
	
		//lets send all events to our logger
//	if( data.groupName != "events logger"){
//		string logStr = data.groupName;
//		
//		if( data.fVal.size() ){
//			for(int i = 0; i < data.fVal.size(); i++) logStr += " - "+ofToString(data.fVal[i], 4);
//		}
//		if( data.iVal.size() ){
//			for(int i = 0; i < data.iVal.size(); i++) logStr += " - "+ofToString(data.iVal[i]);
//		}	
//		if( data.sVal.size() ){
//			for(int i = 0; i < data.sVal.size(); i++) logStr += " - "+data.sVal[i];
//		}
//		
//		logger.log(OF_LOG_NOTICE, "cameraEvent - %s", logStr.c_str());
//	}
	
		// print to terminal if you want to 
		//this code prints out the name of the events coming in and all the variables passed
//	printf("testApp::cameraEventsIn - name is %s - \n", data.groupName.c_str());
//	if( data.elementName != "" ){
//		printf(" element name is %s \n", data.elementName.c_str());
//	}
//	if( data.fVal.size() ){
//		for(int i = 0; i < data.fVal.size(); i++){
//			printf(" float value [%i] = %f \n", i, data.fVal[i]);
//		}
//	}
//	if( data.iVal.size() ){
//		for(int i = 0; i < data.iVal.size(); i++){
//			printf(" int value [%i] = %i \n", i, data.iVal[i]);
//		}
//	}	
//	if( data.sVal.size() ){
//		for(int i = 0; i < data.sVal.size(); i++){
//			printf(" string value [%i] = %s \n", i, data.sVal[i].c_str());
//		}
//	}
//	printf("\n");
}

#endif

//--------------------------------------------------------------
void testApp::keyPressed  (int key){
	switch (key){
		case 'f':{
			ofToggleFullscreen();
			break;				
		}
		case 'h':{
			gui.toggleView();	
			break;
		}
				//		case 's':{
				//			grabber.videoSettings();				//from ofxcontrolpanel, need to port these parts..
				//													//if you need to adjust the camera properties
				//			break;
				//		}	
		case 'm':{
			if(bShowMouse){
				ofHideCursor();
				bShowMouse = false;
			}else {
				ofShowCursor();
				bShowMouse = true;
			} 
			break;				
		} 		
		case 'w':{
			bShowWhiteRect = !bShowWhiteRect;	
			break;
		}
		case 's':{
			rm.saveToXml();	
			break;
		}
		case 'l':{
			rm.reloadFromXml();
			break;
		}
		case 'r':{
			rm.resetCoordinates();
			break;
		}
		case ' ':{
			timeOfSpace = ofGetElapsedTimef();
			break;
		}	
		default:{
			break;
		}
	}
}


//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

	//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	if(!gui.hidden){
		gui.mouseDragged(x, y, button);		
	}else{
		rm.mouseDragInputPoint(guiIn, ofPoint(x, y));
		rm.mouseDragOutputPoint(guiOut, ofPoint( x, y));
	}
}

	//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	if(!gui.hidden){
		gui.mousePressed(x, y, button);		
	}else{
		if( !rm.mouseSelectInputPoint(guiIn, ofPoint(x, y)) ){
			rm.mouseSelectOutputPoint(guiOut, ofPoint( x,  y));
		}
	}	
}

	//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	if(!gui.hidden){
		gui.mouseReleased();		
	}	
}

	//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
	
}

//void testApp::parameterCallback(float param1, float param2, int param_mode, int param_id)
//{
//
//    if(param_mode != NULL_MODE)
//    {
//        //cout << "set mode called: "  << param_mode << " param_id = " << param_id << endl;
//        vidGrabber.setFeatureMode(param_mode, param_id);
//    }
//    else {
//        if(param_id == FEATURE_WHITE_BALANCE) {
//            vidGrabber.setFeatureValue(param1, param2, param_id);
//        }
//        else {
//            //cout << "set feature called: " << param1 << " " << param_id << endl;
//           vidGrabber.setFeatureValue(param1, param_id);
//        }
//    }
//
//}
	   
void testApp::randomiseGuiValueSafely(string whichGuiValue){
	float maxValue = gui.getMaxOfValueF(whichGuiValue);
	float minValue = gui.getMinOfValueF(whichGuiValue);
	
	float newValue = ofRandom(minValue, maxValue);
	
	gui.setValueF(whichGuiValue, newValue);
}


void testApp::drawContourFinder(float xPos, float yPos, float drawWidth, float drawHeight){
		
	float scalex = 0.0f;
	float scaley = 0.0f;
		
	float contourWidth = videoContourFinder.getWidth();
	float contourHeight = videoContourFinder.getHeight();
		
	if( contourWidth != 0 ) { scalex = drawWidth/contourWidth; } else { scalex = 1.0f; }
	if( contourHeight != 0 ) { scaley = drawHeight/contourHeight; } else { scaley = 1.0f; }
	
		//	if(videoContourFinder.bAnchorIsPct){ //no anchoiring crap
//		xPos -= anchor.x * drawWidth;
//		yPos -= anchor.y * drawHeight;
//	}else{
//		xPos -= anchor.x;
//		yPos -= anchor.y;
		//	}
	
		// ---------------------------- draw the bounding rectangle
		//ofSetColor(0xDD00CC);
		//    ofSetColor(255,255,255, 192);
	glPushMatrix();
    glTranslatef( xPos, yPos, 0.0 );
    glScalef( scalex, scaley, 0.0 );
		//
		//	ofNoFill();
		//	for( int i=0; i<(int)blobs.size(); i++ ) {
		//		ofRect( blobs[i].boundingRect.x, blobs[i].boundingRect.y,
		//                blobs[i].boundingRect.width, blobs[i].boundingRect.height );
		//	}
	
		// ---------------------------- draw the blobs
		//ofSetColor(0x00FFFF);
	
		//ofEnableAlphaBlending();
	
		//ofSetColor(255,255,255, 192);
	ofSetColor(255,255,255);
		//ofNoFill();
	
		//ofFill();  not filling for a while with nina
	
	for( int i=0; i<(int)videoContourFinder.blobs.size(); i++ ) {

		ofBeginShape();
		for( int j=0; j<videoContourFinder.blobs[i].nPts; j++ ) {
			ofVertex( videoContourFinder.blobs[i].pts[j].x, videoContourFinder.blobs[i].pts[j].y );
		}
		ofEndShape();
		
	}
	glPopMatrix();
	
		//ofNoFill(); not filling for a while with nina
	//ofDisableAlphaBlending(); no alpha either!
}

/*
 *
 *	blob section, from ollie palmer
 *	
 *	from here on in it's blobs
 *	thanks to stefanix and the opencv library :)
 *	
 */

	//--------------------------------------------------
void testApp::blobOn( int x, int y, int id, int order ) {
		//cout << "blobOn() - id:" << id << " order:" << order << endl;
	
}

void testApp::blobMoved( int x, int y, int id, int order) {
		//cout << "blobMoved() - id:" << id << " order:" << order << endl;
    
		// full access to blob object ( get a reference)
		//ofCvTrackedBlob blob = blobTracker.getById( id );
		//cout << "area: " << blob.area << endl;
}

void testApp::blobOff( int x, int y, int id, int order ) {
		//cout << "blobOff() - id:" << id << " order:" << order << endl;
}


	//curve simplifications from Julapy
	//-- douglas pecker approximation using openCV

void testApp :: simplifyDP_openCV ( const vector<ofPoint>& contourIn, vector<ofPoint>& contourOut, float tolerance )		
{
		//-- copy points.
	
	int numOfPoints;
	numOfPoints = contourIn.size();
		//	numOfPoints += ( contourIn.size() > 0 ) ? 1 : 0;
	
	CvPoint* cvpoints;
	cvpoints = new CvPoint[ numOfPoints ];
	
	for( int i=0; i<numOfPoints; i++)
	{
		int j = i % numOfPoints;
		
		cvpoints[ i ].x = contourIn[ j ].x;
		cvpoints[ i ].y = contourIn[ j ].y;
	}
	
		//-- create contour.
	
	CvContour	contour;
	CvSeqBlock	contour_block;
	
	cvMakeSeqHeaderForArray
	(
	 CV_SEQ_POLYLINE,			// CV_SEQ_POLYLINE, CV_SEQ_POLYGON, CV_SEQ_CONTOUR, CV_SEQ_SIMPLE_POLYGON
	 sizeof(CvContour),
	 sizeof(CvPoint),
	 cvpoints,
	 numOfPoints,
	 (CvSeq*)&contour,
	 &contour_block
	 );
	
	printf( "length = %f \n", cvArcLength( &contour ) );
	
		//-- simplify contour.
	
	CvMemStorage* storage;
	storage = cvCreateMemStorage( 1000 );
	
	CvSeq *result = 0;
	result = cvApproxPoly
	(
	 &contour,
	 sizeof( CvContour ),
	 storage,
	 CV_POLY_APPROX_DP,
	 //		cvContourPerimeter( &contour ) * 0.004,
	 cvContourPerimeter( &contour ) * tolerance,
	 0
	 );
	
		//-- contour out points.
	
	contourOut.clear();
	for( int j=0; j<result->total; j++ )
	{
		CvPoint * pt = (CvPoint*)cvGetSeqElem( result, j );
		
		contourOut.push_back( ofPoint() );
		contourOut.back().x = (float)pt->x;
		contourOut.back().y = (float)pt->y;
	}
	
		//-- clean up.
	
	if( storage != NULL )
		cvReleaseMemStorage( &storage );
	
	delete[] cvpoints;
}

void testApp :: drawCurve ( const vector<ofPoint>& points, float xPos, float yPos, float drawWidth, float drawHeight, float drawLineWidth, float overallAlpha)
{
	ofPushStyle();
	ofSetLineWidth(drawLineWidth);
	
	float scalex = 0.0f;
	float scaley = 0.0f;
	
	float contourWidth = videoContourFinder.getWidth();
	float contourHeight = videoContourFinder.getHeight();
	
	if( contourWidth != 0 ) { scalex = drawWidth/contourWidth; } else { scalex = 1.0f; }
	if( contourHeight != 0 ) { scaley = drawHeight/contourHeight; } else { scaley = 1.0f; }
	
	glPushMatrix();
    glTranslatef( xPos, yPos, 0.0 );
    glScalef( scalex, scaley, 0.0 );	
	
//	for( int i=0; i<points.size(); i++ )
//	{
//		ofSetColor( 0, 255, 255 );
////		ofCircle( points[ i ].x, points[ i ].y, 2 );
//		
//		int j = i + 1;
//		
//		if( j < points.size() )
//		{
//			ofSetColor( 255, 255, 255 );
//			ofLine( points[ i ].x, points[ i ].y, points[ j ].x, points[ j ].y );
//		}
//		
//		
//	}

	if(gui.getValueB("AURA_FILL")){
		ofFill();
	}else{
		ofNoFill();		
	}
	
	
	
	ofSetColor(255, 255, 255, (int)(overallAlpha*255.f));
	
	ofBeginShape();
	for( int i=0; i<points.size(); i++ ){
		ofVertex( points[ i ].x, points[ i ].y ); //to allow the fill
	}
	ofEndShape();	
	
	glPopMatrix();
	
	ofPopStyle();
}

void testApp :: drawFittedCurve ( const vector<ofPoint>& points, float xPos, float yPos, float drawWidth, float drawHeight)
{
	ofPushStyle();
	
	float scalex = 0.0f;
	float scaley = 0.0f;
	
	float contourWidth = videoContourFinder.getWidth();
	float contourHeight = videoContourFinder.getHeight();
	
	if( contourWidth != 0 ) { scalex = drawWidth/contourWidth; } else { scalex = 1.0f; }
	if( contourHeight != 0 ) { scaley = drawHeight/contourHeight; } else { scaley = 1.0f; }
	
	glPushMatrix();
    glTranslatef( xPos, yPos, 0.0 );
    glScalef( scalex, scaley, 0.0 );	
	
	ofFill();
	ofSetColor( 0, 255, 255 );
	
	int t = points.size();
	
//	for( int i=0; i<t; i++ )
//	{
//		ofCircle( points[ i ].x, points[ i ].y, 2 );
//	}
	
	if( t < 2 )
		return;
	
	ofNoFill();
	ofSetColor( 255, 0, 255 );
	
	ofBeginShape();
	
	ofCurveVertex( points[ 0 ].x, points[ 0 ].y );
	
	for( int i=0; i<t; i++ )
	{
		ofCurveVertex( points[ i ].x, points[ i ].y );
	}
	
	ofCurveVertex( points[ t - 1 ].x, points[ t - 1 ].y );
	
	ofEndShape( false );
	
	glPopMatrix();
	
	ofPopStyle();
}

	// Method that receives MIDI event messages.
void testApp::newMidiMessage(ofxMidiEventArgs& eventArgs){
	cout << "byteOne = " << eventArgs.byteOne << endl;
	cout << "byteTwo = " << eventArgs.byteTwo << endl;
	
		// Do the stuff you need to do with 
		// the ofxMidiEventArgs instance.
	
	switch (eventArgs.byteOne){
		case 2:{
				//slider 1, with scene set to 1
			gui.setValueI("AURA_FADE", eventArgs.byteTwo);
			break;				
		}
		case 3:{
				//slider 2 amout of simplification
			float newSimplification = ofMap(eventArgs.byteTwo, 0, 127, 0.f, 1.f, true);
			
			gui.setValueF("AURA_SIMPLIFICATION", newSimplification);					
			break;			
		}				
		case 4:{
				//slider 3, with scene set to 1
			gui.setValueI("AIM_FADE", eventArgs.byteTwo);			
			break;
		}
		case 5:{
				//slider 4 slit height
			float newHeight = ofMap(eventArgs.byteTwo, 1, 127, 0.01f, 1.f, true);
			
			gui.setValueF("AIM_SLIT_HEIGHT", newHeight);		
			break;				
		} 			
		case 6:{
				//slider 5, with scene set to 1
			gui.setValueI("PASSAGE_FADE", eventArgs.byteTwo);			
			break;				
		} 		
		case 9:{
				//slider 7, with scene set to 1
			gui.setValueI("FIELD_FADE", eventArgs.byteTwo);			
			break;
		}
		case 12:{
				//slider 8, with scene set to 1
			gui.setValueI("AURA_FADE_NONE_SMOOTH", eventArgs.byteTwo);			
			break;
		}	
		case 13:{
				//slider 9, with scene set to 1
			gui.setValueI("AURA_FADE_SIMPLIFY_CV", eventArgs.byteTwo);			
			break;
		}			
		case 14:{
				//knob 1
			float newScale = ofMap(eventArgs.byteTwo, 1, 127, 1.f, 15.f, true);
			
			gui.setValueF("AURA_SCALE", newScale);
			
			break;				
		}				
//		case 15:{
//				//knob 2 simplification mode
//			int newContourMode = ofMap(eventArgs.byteTwo, 1, 127, 0, contourDiffNames.size() - 1, true);			
//			
//			gui.setValueI("AURA_CONTOUR_MODE", newContourMode);			
//			break;
//
//		}				
		case 17:{
				//knob 4 slit width
			float newWidth = ofMap(eventArgs.byteTwo, 1, 127, 0.01f, 1.f, true);
			
			gui.setValueF("AIM_SLIT_WIDTH", newWidth);					
			break;
		}			
		case 23:{
				//button 1 top, scale aura from stage or centre
			if(eventArgs.byteTwo == 127){
				if(gui.getValueB("AURA_SCALE_FROM_STAGE")){ //if its on, turn it off
					gui.setValueB("AURA_SCALE_FROM_STAGE", FALSE);
				}else{
					gui.setValueB("AURA_SCALE_FROM_STAGE", TRUE); //vice versa
				}
			}
			break;
		}
		case 25:{
				//button 3 top, invert slit
			if(eventArgs.byteTwo == 127){
				if(gui.getValueB("AIM_SLIT_INVERT")){ //if its on, turn it off
					gui.setValueB("AIM_SLIT_INVERT", FALSE);
				}else{
					gui.setValueB("AIM_SLIT_INVERT", TRUE); //vice versa
				}
			}
			break;
		}
		case 26:{
				//button 4 top, aim red
			if(eventArgs.byteTwo == 127){
				if(gui.getValueB("AIM_REDNESS")){ //if its on, turn it off
					gui.setValueB("AIM_REDNESS", FALSE);
				}else{
					gui.setValueB("AIM_REDNESS", TRUE); //vice versa
				}
			}
			break;
		}			
		case 27:{
				/*
				 gui.addToggle("Draw to head (Horiztonal Block)", "PASSAGE_HORIZONTAL", true);
				 gui.addToggle("Draw to sides (Vertical Blocks)", "PASSAGE_VERTICAL", false);
				 */
				//button 5 top, passage vert/horizontal
			if(eventArgs.byteTwo == 127){
				if(gui.getValueB("PASSAGE_HORIZONTAL")){ //if its on, turn it off
					gui.setValueB("PASSAGE_HORIZONTAL", FALSE);
					gui.setValueB("PASSAGE_VERTICAL", TRUE);
					resetPassageWalls();
				}else{
					gui.setValueB("PASSAGE_HORIZONTAL", TRUE);
					gui.setValueB("PASSAGE_VERTICAL", FALSE); //vice versa
					resetPassageWalls();
				}
			}
			break;
		}
		case 28:{
			/*
			 gui.addToggle("Invert", "PASSAGE_INVERT", false);
			 */			
				//button 6 top, passage invert
			if(eventArgs.byteTwo == 127){
				if(gui.getValueB("PASSAGE_INVERT")){ //if its on, turn it off
					gui.setValueB("PASSAGE_INVERT", FALSE);
				}else{
					gui.setValueB("PASSAGE_INVERT", TRUE); //vice versa
				}
			}
			break;
		}
		case 29:{
			/*	gui.addToggle("Blocking Walls?", "PASSAGE_BLOCK_WALLS", false);*/
			if(eventArgs.byteTwo == 127){
				if(gui.getValueB("PASSAGE_BLOCK_WALLS")){ //if its on, turn it off
					gui.setValueB("PASSAGE_BLOCK_WALLS", FALSE);
				}else{
					gui.setValueB("PASSAGE_BLOCK_WALLS", TRUE); //vice versa
				}
			}
			break;
		}
		case 30:{
			/*
			 gui.addToggle("SMOOTH? Else NONE", "AURA_IS_SMOOTH", false);
			 */			
				//button 8 top, none simplification tek
			if(eventArgs.byteTwo == 127){
				gui.setValueB("AURA_IS_SMOOTH", FALSE);
				
			}
			break;
		}	
		case 31:{
			/*
			 gui.addToggle("CV? Else SIMPLIFY", "AURA_IS_CV", false);
			 */			
				//button 9 top, simplify 
			if(eventArgs.byteTwo == 127){
				gui.setValueB("AURA_IS_CV", FALSE);
				
			}
			break;
		}				
		case 33:{
				//button 1 bottom, aura fill
			if(eventArgs.byteTwo == 127){
				if(gui.getValueB("AURA_FILL")){ //if its on, turn it off
					gui.setValueB("AURA_FILL", FALSE);
				}else{
					gui.setValueB("AURA_FILL", TRUE); //vice versa
				}
			}
			break;
		}			
		case 37:{
				//button 5 bottom, passage snap 
			if(eventArgs.byteTwo == 127){
				if(gui.getValueB("PASSAGE_SNAP")){ //if its on, turn it off
					gui.setValueB("PASSAGE_SNAP", FALSE);
					resetPassageWalls();
				}else{
					gui.setValueB("PASSAGE_SNAP", TRUE);
				}
			}
			break;
		}
		case 38:{
				//button 6 bottom, reset passage walls
			if(eventArgs.byteTwo == 127){
				gui.setValueB("PASSAGE_RESET", TRUE);
			}
			break;
		}	
		case 40:{
			/*
			 gui.addToggle("SMOOTH? Else NONE", "AURA_IS_SMOOTH", false);
			 */			
				//button 8 bottom, smooth simplification tek
			if(eventArgs.byteTwo == 127){
				gui.setValueB("AURA_IS_SMOOTH", TRUE);
				
			}
			break;
		}	
		case 41:{
			/*
			 gui.addToggle("CV? Else SIMPLIFY", "AURA_IS_CV", false);
			 */			
				//button 9 bottom, cv simplification
			if(eventArgs.byteTwo == 127){
				gui.setValueB("AURA_IS_CV", TRUE);
				
			}
			break;
		}
		default:{
			break;
		}
	}	
}

void testApp::resetPassageWalls(){
	blobLeftPreviousPosition = 0;	
	leftPassage = 0.f; //current vertical line position
	velocityLeftPassage = 0.f; //velocity of left passage
	
	blobRightPreviousPosition = 0;
	rightPassage = (float)camWidth; //current vertical line position
	velocityRightPassage = 0.f; //velocity of left passage
	
	blobTopPreviousPosition = 0;
	topPassage = 0; //horizontal line position
	velocityTopPassage = 0.f; //velocity of the top passage	
}

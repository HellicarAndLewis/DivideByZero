/*
 *  AppGlobals.h
 *  emptyExample
 *
 *  Created by Todd Vanderlin on 3/30/10.
 *  Copyright 2010 Interactive Design. All rights reserved.
 *
 */

#pragma once
#include "ofxXmlSettings.h"

class AppSettings {

protected:
	ofxXmlSettings xml;
public:
	
	int GUI_W;
	int GUI_H;
	int PROJECTION_W;
	int PROJECTION_H;	

	AppSettings() {
	}
	
	void loadSettings(string file) {
		if(xml.loadFile(file)) {
			GUI_W = xml.getValue("GUI_W", 1440);
			GUI_H = xml.getValue("GUI_H", 900);
			PROJECTION_W = xml.getValue("PROJECTION_W", 1024);
			PROJECTION_H = xml.getValue("PROJECTION_H", 768);
		}
		
	}
};


// so other apps can see it...
extern AppSettings settings;
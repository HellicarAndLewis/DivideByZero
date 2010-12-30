/*
 *  ofxContourUtil.h
 *  emptyExample
 *
 *  Created by Lukasz Karluk on 17/05/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef OFX_CONTOUR_UTIL
#define OFX_CONTOUR_UTIL

#include "ofMain.h"
#include "ofxVectorMath.h"			// TODO :: need to get rid of this dependency - its just simplify() that uses ofxVectorMath.
#include "polylineSimplify.h"
#include "convexHull.h"

class ofxContourUtil
{

public :
	
	//-- smooth.
	
	template <typename T>
	void smooth		( vector<T>& contourIn, float smoothPct );
	
	template <typename T>
	void smooth		( vector<T>& contourIn, vector<T>& contourOut, float smoothPct );
	
	//-- simplify.
	
	template <typename T>
	void simplify	( vector<T>& contourIn, float tolerance );
	
	template <typename T>
	void simplify	( vector<T>& contourIn, vector<T>& contourOut, float tolerance );
	
	//-- convex hull.
	
	template <typename T>
	void convexHull	( vector<T>& contourIn );
	
	template <typename T>
	void convexHull	( vector<T>& contourIn, vector<T>& contourOut );
};

//////////////////////////////////////////
//	SMOOTH.
//////////////////////////////////////////

template <typename T>
void ofxContourUtil :: smooth ( vector<T>& contourIn, float smoothPct )
{
	vector<T> contourOut;
	
	smooth( contourIn, contourOut, smoothPct );
	
	contourIn = contourOut;								// copies vector content over.
}

template <typename T>
void ofxContourUtil :: smooth ( vector<T>& contourIn, vector<T>& contourOut, float smoothPct )
{
	contourOut.clear();
	contourOut.assign( contourIn.size(), T() );
	
	smoothPct = 1 - MIN( MAX( smoothPct, 0 ), 1 );
	
	float invPct;
	invPct = 1.0 - smoothPct;
	
	for( int i=0; i<contourOut.size(); i++ )			// first copy the data 
	{
		contourOut[ i ] = contourIn[ i ];
	}
	
	for( int i=1; i<contourOut.size(); i++ )			// we start at 1 as we take a small pct of the prev value
	{
		contourOut[ i ].x = contourOut[ i ].x * smoothPct + contourOut[ i - 1 ].x * invPct;
		contourOut[ i ].y = contourOut[ i ].y * smoothPct + contourOut[ i - 1 ].y * invPct;
	}
}

//////////////////////////////////////////
//	SIMPLIFY.
//////////////////////////////////////////

template <typename T>
void ofxContourUtil :: simplify( vector<T>& contourIn, float tolerance )
{
	vector<T> contourOut;
	
	simplify( contourIn, contourOut, tolerance );
	
	contourIn.clear();
	contourIn.assign( contourOut.size(), T() );
	contourIn = contourOut;								// copies vector content over.
	
	contourOut.clear();
}

template <typename T>
void ofxContourUtil :: simplify( vector<T>& contourIn, vector<T>& contourOut, float tolerance )
{
	int length = contourIn.size();
	
	if( length == 0 )										// poly_simplify crashes if length is zero.
		return;
	
	ofxPoint3f *polyLineIn	= new ofxPoint3f[ length ];		// the polyLine simplify class needs data as a vector of ofxPoint3fs 
	ofxPoint3f *polyLineOut	= new ofxPoint3f[ length ];
	
	for( int i=0; i<length; i++ )							// first we copy over the data to a 3d point array
	{						
		polyLineIn[ i ].x	= contourIn[ i ].x;
		polyLineIn[ i ].y	= contourIn[ i ].y;
		polyLineIn[ i ].z	= 0.0f;
		polyLineOut[ i ].x	= 0.0f;
		polyLineOut[ i ].y	= 0.0f;
		polyLineOut[ i ].z	= 0.0f;
	}
	
	int numPoints = poly_simplify( tolerance, polyLineIn, length, polyLineOut );
	contourOut.clear();
	contourOut.assign( numPoints, T() );
	
	for(int i = 0; i < numPoints; i++)						// copy the data to our contourOut vector
	{
		contourOut[ i ].x = polyLineOut[ i ].x;
		contourOut[ i ].y = polyLineOut[ i ].y;
	}
	
	delete[] polyLineIn;
	delete[] polyLineOut;
}

//////////////////////////////////////////
//	CONVEX HULL.
//////////////////////////////////////////

template <typename T>
void ofxContourUtil :: convexHull ( vector<T>& contourIn )
{
	vector<T> contourOut;
	
	convexHull( contourIn, contourOut );
	
	contourIn.clear();
	contourIn.assign( contourOut.size(), T() );
	contourIn = contourOut;								// copies vector content over.
}

template <typename T>
void ofxContourUtil :: convexHull ( vector<T>& contourIn, vector<T>& contourOut )
{
	int numPtsIn = contourIn.size();
	
	vector<hPoint> hullPointsIn;
	hullPointsIn.clear();
	hullPointsIn.assign( numPtsIn, hPoint() );
	
	for( int i=0; i<numPtsIn; i++ )
	{
		hullPointsIn[ i ].x = contourIn[ i ].x;
		hullPointsIn[ i ].y = contourIn[ i ].y;
	}
	
	vector<hPoint> hullPointsOut = calcConvexHull( hullPointsIn );
	int numOut = hullPointsOut.size();
	
	contourOut.clear();
	contourOut.assign( numOut, T() );
	
	for( int i=0; i<numOut; i++ )
	{
		contourOut[ i ].x = hullPointsOut[ i ].x;
		contourOut[ i ].y = hullPointsOut[ i ].y;
	}
}

#endif
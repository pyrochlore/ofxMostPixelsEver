/**
 *  mpeEvents.h
 *  openFrameworks version of the popular synchronization system Most Pixels Ever by Dan Shiffman
 *  original repo: https://github.com/shiffman/Most-Pixels-Ever
 *  our fork: https://github.com/FlightPhase/Most-Pixels-Ever
 * 
 *  Created by James George on 5/17/11 @ Flightphase for the National Maritime Museum
 *
 *	More examples needed!
 */


#pragma once

#include "ofMain.h"
#include "ofEvents.h"

class ofxMPEEventArgs : public ofEventArgs
{
  public:
    int frame;
    string message;
	//TODO:
//	vector<char> bytes;
//	vector<int> ints;
//	vector<float> floats;
};

class ofxMPECoreEvents
{
  public:
    ofEvent<ofxMPEEventArgs> mpeFrame;	
  	ofEvent<ofxMPEEventArgs> mpeMessage;
	//TODO:
//	ofEvent<ofxMPEEventArgs> mpeInts;
//	ofEvent<ofxMPEEventArgs> mpeBytes;
//	ofEvent<ofxMPEEventArgs> mpeFloats;
};

extern ofxMPECoreEvents ofxMPEEvents;

template<class ListenerClass>
void ofxMPERegisterEvents(ListenerClass * listener){
    ofAddListener(ofxMPEEvents.mpeFrame, listener, &ListenerClass::mpeFrameEvent);
    ofAddListener(ofxMPEEvents.mpeMessage, listener, &ListenerClass::mpeMessageEvent);
	//TODO:	
//	ofAddListener(ofxMPEEvents.mpeBytes, listener, &ListenerClass::mpeByteEvent);
//	ofAddListener(ofxMPEEvents.mpeInts, listener, &ListenerClass::mpeIntEvent);
//	ofAddListener(ofxMPEEvents.mpeFloats, listener, &ListenerClass::mpeIntEvent);
}



/*
 *  mpeServerTCP.h
 *  openFrameworks version of the popular synchronization system Most Pixels Ever by Dan Shiffman
 *  original repo: https://github.com/shiffman/Most-Pixels-Ever
 *  our fork: https://github.com/FlightPhase/Most-Pixels-Ever
 *
 *  I affectionately refer to as "Most Pickles Ever" since it's gotten me out of the most pickles. ever!
 * 
 *  Created by James George on 5/17/11 @ Flightphase for the National Maritime Museum: Voyagers installation
 *
 *	More examples needed!
 */

#pragma once

#include "ofMain.h"
#include "ofxNetwork.h"

typedef struct
{
	bool started;
	bool ready;
} Connection;

class mpeServerTCP : public ofThread
{
  public:
	mpeServerTCP();
	~mpeServerTCP();

	void setup(string setupFile);
	void setup(int framerate, int port, int numClients);
	void update(ofEventArgs& args);
	void close();
	
  protected:
	ofxTCPServer server;
	
	void threadedFunction();

	float lastFrameTriggeredTime;
	bool allconnected;
	bool running;
	int framerate;
	int numExpectedClients;
	int currentFrame;
	bool shouldTriggerFrame;
	vector<Connection> connections;
	bool newMessage;
	string currentMessage;

};
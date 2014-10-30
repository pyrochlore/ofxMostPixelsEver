#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup() {
	// initialize app
    ofSetVerticalSync(true);
	ofSetFrameRate(60);
    ofEnableSmoothing();
    ofEnableAlphaBlending();
	ofSetBackgroundAuto(false);
    
	client.setup("settings.xml", true);
    ofSetWindowShape(client.getLWidth(), client.getLHeight());
	
    // set the random seed
	ofSeedRandom(1);
    
    // add a "randomly" placed ball
    Ball* ball = new Ball(ofRandom(0, client.getMWidth()), ofRandom(0, client.getMHeight()), client.getMWidth(), client.getMHeight());
    balls.push_back(ball);
    
	//start client
    client.start();
    ofxMPERegisterEvents(this);
}

//--------------------------------------------------------------
void testApp::update() {	
}

//--------------------------------------------------------------
void testApp::draw() {

}

//--------------------------------------------------------------
void testApp::mpeFrameEvent(ofxMPEEventArgs& event)
{
    // clear the screen     
    ofBackground(0);
    
    client.placeScreen();

    //move and draw all the balls
    for (int i = 0; i < balls.size(); i++) {
        balls[i]->calc();
        balls[i]->draw();
    }

    ofSetColor(255,255,255);
    ofDrawBitmapString( ofToString(event.frame), 10, 20);
    ofDrawBitmapString( "number of balls: "+ofToString( balls.size()), 10, 40);
    ofDrawBitmapString( ofToString(client.getLWidth())+","+
        ofToString(client.getLHeight())+","+ofToString(client.getMWidth())+","+
        ofToString(client.getMHeight()), 10, 60);



}

//--------------------------------------------------------------
void testApp::mpeMessageEvent(ofxMPEEventArgs& event){
    //received a message from the server

    // read any incoming messages
    if (event.message != "") {
        
        vector<string> xy = ofSplitString(event.message, ",");
        if (xy.size() == 2)
        {
            float x = ofToInt(xy[0]);
            float y = ofToInt(xy[1]);
            Ball* ball = new Ball(x, y, client.getMWidth(), client.getMHeight());
            balls.push_back(ball);
        }
        

    }

}

//--------------------------------------------------------------
void testApp::mpeResetEvent(ofxMPEEventArgs& event){
    //triggered if the server goes down, another client goes offline, or a reset was manually triggered in the server code
}

//--------------------------------------------------------------
void testApp::keyPressed(int key) {
}

//--------------------------------------------------------------
void testApp::keyReleased(int key) {
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y) {
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button) {
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button) {
    // never include a ":" when broadcasting your message
    x += client.getXoffset();
    y += client.getYoffset();
    client.broadcast(ofToString(x) + "," + ofToString(y));
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button) {
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h) {
}


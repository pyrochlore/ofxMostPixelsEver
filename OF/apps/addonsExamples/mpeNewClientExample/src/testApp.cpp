#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	lastFrameTime = ofGetElapsedTimef();
	client.setup("mpe_client_settings.xml", true); //false means you can use backthread
	client.start();
	
	ofSetVerticalSync(true);
	ofSetFrameRate(60);
	
	ofxMPERegisterEvents(this);
	
	ofSetBackgroundAuto(false);
}

//--------------------------------------------------------------
void testApp::update(){
}

//--------------------------------------------------------------
void testApp::draw(){
	//don't use this
}

void testApp::mpeFrameEvent(ofxMPEEventArgs& event)
{
	ofClear(0, 0, 0, 0);
	
	ofColor bg = ofColor(client.getFrameCount()*10 % 255);
	ofSetColor(bg);
	
	ofRect(0, 0, ofGetWidth(), ofGetHeight());
	
	/*
	float now = ofGetElapsedTimef();
	cout << "fps would be " << 1./(now - lastFrameTime) << endl;
	lastFrameTime = now;
	*/
}

void testApp::mpeMessageEvent(ofxMPEEventArgs& event)
{
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}
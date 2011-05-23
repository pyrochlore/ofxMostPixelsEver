/**
 *  mpeEvents.h
 *  openFrameworks version of the popular synchronization system Most Pixels Ever by Dan Shiffman
 *  original repo: https://github.com/shiffman/Most-Pixels-Ever
 *  our fork: https://github.com/FlightPhase/Most-Pixels-Ever
 *
 *  I affectionately refer to as "Most Pickles Ever" since it's gotten me out of the most pickles. ever!
 *
 *  Created by James George on 5/17/11 @ Flightphase for the National Maritime Museum
 *
 *	More examples needed!
 */

#include "mpeClientTCP.h"
#include "mpeEvents.h"

//--------------------------------------------------------------
mpeClientTCP::mpeClientTCP() {
    setDefaults();
}

//--------------------------------------------------------------
void mpeClientTCP::setup(string _fileString, bool updateOnMainThread) {

	useMainThread = updateOnMainThread;
	
    loadIniFile(_fileString);
    ofSetWindowShape(lWidth, lHeight);
	frameCount = 0;
}

//--------------------------------------------------------------
void mpeClientTCP::start() {
    
	tcpClient.setVerbose(DEBUG);

	if(useMainThread){
		ofAddListener(ofEvents.draw, this, &mpeClientTCP::draw);
	}
	
    if (!tcpClient.setup(hostName, serverPort)) {
        err("TCP failed to connect to port " + ofToString(serverPort));
		lastConnectionAttempt = ofGetElapsedTimef();
		ofAddListener(ofEvents.update, this, &mpeClientTCP::retryConnectionLoop);
    }
	else{
		
		startThread(true, false);  // blocking, verbose
		out("TCP connection bound on port " + ofToString(serverPort));
	}
    
    
}

void mpeClientTCP::retryConnectionLoop(ofEventArgs& e)
{
	float now = ofGetElapsedTimef();
	if(now - lastConnectionAttempt > 1.0){ //retry every second
		if(tcpClient.setup(hostName, serverPort)) {
			cout << "retry succeeded, removing listener!" << endl;
			ofRemoveListener(ofEvents.update, this, &mpeClientTCP::retryConnectionLoop);
			startThread(true, false);  // blocking, verbose
		}
		lastConnectionAttempt = now;
	}
}

void mpeClientTCP::draw(ofEventArgs& e)
{
    //no blocking
    if(lock()) {
		
		for(int i = 0; i < dataMessage.size(); i++){
			ofxMPEEventArgs e;
			e.message = dataMessage[i];
			e.frame = getFrameCount();
			cout << "sending message in update " << e.frame << " message " << e.message << endl;
			
			ofNotifyEvent(ofxMPEEvents.mpeMessage, e);
		}		
		
		dataMessage.clear();
		//TODO: ints, floats, bytes, 

		if(triggerFrame){
			cout << "Trigger Event :: ! with frame count " << frameCount << endl;
			
			triggerFrame = false;
			//rendering = true;// wait until done

			ofxMPEEventArgs e;
			e.message = "";
			e.frame = frameCount;
			ofNotifyEvent(ofxMPEEvents.mpeFrame, e);
			
			done();
		}
		
		unlock();
    }
}


//--------------------------------------------------------------
// Called automatically by PApplet.draw() when using auto mode.
//--------------------------------------------------------------
//void mpeClientTCP::draw() {
//	if (isThreadRunning()){
//		//JG added this lock around the 'rendering var' needs to be atomic
//		if(lock()){
//			 if(rendering) {
//				placeScreen();
//                if(useMessageMode){
//                    ofSendMessage("frame");
//                } else{
//                    parent->frameEvent();
//                }
//				done();
//			}
//			unlock();
//		}
//	}
//}

//--------------------------------------------------------------
// Loads the settings from the Client INI file.
//--------------------------------------------------------------
void mpeClientTCP::loadIniFile(string _fileString) {
	out("Loading settings from file " + _fileString);

	ofxXmlSettings xmlReader;
    if (!xmlReader.loadFile(_fileString)){
        err("ERROR loading XML file!");
		return;
	}
	
    // parse INI file
    hostName   = xmlReader.getValue("settings:server:ip", "127.0.0.1", 0);
    serverPort = xmlReader.getValue("settings:server:port", 7887, 0);
	id         = xmlReader.getValue("settings:client_id", -1, 0);


	cout << "***MPE:: HOST IS " << hostName << " Server Port is " << serverPort << endl;

	setLocalDimensions(xmlReader.getValue("settings:local_dimensions:width",  640, 0),
					   xmlReader.getValue("settings:local_dimensions:height", 480, 0));

    setOffsets(xmlReader.getValue("settings:local_location:x", 0, 0),
               xmlReader.getValue("settings:local_location:y", 0, 0));

	setMasterDimensions(xmlReader.getValue("settings:master_dimensions:width",  640, 0),
						xmlReader.getValue("settings:master_dimensions:height", 480, 0));

	if (xmlReader.getValue("settings:go_fullscreen", "false", 0).compare("true") == 0)
		ofSetFullscreen(true);

	if(xmlReader.getValue("settings:offset_window", "false", 0).compare("true") == 0)
		ofSetWindowPosition(xOffset, yOffset);

	if (xmlReader.getValue("settings:debug", 0, 0) == 1)
        DEBUG = true;

    out("Settings: server = " + hostName + ":" + ofToString(serverPort) + ",  id = " + ofToString(id)
        + ", local dimensions = " + ofToString(lWidth) + ", " + ofToString(lHeight)
        + ", location = " + ofToString(xOffset) + ", " + ofToString(yOffset));
}

bool mpeClientTCP::isConnected()
{
    return tcpClient.isConnected();
}

//--------------------------------------------------------------
// Sets the dimensions for the local display.
//--------------------------------------------------------------
void mpeClientTCP::setLocalDimensions(int _lWidth, int _lHeight) {
    if (_lWidth > -1 && _lHeight > -1) {
        lWidth = _lWidth;
        lHeight = _lHeight;
    }
}

//--------------------------------------------------------------
// Sets the offsets for the local display.
//--------------------------------------------------------------
void mpeClientTCP::setOffsets(int _xOffset, int _yOffset) {
    if (_xOffset > -1 && _yOffset > -1) {
        xOffset = _xOffset;
        yOffset = _yOffset;
    }
}

//--------------------------------------------------------------
// Sets the dimensions for the local display.
// The offsets are used to determine what part of the Master Dimensions to render.
// For example, if you have two screens, each 100x100, and the master dimensions are 200x100
// then you would set
//  client 0: setLocalDimensions(0, 0, 100, 100);
//  client 1: setLocalDimensions(100, 0, 100, 100);
// for a 10 pixel overlap you would do:
//  client 0: setLocalDimensions(0, 0, 110, 100);
//  client 1: setLocalDimensions(90, 0, 110, 100);
//--------------------------------------------------------------
void mpeClientTCP::setLocalDimensions(int _xOffset, int _yOffset, int _lWidth, int _lHeight) {
    setOffsets(_xOffset, _yOffset);
    setLocalDimensions(_lWidth, _lHeight);
}

//--------------------------------------------------------------
// Sets the master dimensions for the Video Wall.
// This is used to calculate what is rendered.
//--------------------------------------------------------------
void mpeClientTCP::setMasterDimensions(int _mWidth, int _mHeight) {
    if (_mWidth > -1 && _mHeight > -1) {
        mWidth = _mWidth;
        mHeight = _mHeight;
    }
}

//--------------------------------------------------------------
// Sets the field of view of the camera when rendering in 3D.
// Note that this has no effect when rendering in 2D.
//--------------------------------------------------------------
void mpeClientTCP::setFieldOfView(float val) {
    fieldOfView = val;
    cameraZ = (ofGetHeight() / 2.f) / tanf(M_PI * fieldOfView/360.f);
}

//--------------------------------------------------------------
// Places the viewing area for this screen. This must be called at the
// beginning of the render loop.  If you are using Processing, you would
// typically place it at the beginning of your draw() function.
//--------------------------------------------------------------
void mpeClientTCP::placeScreen() {
    if (bEnable3D) {
        placeScreen3D();
    } else {
        placeScreen2D();
    }
}

//--------------------------------------------------------------
// If you want to enable or disable 3D manually in automode
//--------------------------------------------------------------
void mpeClientTCP::enable3D(bool _b) {
    bEnable3D = _b;
}

//--------------------------------------------------------------
// Places the viewing area for this screen when rendering in 2D.
//--------------------------------------------------------------
void mpeClientTCP::placeScreen2D() {
    glTranslatef(xOffset * -1, yOffset * -1, 0);
}


//--------------------------------------------------------------
// Restores the viewing area for this screen when rendering in 3D.
//--------------------------------------------------------------
void mpeClientTCP::restoreCamera() {
    gluLookAt(ofGetWidth()/2.f, ofGetHeight()/2.f, cameraZ,
              ofGetWidth()/2.f, ofGetHeight()/2.f, 0,
              0, 1, 0);

    float mod = .1f;
    glFrustum(-(ofGetWidth()/2.f)*mod, (ofGetWidth()/2.f)*mod,
              -(ofGetHeight()/2.f)*mod, (ofGetHeight()/2.f)*mod,
              cameraZ*mod, 10000);
}


//--------------------------------------------------------------
// Places the viewing area for this screen when rendering in 3D.
//--------------------------------------------------------------
void mpeClientTCP::placeScreen3D() {
    gluLookAt(mWidth/2.f, mHeight/2.f, cameraZ,
              mWidth/2.f, mHeight/2.f, 0,
              0, 1, 0);


    // The frustum defines the 3D clipping plane for each Client window!
    float mod = .1f;
    float left   = (xOffset - mWidth/2)*mod;
    float right  = (xOffset + lWidth - mWidth/2)*mod;
    float top    = (yOffset - mHeight/2)*mod;
    float bottom = (yOffset + lHeight-mHeight/2)*mod;

    //float far    = 10000;
    float a = 10;
    float Far = 10000;
    float Near   = cameraZ*mod;
    glFrustum(left, right,
              top, bottom,
              Near, Far);
}


//--------------------------------------------------------------
// Checks whether the given point is on screen.
//--------------------------------------------------------------
bool mpeClientTCP::isOnScreen(float _x, float _y) {
    return (_x > xOffset &&
            _x < (xOffset + lWidth) &&
            _y > yOffset &&
            _y < (yOffset + lHeight));
}

//--------------------------------------------------------------
// Checks whether the given rectangle is on screen.
//--------------------------------------------------------------
bool mpeClientTCP::isOnScreen(float _x, float _y, float _w, float _h) {
    return (isOnScreen(_x, _y) ||
            isOnScreen(_x + _w, _y) ||
            isOnScreen(_x + _w, _y + _h) ||
            isOnScreen(_x, _y + _h));
}

//--------------------------------------------------------------
// Outputs a message to the console.
//--------------------------------------------------------------
void mpeClientTCP::out(string _str) {
    //print(_str);
	//cout << _str << endl;
}

//--------------------------------------------------------------
// Outputs a message to the console.
//--------------------------------------------------------------
void mpeClientTCP::print(string _str) {
    if (DEBUG)
        cout << "mpeClient: " << _str << endl;
}

//--------------------------------------------------------------
// Outputs an error message to the console.
//--------------------------------------------------------------
void mpeClientTCP::err(string _str) {
    //cerr << "mpeClient: " << _str << endl;
	ofLog(OF_LOG_ERROR, "MPE Client Error :: " + _str);
}

//--------------------------------------------------------------
void mpeClientTCP::threadedFunction() {
    out("Running!");

    // let the server know that this client is ready to start
    send("S" + ofToString(id));
	
    while (isThreadRunning()) {
		if(!tcpClient.isConnected()){
			//we lost connection, start the retry loop and kill the thread
			lastConnectionAttempt =  ofGetElapsedTimef();
			ofAddListener(ofEvents.update, this, &mpeClientTCP::retryConnectionLoop);
			stopThread(true);
			frameCount = 0;
			return;
		}
		
        if (lock()) {
			
			//JG added this call conditional
			//we shouldn't consume more data until the current frame is finished rendered
//			if(!rendering){
				//string msg = tcpClient.receiveRaw();
				string msg = tcpClient.receive();
				if (msg.length() > 0 && lastmsg != msg) {
					read(msg);
                    lastmsg = msg;
					cout << "MPE Client :: Received " << msg << endl;
				}
//			}
			
			unlock();

            ofSleepMillis(5);
        }
    }
}

//--------------------------------------------------------------
// Reads and parses a message from the server.
//--------------------------------------------------------------
void mpeClientTCP::read(string _serverInput) {
    out("Receiving: " + _serverInput);

    char c = _serverInput.at(0);
	if(c == 'R'){
		//we received a reset signal
		frameCount = 0;
		cout << "Received frame reset" << endl;
	}
    else if (c == 'G' || c == 'B' || c == 'I') {
        if (!allConnected) {
            if (DEBUG) out("all connected!");
            allConnected = true;
        }
		
        // split into frame message and data message
        vector<string> info = ofSplitString(_serverInput, ":");
        vector<string> frameMessage = ofSplitString(info[0], ",");
        int fc = ofToInt(frameMessage[1]);

        if (info.size() > 1) {
            // there is a message here with the frame event
            info.erase(info.begin());
			
            //TODO: Track Byte/Int/Floats messages too
            for(int i = 0; i < info.size(); i++){
				if(useMainThread){
					dataMessage.push_back( info[i] );
				}
				else{
					ofxMPEEventArgs e;
					e.message = dataMessage[i];
					e.frame = getFrameCount();
					
					cout << "sending message in update " << e.frame << " message " << e.message << endl;
					
					ofNotifyEvent(ofxMPEEvents.mpeMessage, e);					
					
				}
				//cout << "MPE frame " << getFrameCount() << " receiving data message " << dataMessage[i] << endl;
            }
			
//			if(useMessageMode){
//				for(int i = 0; i < info.size(); i++){
//					ofSendMessage(info[i]);
//				}
//			}
//			else {
//                cout << " message is available " << endl;
//				bMessageAvailable = true;
//			}
		}
//        } else {
//    		//cout << " message not available " << endl;
////            bMessageAvailable = false;
//        }

        // assume no arrays are available
//        bIntsAvailable  = false;
//        bBytesAvailable = false;

        if (fc == frameCount) {
            frameCount++;

            // calculate new framerate
            float ms = ofGetElapsedTimeMillis() - lastMs;
            fps = 1000.f / ms;
            lastMs = ofGetElapsedTimeMillis();

			if(!useMainThread){
//				rendering = true;
				ofxMPEEventArgs e;
				e.message = "";
				e.frame = frameCount;
				ofNotifyEvent(ofxMPEEvents.mpeFrame, e);	
				done();
            }
			else {
				triggerFrame = true;
			}
        }
    }
}

//--------------------------------------------------------------
// Send a message to the server.
//--------------------------------------------------------------
void mpeClientTCP::send(string _msg) {
    out("Sending: " + _msg);

    _msg += "\n";
    //tcpClient.sendRaw(_msg);
	tcpClient.send(_msg);
}

//--------------------------------------------------------------
// Format a broadcast message and send it.
// Do not use a colon ':' in your message!!!
//--------------------------------------------------------------
void mpeClientTCP::broadcast(string _msg) {
    _msg = "T" + _msg;
    send(_msg);
}

//--------------------------------------------------------------
// Sends a "Done" command to the server. This must be called at
// the end of the draw loop.
//--------------------------------------------------------------
//TODO: if done has already been called, dont call it again
void mpeClientTCP::done() {	
//    rendering = false;
    string msg = "D," + ofToString(id) + "," + ofToString(frameCount);
    send(msg);
}

//--------------------------------------------------------------
// Stops the client thread.  You don't really need to do this ever.
//--------------------------------------------------------------
void mpeClientTCP::stop() {
    out("Quitting.");
    stopThread();
	if(useMainThread){
		ofRemoveListener(ofEvents.draw, this, &mpeClientTCP::draw);
	}
}



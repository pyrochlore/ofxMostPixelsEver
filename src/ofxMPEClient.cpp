/**
 *  openFrameworks version of the popular synchronization system Most Pixels Ever by Dan Shiffman
 *  original repo: https://github.com/shiffman/Most-Pixels-Ever
 *  our fork: https://github.com/FlightPhase/Most-Pixels-Ever
 *
 *  I affectionately refer to as "Most Pickles Ever" since it's gotten me out of the most pickles. ever!
 *
 *  Standing on the shoulders of the original creators:
 *  Dan Shiffman with Jeremy Rotsztain, Elie Zananiri, Chris Kairalla.
 *  Extended by James George on 5/17/11 @ Flightphase for the National Maritime Museum
 *
 *  Still need to convert the original examples to the new format
 *
 *  There is a drawback that this is not compatible with the Java MPE jar, the connections must go OF client to OF Server
 *
 */

#include "ofxMPEClient.h"
#include "ofxMPEEvents.h"

//--------------------------------------------------------------
ofxMPEClient::ofxMPEClient() {
    setDefaults();
}

//--------------------------------------------------------------
void ofxMPEClient::setDefaults(){
	verbose = false;
	frameLock = true;
	delimiter = "|";
	messageDelimiter = "T"+delimiter;
	id = 0;
	mWidth  = -1;
	mHeight = -1;
	lWidth  = 640;
	lHeight = 480;
	xOffset = 0;
	yOffset = 0;
	
	retryConnection = true;
	triggerFrame = false;
	
	lastConnectionAttempt = 0;
	isAttemptingToConnect = false;
	simulationMode = false;
	
	frameCount = 0;
	fps        = 0.0f;
	lastMs     = 0;
	
	allConnected = false;
	lastHeartbeatTime = 0.0;
	
	outgoingMessage = "";
	outgoingMessage.reserve(1000);
	lastmsg = "";
	
	goFullScreen = false;
	offsetWindow = false;
	
	bEnable3D    = false;
	fieldOfView = 30.f;
	
	clientName = "noname";
	
}

//--------------------------------------------------------------
void ofxMPEClient::setup(string _fileString, bool updateOnMainThread) {

    useMainThread = updateOnMainThread;

    fps = 0;
    loadIniFile(_fileString);
    frameCount = 0;
    shouldReset = false;
    heartbeatInterval = 0;
    timeOfNextHeartbeat = ofGetElapsedTimef();
}

//--------------------------------------------------------------
void ofxMPEClient::useSimulationMode(int framesPerSecond) {
    simulatedFPS = framesPerSecond;
    simulationMode = true;
    lastFrameTime = ofGetElapsedTimef();
}

//--------------------------------------------------------------
void ofxMPEClient::start() {

    tcpClient.setVerbose(verbose);
	tcpClient.setMessageDelimiter("\n");
    ofAddListener(ofEvents().draw, this, &ofxMPEClient::draw);

    if (!simulationMode && !tcpClient.setup(hostName, serverPort)) {
        error("TCP failed to connect to port " + ofToString(serverPort));
        lastConnectionAttempt = ofGetElapsedTimef();
        ofAddListener(ofEvents().update, this, &ofxMPEClient::retryConnectionLoop);
    }
    else{
        startThread(true, false);  // blocking, verbose
        log("TCP connection bound on port " + ofToString(serverPort));
    }
}

//--------------------------------------------------------------
void ofxMPEClient::retryConnectionLoop(ofEventArgs& e)
{
    float now = ofGetElapsedTimef();
    if(now - lastConnectionAttempt > 1.0){ //retry every second
        if(tcpClient.setup(hostName, serverPort)) {
            //cout << "retry succeeded, removing listener!" << endl;
            ofRemoveListener(ofEvents().update, this, &ofxMPEClient::retryConnectionLoop);
            startThread(true, false);  // blocking, verbose
        }
        lastConnectionAttempt = now;
    }
}

//--------------------------------------------------------------
void ofxMPEClient::draw(ofEventArgs& e)
{
    //no blocking
    if(useMainThread){
		
		lock();

        for(int i = 0; i < dataMessage.size(); i++){
            ofxMPEEventArgs e;
            e.message = dataMessage[i];
            e.frame = getFrameCount();
            //cout << "sending message in update " << e.frame << " message " << e.message << endl;

            ofNotifyEvent(ofxMPEEvents.mpeMessage, e);

        }
        dataMessage.clear();

        if(shouldReset){
            reset();
        }

        if(triggerFrame){
            //ofLog(OF_LOG_VERBOSE, "Trigger Event :: ! with frame count " + frameCount);
            triggerFrame = false;
            ofxMPEEventArgs e;
            e.message = "";
            e.frame = frameCount;
			ofNotifyEvent(ofxMPEEvents.mpeFrame, e);

            if(!simulationMode){
                done();
            }

        }

        unlock();
    }

    //cout << ofGetWidth() << " :: " << lWidth << "  " << ofGetHeight() << " :: " << lHeight << endl;
    if(ofGetWindowPositionX() != xOffset || ofGetWindowPositionY() != yOffset || ofGetWidth() != lWidth || ofGetHeight() != lHeight){
        setupViewport();
    }
}

//--------------------------------------------------------------
// Loads the settings from the Client XML file.
//--------------------------------------------------------------
void ofxMPEClient::loadIniFile(string _fileString) {
    log("Loading settings from file " + _fileString);
    
    ofxXmlSettings xmlReader;
    if (!xmlReader.loadFile(_fileString)){
        error("ERROR loading XML file!");
        return;
    }

    // parse INI file
    hostName   = xmlReader.getValue("settings:server:ip", "127.0.0.1", 0);
    serverPort = xmlReader.getValue("settings:server:port", 7887, 0);
    //turn this off if you don't want this client to sync frames but can still
    //receive messages. default is ON as that's the normal behavior
    frameLock = xmlReader.getValue("settings:framelock", true);
    if(frameLock){
        id         = xmlReader.getValue("settings:client_id", -1, 0);
        clientName = xmlReader.getValue("settings:client_name", "noname", 0);
    }
    else{
        cout << "opting out of frame lock" << endl;
    }
    cout << "***MPE:: HOST IS " << hostName << " Server Port is " << serverPort << endl;

    setLocalDimensions(xmlReader.getValue("settings:local_dimensions:width",  640, 0),
                       xmlReader.getValue("settings:local_dimensions:height", 480, 0));

    setOffsets(xmlReader.getValue("settings:local_location:x", 0, 0),
               xmlReader.getValue("settings:local_location:y", 0, 0));

    setMasterDimensions(xmlReader.getValue("settings:master_dimensions:width",  640, 0),
                        xmlReader.getValue("settings:master_dimensions:height", 480, 0));

    goFullScreen = xmlReader.getValue("settings:go_fullscreen", "false", 0).compare("true") == 0;
    offsetWindow = xmlReader.getValue("settings:offset_window", "false", 0).compare("true") == 0;

    setupViewport();

    if (xmlReader.getValue("settings:debug", 0, 0) == 1){
        verbose = true;
    }

    if(xmlReader.getValue("settings:simulation:on", 0, 0) == 1){
        useSimulationMode(xmlReader.getValue("settings:simulation:fps", 30));
        cout << "using simulation mode" << endl;
    }

    log("Settings: server = " + hostName + ":" + ofToString(serverPort) + ",  id = " + ofToString(id)
        + ", local dimensions = " + ofToString(lWidth) + ", " + ofToString(lHeight)
        + ", location = " + ofToString(xOffset) + ", " + ofToString(yOffset));
}

//--------------------------------------------------------------
void ofxMPEClient::setupViewport()
{
    if (goFullScreen){
        ofSetFullscreen(true);
    }
    if(offsetWindow){
        ofSetWindowPosition(xOffset, yOffset);
        ofSetWindowShape(lWidth, lHeight);
    }
}

//--------------------------------------------------------------
bool ofxMPEClient::isConnected()
{
    return tcpClient.isConnected();
}

//--------------------------------------------------------------
// Sets the dimensions for the local display.
//--------------------------------------------------------------
void ofxMPEClient::setLocalDimensions(int _lWidth, int _lHeight) {
    if (_lWidth > -1 && _lHeight > -1) {
        lWidth = _lWidth;
        lHeight = _lHeight;
    }
}

//--------------------------------------------------------------
// Sets the offsets for the local display.
//--------------------------------------------------------------
void ofxMPEClient::setOffsets(int _xOffset, int _yOffset) {
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
void ofxMPEClient::setLocalDimensions(int _xOffset, int _yOffset, int _lWidth, int _lHeight) {
    setOffsets(_xOffset, _yOffset);
    setLocalDimensions(_lWidth, _lHeight);
}

//--------------------------------------------------------------
// Sets the master dimensions for the Video Wall.
// This is used to calculate what is rendered.
//--------------------------------------------------------------
void ofxMPEClient::setMasterDimensions(int _mWidth, int _mHeight) {
    if (_mWidth > -1 && _mHeight > -1) {
        mWidth = _mWidth;
        mHeight = _mHeight;
    }
}

//--------------------------------------------------------------
// Sets the field of view of the camera when rendering in 3D.
// Note that this has no effect when rendering in 2D.
//--------------------------------------------------------------
void ofxMPEClient::setFieldOfView(float val) {
    fieldOfView = val;
    cameraZ = (ofGetHeight() / 2.f) / tanf(PI * fieldOfView/360.f);
}

//--------------------------------------------------------------
// Places the viewing area for this screen. This must be called at the
// beginning of the render loop.  If you are using Processing, you would
// typically place it at the beginning of your draw() function.
//--------------------------------------------------------------
void ofxMPEClient::placeScreen() {
    if (bEnable3D) {
        placeScreen3D();
    } else {
        placeScreen2D();
    }
}

//--------------------------------------------------------------
// If you want to enable or disable 3D manually in automode
//--------------------------------------------------------------
void ofxMPEClient::enable3D(bool _b) {
    bEnable3D = _b;
}

//--------------------------------------------------------------
// Places the viewing area for this screen when rendering in 2D.
//--------------------------------------------------------------
void ofxMPEClient::placeScreen2D() {
    glTranslatef(xOffset * -1, yOffset * -1, 0);
}


//--------------------------------------------------------------
// Restores the viewing area for this screen when rendering in 3D.
//--------------------------------------------------------------
void ofxMPEClient::restoreCamera() {
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
void ofxMPEClient::placeScreen3D() {
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
bool ofxMPEClient::isOnScreen(float _x, float _y) {
    return (_x > xOffset &&
            _x < (xOffset + lWidth) &&
            _y > yOffset &&
            _y < (yOffset + lHeight));
}

//--------------------------------------------------------------
// Checks whether the given rectangle is on screen.
//--------------------------------------------------------------
bool ofxMPEClient::isOnScreen(float _x, float _y, float _w, float _h) {
    return (isOnScreen(_x, _y) ||
            isOnScreen(_x + _w, _y) ||
            isOnScreen(_x + _w, _y + _h) ||
            isOnScreen(_x, _y + _h));
}

//--------------------------------------------------------------
// Outputs a message to the console.
//--------------------------------------------------------------
void ofxMPEClient::log(string _str) {
    //print(_str);
    //cout << _str << endl;
	if(verbose){
		ofLogNotice("MPE Client " + clientName + " : " + ofToString(id)) << _str;
	}
}

//--------------------------------------------------------------
// Outputs an error message to the console.
//--------------------------------------------------------------
void ofxMPEClient::error(string _str) {
    //cerr << "mpeClient: " << _str << endl;
	ofLogError("MPE Client " + clientName + " : " + ofToString(id)) << _str;
}

//--------------------------------------------------------------
void ofxMPEClient::threadedFunction() {
    log("Running!");
    

    
    if(frameLock){
        // let the server know that this client is ready to start
        send("S" + delimiter + ofToString(id) + delimiter + clientName);
    }
    else{
        //start a listener
        send("A" + delimiter + ofToString(id) + clientName + delimiter);
    }

    while(isThreadRunning()) {

        if(frameLock && simulationMode){
            
            float now = ofGetElapsedTimef();
			
            if(now - lastFrameTime > 1./simulatedFPS){
                if(!useMainThread){
                    ofxMPEEventArgs e;
                    e.message = "";
                    e.frame = frameCount;
                    ofNotifyEvent(ofxMPEEvents.mpeFrame, e);
                }
                else {
                    triggerFrame = true;
                }

                lastFrameTime = now;
                frameCount++;
            }

            ofSleepMillis(5);
            continue;
        }

        
//        if (allConnected && ofGetElapsedTimef() - lastHeartbeatTime > 2.0) {
//            //we lost connection... manually disconnect and join reset cycle
//            if(tcpClient.close()){
//                ofLog(OF_LOG_ERROR, "ofxMPEClient -- server connection timed out. Closing and entering reconnect loop.");
//            }
//            else{
//                ofLog(OF_LOG_ERROR, "ofxMPEClient -- Error when closing TCP connection after timeout.");            
//            }
//        }
        
        if(!tcpClient.isConnected()){
            //we lost connection, start the retry loop and kill the thread
            lastConnectionAttempt = ofGetElapsedTimef();
            ofAddListener(ofEvents().update, this, &ofxMPEClient::retryConnectionLoop);
            waitForThread(true);
			//stopThread();
            if(useMainThread){
                shouldReset = true;
            }
            else{
                reset();
            }

			error("lost connection to server");
            
			
            //break the loop because we'll need to restart
            return;
        }

        if (!useMainThread || (useMainThread && lock()) ) {
            string msg = tcpClient.receive();
            if (msg.length() > 0 && lastmsg != msg) {
                read(msg);
                lastmsg = msg;
            }

            if(useMainThread){
                unlock();
            }
        }
        ofSleepMillis(5);
    }
}

//--------------------------------------------------------------
// Reads and parses a message from the server.
//--------------------------------------------------------------
void ofxMPEClient::read(string _serverInput) {
    log("Receiving: " + _serverInput);

    char c = _serverInput.at(0);
    if(c == 'R'){
        if(frameCount != 0){
            //we received a reset signal
            if(useMainThread){
                shouldReset = true;
            }
            else{
                reset();
            }
            cout << "Received frame reset" << endl;
        }
    }
    else if (c == 'G') {
        if (!allConnected) {
            if (verbose) log("all connected!");
            allConnected = true;
        }
        
        lastHeartbeatTime = ofGetElapsedTimef();
        
        // split into frame message and data message
        vector<string> info = ofSplitString(_serverInput, delimiter);
        int fc = ofToInt( info[1] );
		//cout << "frameCount is " << fc << " and our current frame is " << frameCount << endl;
        if (frameLock && fc == frameCount) {
//            frameCount++;
            
            // calculate new framerate
            float nowms = ofGetElapsedTimeMillis();
            float ms = nowms - lastMs;
            fps += ((1000.f / ms) - fps)*.2;
            lastMs = nowms;

            if(!useMainThread){

                //cout << "trigger frame " << frameCount << endl;

                ofxMPEEventArgs e;
                e.message = "";
                e.frame = frameCount;
                ofNotifyEvent(ofxMPEEvents.mpeFrame, e);

                done();
            }
            else {
                //cout << "trigger frame " << frameCount << endl;
                triggerFrame = true;
            }
        }

         //JG switched to after done event
        if (info.size() > 1) {
            // there is a message here with the frame event
            info.erase(info.begin());

            for(int i = 0; i < info.size(); i++){
                if(useMainThread){
                    dataMessage.push_back( info[i] );
                }
                else{
                    ofxMPEEventArgs e;
                    e.message = info[i];
                    e.frame = getFrameCount();

                    //cout << "sending message in update " << e.frame << " message " << e.message << endl;

                    ofNotifyEvent(ofxMPEEvents.mpeMessage, e);

                }
                //cout << "MPE frame " << getFrameCount() << " receiving data message " << dataMessage[i] << endl;
            }
        }
    }
}

void ofxMPEClient::reset()
{
    ofxMPEEventArgs e;
    e.message = "reset";
    e.frame = frameCount;
    ofNotifyEvent(ofxMPEEvents.mpeReset, e);
    allConnected = false;
    
    shouldReset = false;
    frameCount = 0;
}

//void ofxMPEClient::sendPauseEvent()
//{
//    ofxMPEEventArgs e;
//    e.message = "pause";
//    e.frame = frameCount;
//
//    ofNotifyEvent(ofxMPEEvents.mpePause, e);
//}

//--------------------------------------------------------------
// Send a message to the server.
//--------------------------------------------------------------
void ofxMPEClient::send(const string &_msg) {

    //_msg += "\n";
    if(!simulationMode && frameLock){
        //log("Sending: " + _msg);
        tcpClient.send(_msg);
    }
}

//--------------------------------------------------------------
// Format a broadcast message and send it.
// Do not use a colon ':' in your message!!!
//--------------------------------------------------------------
void ofxMPEClient::broadcast(const string &_msg) {
    if(!simulationMode){
		bool add_return = _msg.find('\n') == string::npos;
		outgoingMessage += messageDelimiter;
		outgoingMessage += _msg;
		if(add_return){ outgoingMessage += "\n"; }
    }
}

//--------------------------------------------------------------
// Sends a "Done" command to the server. This must be called at
// the end of the draw loop.
//--------------------------------------------------------------
//TODO: if done has already been called, dont call it again
void ofxMPEClient::done() {
//    rendering = false;
    string msg;
	msg.reserve(256);
	msg += "D";
	msg += delimiter;
	msg += ofToString(id);
	msg += delimiter;
	msg += ofToString(frameCount);
	msg += "\n";
	//TODO: Messaging needs "T"....
	if(!outgoingMessage.empty()){
        msg += outgoingMessage;
		outgoingMessage.clear();
    }
		
    send(msg);
	frameCount++;
}

//--------------------------------------------------------------
// Stops the client thread.  You don't really need to do this ever.
//--------------------------------------------------------------
void ofxMPEClient::stop() {
    log("Quitting.");
    stopThread();
    if(useMainThread){
        ofRemoveListener(ofEvents().draw, this, &ofxMPEClient::draw);
    }
}



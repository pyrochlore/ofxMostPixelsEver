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

#pragma once

#include "ofMain.h"

#include "ofxNetwork.h"
#include "ofxXmlSettings.h"

//--------------------------------------------------------------
class ofxMPEClient : public ofThread {

  public:
    ofxMPEClient();

    //must call this before calling setup and it will work offline
    void  useSimulationMode(int framesPerSecond); //will work offline

    void setup(string _fileString, bool mainThreadOnly = true);
    //TODO: simple non-xml based setup method
    //void setup(string hostname, int clientId, int port, bool mainThreadOnly = true);

    void  done(); //done drawing

    void  start();
    void  stop();

//    void  draw();

    int   getPort() { return serverPort; }
    int   getID()   { return id; }


    //we need this incase the server launches after the clients
    void setDoesRetry(bool doRetry);

    void  setLocalDimensions(int _xOffset, int _yOffset, int _lWidth, int _lHeight);
    void  setMasterDimensions(int _mWidth, int _mHeight);

    // return dimensions in pixels
    int   getLWidth()  { return lWidth; }
    int   getLHeight() { return lHeight; }
    int   getXoffset() { return xOffset; }
    int   getYoffset() { return yOffset; }
    int   getMWidth()  { return mWidth; }
    int   getMHeight() { return mHeight; }

    void setClientName(string name){ clientName = name; }

    int   getFrameCount() { return frameCount; }
    float getFPS()        { return fps; }
    
//    bool  isRendering()   { return rendering; }
    bool isConnected();
    void  setFieldOfView(float _val);
    float getFieldOfView() { return fieldOfView; }

    //TODO: move camera abstractions into ofCamera()
    void  placeScreen();
    void  enable3D(bool _b);
    void  placeScreen2D();
    void  placeScreen3D();
    void  restoreCamera();

    bool  isOnScreen(float _x, float _y);
    bool  isOnScreen(float _x, float _y, float _w, float _h);

    void  broadcast(const string &_msg);
    bool  areAllConnected() { return allConnected; }


  protected:
    void setDefaults();

	bool verbose;
	string delimiter;
	string messageDelimiter;
	
    void draw(ofEventArgs& e);
    void retryConnectionLoop(ofEventArgs& e);

    void threadedFunction();

    void loadIniFile(string _fileString);

    void setLocalDimensions(int _lWidth, int _lHeight);
    void setOffsets(int _xOffset, int _yOffset);

    void log(string _msg);
    void error(string _msg);

    void setupViewport();

    void read(string _serverInput);
    void send(string &_msg);

    void reset();

    string       hostName;
    int          serverPort;
    ofxTCPClient tcpClient;

    /** The id is used for communication with the server, to let it know which
     *  client is speaking and how to order the screens. */
    int id;
	
    /** The total number of screens. */
    int numScreens;
    bool goFullScreen;
    bool offsetWindow;
    float lastHeartbeatTime;
    
    /** The master width. */
    int mWidth;
    /** The master height. */
    int mHeight;

    /** The local width. */
    int lWidth;
    /** The local height. */
    int lHeight;

    int xOffset;
    int yOffset;

    string outgoingMessage;

    bool frameLock;
    bool retryConnection; //TODO: retry connection
    bool isAttemptingToConnect;
    float lastConnectionAttempt;

    string clientName;

    bool triggerFrame;
    bool shouldReset;
    bool useMainThread;

    //simulation mode
    bool simulationMode;
    float lastFrameTime;
    int simulatedFPS;

    int   frameCount;
    float fps;
    long  lastMs;

    /** True if all the other clients are connected. */
    bool allConnected;

    bool bMessageAvailable;
    bool bIntsAvailable;
    bool bBytesAvailable;
    vector<string> dataMessage;
    float heartbeatInterval;
    float timeOfNextHeartbeat;

    string lastmsg; //used to ignore duplicates

    // 3D variables
    bool  bEnable3D;
    float fieldOfView;
    float cameraZ;
};

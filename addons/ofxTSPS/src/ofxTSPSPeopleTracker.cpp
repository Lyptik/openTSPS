
#include "ofxTSPSPeopleTracker.h"
#include "CPUImageFilter.h"

//scales down tracking images for improved performance
#define TRACKING_SCALE_FACTOR .5

//Fix for FMAX not in Visual Studio C++
#if defined _MSC_VER
#define fmax max
#define fmin min
#pragma warning (disable:4996)
#define snprintf sprintf_s
#endif

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#pragma mark Setup

//---------------------------------------------------------------------------
ofxTSPSPeopleTracker::ofxTSPSPeopleTracker(){
    p_Settings = NULL;
    hasMouseEvents = false;
    tspsProcessor = NULL;
}

//---------------------------------------------------------------------------
ofxTSPSPeopleTracker::~ofxTSPSPeopleTracker(){
    if ( hasMouseEvents ){
        hasMouseEvents = false;
        ofRemoveListener(ofEvents().mousePressed, this, &ofxTSPSPeopleTracker::mousePressed);
    }
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::setup(int w, int h, string settingsfile){	
    ofxAddTSPSListeners( this );
	ofAddListener(ofEvents().mousePressed, this, &ofxTSPSPeopleTracker::mousePressed);
	hasMouseEvents = true;
    
	width  = w;
	height = h;
	
    grayBg.allocate(width, height);
	grayImage.allocate(width, height);
	colorImage.allocate(width,height);
	grayImageWarped.allocate(width, height);
	colorImageWarped.allocate(width,height);
    grayDiff.allocate( width,height );
	
	//set tracker
	bOscEnabled = bTuioEnabled = bTcpEnabled = bWebSocketServerEnabled = bWebSocketClientEnabled = false;
	p_Settings = gui.getSettings();
	
	//setup gui
	gui.setup();
	gui.setupQuadGui( width, height );
	gui.loadSettings( settingsfile );
    
	activeHeight = ofGetHeight();
	activeWidth = ofGetWidth();
	activeViewIndex = 4;
	
	//setup view rectangles 
	
	cameraView.setup(width, height);
	adjustedView.setup(width, height);
	bgView.setup(width, height);
	processedView.setup(width, height);
	dataView.setup(width, height);
	
	updateViewRectangles();
	
	cameraView.setImage(&colorImage);
	cameraView.setTitle("Camera Source View", "Camera");
	cameraView.setColor(218,173,90);
	
	adjustedView.setImage(&grayImageWarped);
	adjustedView.setTitle("Adjusted Camera View", "Adjusted");
	adjustedView.setColor(174,139,138);
	
	bgView.setImage(&grayBg);
	bgView.setTitle("Background Reference View", "Background");
	bgView.setColor(213,105,68);
		
	processedView.setImage(&grayDiff);
	processedView.setTitle("Differenced View", "Differencing");
	processedView.setColor(113,171,154);
	
	dataView.setTitle("Data View", "Data");
	dataView.setColor(191,120,0);
	
	setActiveView(PROCESSED_VIEW);
	
	lastHaarFile = "";
    
    // setup default processor
    if ( tspsProcessor == NULL ){
        setProcessor( new ofxTSPSofxOpenCvProcessor() );
    }
    tspsProcessor->setup( width, height, &scene, &trackedPeople );
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::setProcessor ( ofxTSPSProcessor * _processor ){
    tspsProcessor = _processor;
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::onPersonEntered( ofxTSPSEventArgs & tspsEvent ){
    ofPoint centroid = tspsEvent.person->getCentroidNormalized(width, height);
    
	if(bTuioEnabled){
		tuioClient.cursorPressed(1.0*centroid.x/width, 1.0*centroid.y/height, tspsEvent.person->oid );
	}
	if(bOscEnabled){
		oscClient.personEntered(tspsEvent.person, centroid, width, height, p_Settings->bSendOscContours);
	}
	if(bTcpEnabled){
		tcpClient.personEntered(tspsEvent.person, centroid, width, height, p_Settings->bSendOscContours);
	}
	if( bWebSocketClientEnabled || bWebSocketServerEnabled ){
		webSocketServer.personEntered(tspsEvent.person, centroid, width, height, p_Settings->bSendOscContours);
	}
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::onPersonUpdated( ofxTSPSEventArgs & tspsEvent ){
    ofPoint centroid = tspsEvent.person->getCentroidNormalized(width, height);
    
    if (bTuioEnabled){
        ofPoint tuioCursor = tspsEvent.person->getCentroidNormalized(width, height);
        tuioClient.cursorDragged( tuioCursor.x, tuioCursor.y, tspsEvent.person->oid);
    }
    
    if (bOscEnabled){
        oscClient.personUpdated(tspsEvent.person, centroid, width, height, p_Settings->bSendOscContours);
    }
    
    if (bTcpEnabled){
        tcpClient.personUpdated(tspsEvent.person, centroid, width, height, p_Settings->bSendOscContours);
    }
    
    if ( bWebSocketClientEnabled || bWebSocketServerEnabled ){
        webSocketServer.personUpdated(tspsEvent.person, centroid, width, height, p_Settings->bSendOscContours);
    }
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::onPersonWillLeave( ofxTSPSEventArgs & tspsEvent ){
    ofPoint centroid = tspsEvent.person->getCentroidNormalized(width, height);
	if (bTuioEnabled) {
		tuioClient.cursorReleased(centroid.x, centroid.y, tspsEvent.person->oid);	
	}
	//send osc kill message if enabled
	if (bOscEnabled){
		oscClient.personWillLeave(tspsEvent.person, centroid, width, height, p_Settings->bSendOscContours);
	};
	
	//send tcp kill message if enabled
	if(bTcpEnabled){
		tcpClient.personWillLeave(tspsEvent.person, centroid, width, height, p_Settings->bSendOscContours);
	}
    
	if( bWebSocketClientEnabled || bWebSocketServerEnabled ){
		webSocketServer.personWillLeave(tspsEvent.person, centroid, width, height, p_Settings->bSendOscContours);
	}
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::resize( int w, int h ){
    width  = w;
	height = h;
	
	grayImage.allocate(width, height);
	colorImage.allocate(width,height);
	grayImageWarped.allocate(width, height);
	colorImageWarped.allocate(width,height);
    
	grayDiff.allocate(width, height);
	gui.setupQuadGui( width, height );
	
	activeViewIndex = 4;
	
	//setup view rectangles 
	
	cameraView.setup(width, height);
	adjustedView.setup(width, height);
	bgView.setup(width, height);
	processedView.setup(width, height);
	dataView.setup(width, height);
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::setHaarXMLFile(string haarFile){
	haarFile = "haar/" + haarFile;
	
	//check if haar file has changed
	if(lastHaarFile != haarFile){
		ofLog(OF_LOG_VERBOSE, "changing haar file to " + haarFile);
		tspsProcessor->setHaarXMLFile(haarFile);
		lastHaarFile = haarFile;
	}
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#pragma mark Setup Communication

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::setupTuio(string ip, int port){
	ofLog(OF_LOG_VERBOSE, "SEND TUIO");
	bTuioEnabled = true;
    if (p_Settings == NULL) p_Settings = gui.getSettings();
	p_Settings->oscPort = port;
	p_Settings->oscHost = ip;
	tuioClient.setup(ip, port);
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::setupOsc(string ip, int port)
{
	ofLog(OF_LOG_VERBOSE, "SEND OSC");
	bOscEnabled = true;
    if (p_Settings == NULL) p_Settings = gui.getSettings();
    p_Settings->oscPort = port;
	p_Settings->oscHost = ip;
	oscClient.setupSender(ip, port);
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::setupTcp(int port)
{
	bTcpEnabled = true;
	ofLog(OF_LOG_VERBOSE, "SEND TCP TO PORT "+port);
    if (p_Settings == NULL) p_Settings = gui.getSettings();
	p_Settings->tcpPort = port;
	tcpClient.setup(port);
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::setupWebSocketServer( int port)
{
	ofLog(OF_LOG_VERBOSE, "SEND WEBSOCKET SERVER ON PORT "+port);
    if (p_Settings == NULL) p_Settings = gui.getSettings();
    bWebSocketServerEnabled = true;
	p_Settings->webSocketServerPort = port;
    gui.setValueI("WSS_PORT", port);
    bWebSocketServerEnabled = webSocketServer.setupServer(port);
    p_Settings->bSendWebSocketServer = bWebSocketServerEnabled;
    gui.setValueB("SEND_WSS", p_Settings->bSendWebSocketServer);
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::setupWebSocketClient( string host, int port, bool bUseSSL, string channel) {
	ofLog(OF_LOG_VERBOSE, "SEND WEBSOCKET CLIENT AT ws://"+host+channel+":"+ofToString( port ));
    if (p_Settings == NULL) p_Settings = gui.getSettings();
    bWebSocketClientEnabled = true;
    p_Settings->webSocketHost = host;
	p_Settings->webSocketPort = port;
	p_Settings->webSocketChannel = channel;
    p_Settings->webSocketUseSSL = bUseSSL;
    bWebSocketClientEnabled = webSocketServer.setupClient( host, port, false, channel);
    p_Settings->bSendWebSocketClient = bWebSocketClientEnabled;
    gui.setValueS("WS_HOST", host);
    gui.setValueS("WS_PORT", ofToString( port ));
    gui.setValueS("WS_CHANNEL", channel);
    gui.setValueB("WS_USESSL", bUseSSL);
    gui.setValueB("SEND_WS", p_Settings->bSendWebSocketClient);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#pragma mark Track People

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::updateSettings()
{
    if (p_Settings == NULL) p_Settings = gui.getSettings();
	setHaarXMLFile(p_Settings->haarFile);
    
    //----------------------------------------------
	// Processor
	//----------------------------------------------
    
    // processor settings
    //processor->setTrackContours ( true );
    tspsProcessor->setTrackHaar ( p_Settings->bDetectHaar );
    tspsProcessor->setTrackSkeleton ( p_Settings->bTrackSkeleton );
    tspsProcessor->setTrackOpticalFlow( p_Settings->bTrackOpticalFlow );
    
    tspsProcessor->setThreshold( p_Settings->threshold );
    tspsProcessor->setBlobSettings( p_Settings->minBlob*width*height, p_Settings->maxBlob*width*height, p_Settings->bFindHoles);
    tspsProcessor->setOpticalflowMinMax( p_Settings->maxOpticalFlow, p_Settings->minOpticalFlow );
    tspsProcessor->setHaarPadding( p_Settings->haarAreaPadding );
    
    //----------------------------------------------
	// COMMUNICATION : Enable
	//----------------------------------------------
    
	//check to enable OSC
	if (p_Settings->bSendOsc && !bOscEnabled) setupOsc(p_Settings->oscHost, p_Settings->oscPort);
	else if (!p_Settings->bSendOsc) bOscEnabled = false;
	
	//check to enable TUIO
	if (p_Settings->bSendTuio && !bTuioEnabled) setupTuio(p_Settings->tuioHost, p_Settings->tuioPort);
	else if (!p_Settings->bSendTuio) bTuioEnabled = false;

	//check to enable TCP
	if (p_Settings->bSendTcp && !bTcpEnabled) setupTcp(p_Settings->tcpPort);
	else if (!p_Settings->bSendTcp) bTcpEnabled = false;
        
    //check to enable websockets
    if (p_Settings->bSendWebSocketClient && !bWebSocketClientEnabled){
        setupWebSocketClient(p_Settings->webSocketHost, p_Settings->webSocketPort, p_Settings->webSocketUseSSL, p_Settings->webSocketChannel);
    } else if (!p_Settings->bSendWebSocketClient){
        bWebSocketClientEnabled = false;
        webSocketServer.closeClient();
    }
    if (p_Settings->bSendWebSocketServer && !bWebSocketServerEnabled){
        setupWebSocketServer(p_Settings->webSocketServerPort);
    } 
    if (!p_Settings->bSendWebSocketServer){
        bWebSocketServerEnabled = false;
        webSocketServer.closeServer();
    }
    //----------------------------------------------
	// COMMUNICATION : Send data
	//----------------------------------------------	
    
	if(bTuioEnabled){
		tuioClient.update();		
	}
	
	if (bOscEnabled){
		oscClient.ip = p_Settings->oscHost;
		oscClient.port = p_Settings->oscPort;
		oscClient.useLegacy = p_Settings->bUseLegacyOsc;
		oscClient.update();
	};
    
	if (bTcpEnabled){
		tcpClient.port = p_Settings->oscPort;
		tcpClient.update();
		tcpClient.send();
	}
    
    if ( bWebSocketClientEnabled || bWebSocketServerEnabled ){
        if ( bWebSocketClientEnabled && (p_Settings->webSocketPort != webSocketServer.getPort() || p_Settings->webSocketHost != webSocketServer.getHost()) ){
            webSocketServer.closeClient();
            setupWebSocketClient( p_Settings->webSocketHost, p_Settings->webSocketPort );
        }
        
        if ( bWebSocketServerEnabled && p_Settings->webSocketServerPort != webSocketServer.getServerPort() ){
            webSocketServer.closeServer();
            setupWebSocketServer( p_Settings->webSocketServerPort );
        }
        
        //sent automagically
        webSocketServer.send();
    }
    
	//switch camera view if new panel is selected
	if (p_Settings->currentPanel != p_Settings->lastCurrentPanel) setActiveView(p_Settings->currentPanel + 1);

	// Set the current view within the gui so the image can only be warped when in Camera View
	if (cameraView.isActive()) {
		gui.changeGuiCameraView(true);
	} else {
		gui.changeGuiCameraView(false);
	}
}

/**
 * Core Method
 * Run every frame to update
 * the system to the current location
 * of people
 */
//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::trackPeople()
{	
    if (p_Settings == NULL) p_Settings = gui.getSettings();
    	
    //-------------------------------
    // Camera view
    //-------------------------------
    
	//warp background
    //colorImage = grayImage;
    //colorImageWarped = colorImage;
    getQuadSubImage(&grayImage, &grayImageWarped, &p_Settings->quadWarpScaled, 1);	
	
    // update scaled down images
	grayDiff = grayImageWarped;
	
	//amplify
	if(p_Settings->bAmplify){
		grayDiff.amplify(grayDiff, p_Settings->highpassAmp/15.0f);
	}
	
	grayImageWarped = grayDiff;
	
    //learn background
	if (p_Settings->bLearnBackground){
        grayBg = grayImageWarped;
        tspsProcessor->captureBackground( grayImageWarped );
	}
    
    //progressive relearn background
	if (p_Settings->bLearnBackgroundProgressive){
		tspsProcessor->progressiveBackground( grayImageWarped, p_Settings->fLearnRate * .00001 );
	}
    
    // black out background?
    if (p_Settings->bBlankBackground){
        tspsProcessor->blankBackground();
    }
    
	//-----------------------
	// Difference image
	//-----------------------
	grayDiff.setFromPixels( tspsProcessor->difference( grayImageWarped, (TSPSTrackingType) p_Settings->trackType ) );
    
	//-----------------------
	// Post-difference filters
	//-----------------------
    
	if(p_Settings->bSmooth){
		grayDiff.blur((p_Settings->smooth * 2) + 1); //needs to be an odd number
	}
	
	//highpass filter
	if(p_Settings->bHighpass){
		grayDiff.highpass(p_Settings->highpassBlur, p_Settings->highpassNoise);
	}
	
	//-----------------------
	// Track
	//-----------------------	
    grayDiff.setFromPixels( tspsProcessor->process( grayDiff ) );
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#pragma mark Draw
//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::draw()
{
	draw(0,0);
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::draw(int x, int y)
{
	draw(x,y,drawMode);
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::draw(int x, int y, int mode)
{
    if (p_Settings == NULL) p_Settings = gui.getSettings();
    // run lean + mean if we're minimized
    if (p_Settings->bMinimized) return;
	ofPushMatrix();
		ofTranslate(x, y, 0);
		// draw the incoming, the grayscale, the bg and the thresholded difference
		ofSetHexColor(0xffffff);
	
        // draw gui
        gui.draw();
    
		//draw large image
		if (activeViewIndex ==  CAMERA_SOURCE_VIEW){
			cameraView.drawLarge(activeView.x, activeView.y, activeView.width, activeView.height);		
			gui.drawQuadGui( activeView.x, activeView.y, activeView.width, activeView.height );
		} else if ( activeViewIndex == ADJUSTED_CAMERA_VIEW){
			adjustedView.drawLarge(activeView.x, activeView.y, activeView.width, activeView.height);				
		} else if ( activeViewIndex == REFERENCE_BACKGROUND_VIEW){
			bgView.drawLarge(activeView.x, activeView.y, activeView.width, activeView.height);			
		} else if ( activeViewIndex == PROCESSED_VIEW){ 
			processedView.drawLarge(activeView.x, activeView.y, activeView.width, activeView.height);
		} else if ( activeViewIndex == DATA_VIEW ){
			ofPushMatrix();
				ofTranslate(activeView.x, activeView.y);
                drawBlobs(activeView.width, activeView.height);
                ofPushMatrix();{
                    ofScale( (float) activeView.width / width , (float) activeView.height / height );
                    tspsProcessor->draw();
                } ofPopMatrix();
			ofPopMatrix();
			dataView.drawLarge(activeView.x, activeView.y, activeView.width, activeView.height);
		}
		
		//draw all images small
		cameraView.draw();
		adjustedView.draw();
		bgView.draw();
		processedView.draw();
		dataView.draw();	
		
		ofPushMatrix();
			ofTranslate(dataView.x, dataView.y);
			drawBlobs(dataView.width, dataView.height);
		ofPopMatrix();
		
	ofPopMatrix();
	
	//draw framerate in a box
	
	char frmrate[1024];
	sprintf(frmrate, "Frame rate: %f", ofGetFrameRate() );
	
	ofPushStyle();
	ofFill();
	ofSetColor(196,182,142);
	ofRect(cameraView.x, cameraView.y + cameraView.height + spacing*3 + 8, cameraView.width*2 + spacing, spacing*4);
	ofPopStyle();
	
	if (!bFontLoaded) ofDrawBitmapString(frmrate, cameraView.x + 10, cameraView.y + 10 + cameraView.height + spacing*5);
	else font.drawString(frmrate, (int)cameraView.x + 10, (int) (cameraView.y + 10 + cameraView.height + spacing*5));
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::drawBlobs( float drawWidth, float drawHeight){
	
    if (p_Settings == NULL) p_Settings = gui.getSettings();
	float scaleVar = (float) drawWidth/width;
	
	ofFill();
	ofSetHexColor(0x333333);
	ofRect(0,0,drawWidth,drawHeight);
	ofSetHexColor(0xffffff);
	
	ofNoFill();
	
	ofPushMatrix();
	ofScale(scaleVar, scaleVar);
	
	// simpler way to draw contours: contourFinder.draw();
	for (int i=0; i < trackedPeople.size(); i++){
		
		//draw blobs				
		//if haarfinder is looking at these blobs, draw the area it's looking at
		ofxTSPSPerson* p = trackedPeople[i];
		
		//draw contours 
		ofPushStyle();
		ofNoFill();
		if (p_Settings->bSendOscContours){
			ofSetHexColor(0x3abb93);
		} else {
			ofSetHexColor(0xc4b68e);
		}
		ofBeginShape();
		for( int j=0; j<p->contour.size(); j++ ) {
			ofVertex( p->contour[j].x, p->contour[j].y );
		}
		ofEndShape();
		ofPopStyle();
		
		if(p_Settings->bTrackOpticalFlow){
			//purple optical flow arrow
			ofSetHexColor(0xff00ff);
			//JG Doesn't really provide any helpful information since its so scattered
//			ofLine(p->centroid.x, 
//				   p->centroid.y, 
//				   p->centroid.x + p->opticalFlowVectorAccumulation.x, 
//				   p->centroid.y + p->opticalFlowVectorAccumulation.y);
		}
		
        // draw haar
        
		ofSetHexColor(0xffffff);							
		if(p_Settings->bDetectHaar){
			ofSetHexColor(0xee3523);
			//draw haar search area expanded 
			//limit to within data box so it's not confusing				
            ofRectangle haarRect = ofRectangle(p->boundingRect.x - p_Settings->haarAreaPadding, 
                                               p->boundingRect.y - p_Settings->haarAreaPadding, 
                                               p->boundingRect.width  + p_Settings->haarAreaPadding*2, 
                                               p->boundingRect.height + p_Settings->haarAreaPadding*2);
            if (haarRect.x < 0){
                haarRect.width += haarRect.x;
                haarRect.x = 0;					
            }
            if (haarRect.y < 0){
                haarRect.height += haarRect.y;	
                haarRect.y = 0;
            }
            if (haarRect.x + haarRect.width > width) haarRect.width = width-haarRect.x;
            if (haarRect.y + haarRect.height > height) haarRect.height = height-haarRect.y;
            ofRect(haarRect.x, haarRect.y, haarRect.width, haarRect.height);

            if(p->hasHaarRect()){
                //draw the haar rect
                ofSetHexColor(0xee3523);
                ofRect(p->getHaarRect().x, p->getHaarRect().y, p->getHaarRect().width, p->getHaarRect().height);
                //haar-detected people get a red square
                ofSetHexColor(0xfd5f4f);
            } else {
                //no haar gets a yellow square
                ofSetHexColor(0xeeda00);
            }
		} else {
			//no haar gets a yellow square
			ofSetHexColor(0xeeda00);
		}
        		
		//draw person
		ofRect(p->boundingRect.x, p->boundingRect.y, p->boundingRect.width, p->boundingRect.height);
		
        // draw highest point
        ofSetHexColor(0xff00ff);	
        ofCircle(p->highest.x, p->highest.y, 4);
        
		//draw centroid
		ofSetHexColor(0xff0000);
		ofCircle(p->centroid.x, p->centroid.y, 3);
		
		//draw id
		ofSetHexColor(0xffffff);
		char idstr[1024];
		sprintf(idstr, "pid: %d\noid: %d\nage: %d", p->pid, p->oid, p->age );
		ofDrawBitmapString(idstr, p->centroid.x+8, p->centroid.y);													
	}
	ofPopMatrix();
	ofSetHexColor(0xffffff);				
	//ofDrawBitmapString("blobs and optical flow", 5, height - 5 );
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#pragma mark mouse

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::mousePressed( ofMouseEventArgs &e )
{
	if (isInsideRect(e.x, e.y, cameraView)){
		activeViewIndex = CAMERA_SOURCE_VIEW;
		cameraView.setActive();
		adjustedView.setActive(false);
		bgView.setActive(false);
		processedView.setActive(false);
		dataView.setActive(false);
	} else if (isInsideRect(e.x, e.y, adjustedView)){
		activeViewIndex = ADJUSTED_CAMERA_VIEW;
		adjustedView.setActive();
		cameraView.setActive(false);
		bgView.setActive(false);
		processedView.setActive(false);
		dataView.setActive(false);
	} else if (isInsideRect(e.x, e.y, bgView)){
		activeViewIndex = REFERENCE_BACKGROUND_VIEW;
		bgView.setActive();
		cameraView.setActive(false);
		adjustedView.setActive(false);
		processedView.setActive(false);
		dataView.setActive(false);
	} else if (isInsideRect(e.x, e.y, processedView)){
		activeViewIndex = PROCESSED_VIEW;
		processedView.setActive();
		cameraView.setActive(false);
		adjustedView.setActive(false);
		bgView.setActive(false);
		dataView.setActive(false);
	} else if (isInsideRect(e.x, e.y, dataView)){
		activeViewIndex = DATA_VIEW;
		dataView.setActive();
		cameraView.setActive(false);
		adjustedView.setActive(false);
		bgView.setActive(false);
		processedView.setActive(false);
	}
}

//---------------------------------------------------------------------------
bool ofxTSPSPeopleTracker::isInsideRect(float x, float y, ofRectangle rect){
    return ( x >= rect.x && x <= rect.x + rect.width && y >= rect.y && y <= rect.y + rect.height );
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#pragma mark gui extension


//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::enableGuiEvents(){
    gui.enableEvents();
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::disableGuiEvents(){
    gui.disableEvents();    
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::addSlider(string name, int* value, int min, int max){
	//forward to the gui manager
	gui.addSlider(name, value, min, max);
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::addSlider(string name, float* value, float min, float max){
	gui.addSlider(name, value, min, max);	
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::addToggle(string name, bool* value){
	gui.addToggle(name, value);	
}

//---------------------------------------------------------------------------
guiTypeButton * ofxTSPSPeopleTracker::addExternalButton( string name, ofRectangle dimensions){
	return gui.addButton( name, dimensions );
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#pragma mark accessors

/**
 * simple public getter for external classes
 */
//---------------------------------------------------------------------------
ofxTSPSPerson* ofxTSPSPeopleTracker::personAtIndex(int i) {
	return trackedPeople[i];
}

//---------------------------------------------------------------------------
int ofxTSPSPeopleTracker::totalPeople() {
	return trackedPeople.size();
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::enableHaarFeatures(bool doHaar){
    if (p_Settings == NULL) p_Settings = gui.getSettings();
	p_Settings->bDetectHaar = doHaar;
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::enableOpticalFlow(bool doOpticalFlow){
    if (p_Settings == NULL) p_Settings = gui.getSettings();
	p_Settings->bTrackOpticalFlow = doOpticalFlow;
}

//---------------------------------------------------------------------------
// for accessing the OSC sender whose parameters are adjusted in the GUI
ofxTSPSOscSender* ofxTSPSPeopleTracker::getOSCsender() {
	return &oscClient;
}

//---------------------------------------------------------------------------
ofxTSPSWebSocketSender * ofxTSPSPeopleTracker::getWebSocketServer(){
    return &webSocketServer;
}


//---------------------------------------------------------------------------
bool ofxTSPSPeopleTracker::useKinect(){
    if (p_Settings == NULL) p_Settings = gui.getSettings();
    return p_Settings->bUseKinect;
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::setUseKinect( bool bUseKinect ){
    gui.setValueB( "USE_KINECT", bUseKinect );
    gui.update();
    if (p_Settings == NULL) p_Settings = gui.getSettings();
    p_Settings->bUseKinect = bUseKinect;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#pragma mark background management
//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::relearnBackground()
{
    if (p_Settings == NULL) p_Settings = gui.getSettings();
	p_Settings->bLearnBackground = true;
}

//JG Disabled this feature
//void ofxTSPSPeopleTracker::enableBackgroundRelearnSmart(bool doSmartLearn)//auto-relearns if there are too many blobs in the scene
//{
//	p_Settings->bSmartLearnBackground = doSmartLearn;
//}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::enableBackgroundReleaernProgressive(bool doProgressive) //relearns over time using progessive frame averagering
{
    if (p_Settings == NULL) p_Settings = gui.getSettings();
	p_Settings->bLearnBackgroundProgressive = doProgressive;
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::setRelearnRate(float relearnRate)
{
    if (p_Settings == NULL) p_Settings = gui.getSettings();
	p_Settings->fLearnRate = relearnRate;
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#pragma mark image control
//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::setThreshold(float thresholdAmount)
{
    if (p_Settings == NULL) p_Settings = gui.getSettings();
	p_Settings->threshold = thresholdAmount;
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::setMinBlobSize(float minBlobSize)
{
    if (p_Settings == NULL) p_Settings = gui.getSettings();
	p_Settings->minBlob = minBlobSize; 
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::setMaxBlobSize(float maxBlobSize)
{
    if (p_Settings == NULL) p_Settings = gui.getSettings();
	p_Settings->maxBlob = maxBlobSize;
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::enableSmooth(bool doSmooth)
{
    if (p_Settings == NULL) p_Settings = gui.getSettings();
	p_Settings->bSmooth = doSmooth;
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::setSmoothAmount(int smoothAmount)
{
    if (p_Settings == NULL) p_Settings = gui.getSettings();
	p_Settings->smooth = smoothAmount;
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::enableHighpass(bool doHighpass)
{
    if (p_Settings == NULL) p_Settings = gui.getSettings();
	p_Settings->bHighpass = doHighpass;
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::setHighpassBlurAmount(int highpassBlurAmount)
{
    if (p_Settings == NULL) p_Settings = gui.getSettings();
	p_Settings->highpassBlur = highpassBlurAmount;
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::setHighpassNoiseAmount(int highpassNoiseAmount)
{
    if (p_Settings == NULL) p_Settings = gui.getSettings();
	p_Settings->highpassNoise = highpassNoiseAmount;
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::enableAmplify(bool doAmp)
{
    if (p_Settings == NULL) p_Settings = gui.getSettings();
	p_Settings->bAmplify = doAmp;
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::setAmplifyAmount(int amplifyAmount)
{
    if (p_Settings == NULL) p_Settings = gui.getSettings();
	p_Settings->highpassAmp = amplifyAmount;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#pragma mark filter controls
//haar
//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::setHaarExpandArea(float haarExpandAmount) //makes the haar rect +area bigger
{
    if (p_Settings == NULL) p_Settings = gui.getSettings();
	p_Settings->haarAreaPadding = haarExpandAmount;
}

//blobs
//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::enableFindHoles(bool findHoles)
{
    if (p_Settings == NULL) p_Settings = gui.getSettings();
	p_Settings->bFindHoles = findHoles;
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::trackDarkBlobs()
{
    if (p_Settings == NULL) p_Settings = gui.getSettings();
	p_Settings->trackType = TRACK_DARK;
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::trackLightBlobs()
{
    if (p_Settings == NULL) p_Settings = gui.getSettings();
	p_Settings->trackType = TRACK_LIGHT;	
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::setDrawMode(int mode)
{
	drawMode = mode;
}

//---------------------------------------------------------------------------
int ofxTSPSPeopleTracker::getDrawMode() 
{
	return drawMode;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#pragma mark gui customization

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::setActiveView( int viewIndex ){
	int oldActiveView = activeViewIndex;
	activeViewIndex = viewIndex;
	
	if (activeViewIndex == CAMERA_SOURCE_VIEW){
		cameraView.setActive();
		adjustedView.setActive(false);
		bgView.setActive(false);
		processedView.setActive(false);
		dataView.setActive(false);
	} else if (activeViewIndex == ADJUSTED_CAMERA_VIEW){
		adjustedView.setActive();
		cameraView.setActive(false);
		bgView.setActive(false);
		processedView.setActive(false);
		dataView.setActive(false);
	} else if (activeViewIndex == REFERENCE_BACKGROUND_VIEW){
		bgView.setActive();
		cameraView.setActive(false);
		adjustedView.setActive(false);
		processedView.setActive(false);
		dataView.setActive(false);
	} else if (activeViewIndex == PROCESSED_VIEW){
		processedView.setActive();		cameraView.setActive(false);
		adjustedView.setActive(false);
		bgView.setActive(false);
		dataView.setActive(false);
	} else if (activeViewIndex == DATA_VIEW){
		dataView.setActive();
		cameraView.setActive(false);
		adjustedView.setActive(false);
		bgView.setActive(false);
		processedView.setActive(false);
	} else {
		activeViewIndex = oldActiveView;
	}
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::setActiveDimensions ( int actWidth, int actHeight){
	activeWidth = actWidth;
	activeHeight = actHeight;
	updateViewRectangles();
}

//---------------------------------------------------------------------------
bool ofxTSPSPeopleTracker::loadFont( string fontName, int fontSize){
	bFontLoaded = font.loadFont(fontName, fontSize);
    if (bFontLoaded){
        cameraView.setFont(&font);
        adjustedView.setFont(&font);
        bgView.setFont(&font);
        processedView.setFont(&font);
        dataView.setFont(&font);
    }
    return bFontLoaded;
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::setVideoGrabber(ofBaseVideo* grabber, tspsInputType inputType)
{
    if (p_Settings == NULL) p_Settings = gui.getSettings();
	p_Settings->setVideoGrabber( grabber, inputType );
    if (inputType == TSPS_INPUT_VIDEO){
        gui.enableElement( "open video settings" );
        //gui.disableElement( "use kinect" );
    } else if (inputType == TSPS_INPUT_KINECT){
        gui.disableElement( "open video settings" );
        //gui.enableElement( "use kinect" );
    }
}

//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::updateViewRectangles(){
	//build all rectangles for drawing views
	ofPoint smallView;
	smallView.x = (activeWidth - GUI_WIDTH - spacing*6)/5.f;
	smallView.y = (height*TRACKING_SCALE_FACTOR) * (smallView.x/(width*TRACKING_SCALE_FACTOR));
	
	activeView.x = GUI_WIDTH + spacing;
	activeView.y = spacing;
	activeView.width = (activeWidth - GUI_WIDTH - spacing*2);
	activeView.height = (height*TRACKING_SCALE_FACTOR)*activeView.width/(width*TRACKING_SCALE_FACTOR);
	
	cameraView.x = GUI_WIDTH + spacing;
	cameraView.y = activeView.y + activeView.height + spacing;
	cameraView.width = smallView.x;
	cameraView.height = smallView.y;
	
	adjustedView.x = cameraView.x + cameraView.width + spacing;
	adjustedView.y = cameraView.y;
	adjustedView.width = smallView.x;
	adjustedView.height = smallView.y;
	
	bgView.x = adjustedView.x + adjustedView.width + spacing;
	bgView.y = cameraView.y;
	bgView.width = smallView.x;
	bgView.height = smallView.y;
	
	processedView.x = bgView.x + bgView.width + spacing;
	processedView.y = cameraView.y;
	processedView.width = smallView.x;
	processedView.height = smallView.y;
	
	dataView.x = processedView.x + processedView.width + spacing;
	dataView.y = cameraView.y;
	dataView.width = smallView.x;
	dataView.height = smallView.y;	
	gui.drawQuadGui( activeView.x, activeView.y, activeView.width, activeView.height );
}

//---------------------------------------------------------------------------
bool ofxTSPSPeopleTracker::useVideoFile(){    
    if (p_Settings == NULL) p_Settings = gui.getSettings();
    return p_Settings->bUseVideoFile;
}


//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::setUseVideoFile( bool bUseVideoFile ){
    if (p_Settings == NULL) p_Settings = gui.getSettings();
    p_Settings->bUseVideoFile = bUseVideoFile;
}

//---------------------------------------------------------------------------
string ofxTSPSPeopleTracker::getVideoFile(){
    if (p_Settings == NULL) p_Settings = gui.getSettings();
    return p_Settings->videoFile;    
}
//---------------------------------------------------------------------------
void ofxTSPSPeopleTracker::setVideoFile( string file ){
    if (p_Settings == NULL) p_Settings = gui.getSettings();
    p_Settings->videoFile = file;    
}

//---------------------------------------------------------------------------
// for accessing which view is the current view
bool ofxTSPSPeopleTracker::inCameraView() {
	return cameraView.isActive();
}

//---------------------------------------------------------------------------
bool ofxTSPSPeopleTracker::inBackgroundView() {
	return bgView.isActive();
}

//---------------------------------------------------------------------------
bool ofxTSPSPeopleTracker::inDifferencingView() {
	return processedView.isActive();
}

//---------------------------------------------------------------------------
bool ofxTSPSPeopleTracker::inDataView() {
	return dataView.isActive();
}

//---------------------------------------------------------------------------
bool ofxTSPSPeopleTracker::inAdjustedView() {
	return adjustedView.isActive();
}


// for getting a color version of the adjusted view image
// NOTE:  only works if the adjusted view is currently in color
//        (this parameter can be set in the GUI under the 'views' tab)
ofxCvColorImage ofxTSPSPeopleTracker::getAdjustedImageInColor() {
    if (p_Settings == NULL) p_Settings = gui.getSettings();
	if (p_Settings->bAdjustedViewInColor)
		return *adjustedView.getColorImage();
}

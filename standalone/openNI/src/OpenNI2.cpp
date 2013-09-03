//
//  OpenNI2.cpp
//  openTSPS
//
//  Created by Andrew on 8/7/13.
//
//
#include "OpenNI2.h"
#include "ofxTSPS/source/Source.h"

// This is a source, this helper identitifes us to the system
ofxTSPS::SourceType ofxTSPS::OpenNI2::getType(){return CAMERA_CUSTOM;}


// Constructor initialize an object with unintialized device
ofxTSPS::OpenNI2::OpenNI2(){
    // type defaults to CAMERA_CUSTOM
    bCanTrackHaar = false;
    bDepthSetup = false;
    bDoProcessFrame = false;
    device = NULL;
    nearClipping = 0;
    farClipping = -1; // will be reset to max value
}


// Open the source, which instantiates the device
bool ofxTSPS::OpenNI2::openSource(int width, int height, string etc){
    
    // Initialize and setup if device is null
    if ( device == NULL ){
        cout << "OPENNI2: Initializing device..." <<endl;
        device = new ofxNI2::Device;
        
        // Setup with or without etc
        if(etc == "")
            device->setup();
        else
            device->setup(etc);
        
    }
    
    // Set up depth if not correct
    if ( !bDepthSetup ){
        cout << "OPENNI2: BISOPEN setup..." << endl;
        bIsOpen = setup(*device);
        if ( farClipping == -1 ) farClipping = stream.getMaxPixelValue();
        setFps(30);
        bDepthSetup  = bIsOpen;
    } else {
        bIsOpen = true;
    }
    
    if (bIsOpen){start();}
    
    
    // Now set up user tracker
    cout << "OPENNI2: Tracker setup..."<<endl;
    
    // This works on ONI files but breaks on live
    //userTracker.setup(*device);

    
    return bIsOpen;

}



// Destructor destroys the decice
ofxTSPS::OpenNI2::~OpenNI2(){
    device->exit();
    delete device;
}


// Device available?
bool ofxTSPS::OpenNI2::available(){
    
    if (device == NULL )
        return false;
    
    return (device->listDevices() >= 1);
}

// Update:
void ofxTSPS::OpenNI2::update(){
    device->update();
    if ( !bDoProcessFrame ){
        bDoProcessFrame = isFrameNew();
    }
    updateTextureIfNeeded();
    
    #ifdef TARGET_OSX
    publishToSyphon( getTextureReference() );
    #endif
}

// getPixels:
unsigned char * ofxTSPS::OpenNI2::getPixels(){
    return getPixelsRef().getPixels();
}

// getPixelsRef:
ofPixels & ofxTSPS::OpenNI2::getPixelsRef(){
    static ofPixels retPix;
    currentDepthBufferRef = ofxNI2::DepthStream::getPixelsRef();
    depthRemapToRange(currentDepthBufferRef, retPix, nearClipping, farClipping, false);
    return retPix;
}

// Per frame
bool ofxTSPS::OpenNI2::doProcessFrame(){
    bool bReturn = bDoProcessFrame;
    if ( bDoProcessFrame ) bDoProcessFrame = false;
    return bReturn;
}

// Convert depthmap value to range
inline void ofxTSPS::OpenNI2::depthRemapToRange(const ofShortPixels &src, ofPixels &dst, int near, int far, int invert){
    
    // If invert, then we swap near and far
    // Why? Who knows... we always call it with FALSE
    if (invert)
        std::swap(near, far);
    
    // Iterate over source and create destination
    dst.allocate(src.getWidth(), src.getHeight(), 1);
    const unsigned short *src_ptr = src.getPixels();
    unsigned char *dst_ptr = dst.getPixels();
    
    // Linear array of pixels (W x H)
    for (int i=0; i<(src.getWidth()*src.getHeight()); i++){
        // Set out of range values to 0
        unsigned short displayPixel = *src_ptr;
        
        if (displayPixel < near || displayPixel > far){
            displayPixel=0;
        }
        
        // For remaining nonzero values, remap 0-255
        *dst_ptr = (displayPixel==0?0:ofMap(displayPixel, near, far, 0, 255, false));
        src_ptr++;
        dst_ptr++;
    }
}

// Close the source
void ofxTSPS::OpenNI2::closeSource(){
    stream.stop();
    bIsOpen = false;
}

// Return the device handle (fail if it isn't initialized)
ofxNI2::Device * ofxTSPS::OpenNI2::getDevice(){
    if (device == NULL){
        ofLogError("OpenNI2") << "Cannot return device handle, device not initialized!";
        throw;
    }
    return device;
}

// Set the near clipping
void ofxTSPS::OpenNI2::setNearClipping( int near ){
    nearClipping = max(0,near);
}

// Set the far clipping
void ofxTSPS::OpenNI2::setFarClipping( int far ){
    farClipping = min( far, bIsOpen ? stream.getMaxPixelValue() : 10000 );
}


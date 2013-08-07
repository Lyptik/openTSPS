//
//  OpenNI2.cpp
//  openTSPS
//
//  Created by Andrew on 8/7/13.
//
//
#include "OpenNI2.h"
#include "ofxTSPS/source/Source.h"

// Constructor
ofxTSPS::OpenNI2::OpenNI2(){
    // type defaults to CAMERA_CUSTOM
    bCanTrackHaar = false;
    bDepthSetup = false;
    bDoProcessFrame = false;
    device = NULL;
    nearClipping = 0;
    farClipping = -1; // will be reset to max value
}

// Destructor
ofxTSPS::OpenNI2::~OpenNI2(){
    device->exit();
    delete device;
}

// Return availble device (tries to initialize one if we call this before init, which you shouldn't do)
bool ofxTSPS::OpenNI2::available(){
    if ( device == NULL ){
        device = new Ayb_openNI2Device;
        device->setup();
    }
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
    depthRemapToRange(ofxNI2::DepthStream::getPixelsRef(), retPix, nearClipping, farClipping, false);
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
    int N = src.getWidth() * src.getHeight();
    dst.allocate(src.getWidth(), src.getHeight(), 1);
    
    const unsigned short *src_ptr = src.getPixels();
    unsigned char *dst_ptr = dst.getPixels();
    
    //            float inv_range = 1. / (far - near);
    
    if (invert)
        std::swap(near, far);
    
    for (int i = 0; i < N; i++)
    {
        unsigned short C = *src_ptr;
        *dst_ptr = C == 0 ? 0 : ofMap(C, near, far, 0, 255, true);
        src_ptr++;
        dst_ptr++;
    }
}

// Open the hardware 
bool ofxTSPS::OpenNI2::openSource(int width, int height, string etc){
    
    // Initialize if device is null
    if ( device == NULL ){
        device = new Ayb_openNI2Device;
        device->setup();
    }
    
    // If initialization failed, the object exists
    
    //openni::Device thisDevice = device->get();
    
    //cout << this->device->device.m_device;
    //cout << this->device->get().m_device;
    
    
    if ( device == NULL){
         cout << "OpenNI2: Cannot locate suitable device - is hardware plugged in?";
        return false;
        
    // Init okay, make sure we attach device (do this once)
    }else{
        
        if ( !bDepthSetup ){
            bIsOpen = setup(*device);
            if ( farClipping == -1 ) farClipping = stream.getMaxPixelValue();
            setFps(30);
            bDepthSetup  = bIsOpen;
        } else {
            bIsOpen = true;
        }
        
        if (bIsOpen){start();}
        
        return bIsOpen;

    }
}

// Close the source
void ofxTSPS::OpenNI2::closeSource(){
    stream.stop();
    bIsOpen = false;
}

// Return the device handle (fail if it isn't initialized)
ofxTSPS::Ayb_openNI2Device * ofxTSPS::OpenNI2::getDevice(){
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


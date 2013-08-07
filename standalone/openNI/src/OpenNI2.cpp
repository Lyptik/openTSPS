//
//  OpenNI2.cpp
//  openTSPS
//
//  Created by Andrew on 8/7/13.
//
//
#include "OpenNI2.h"
#include "ofxTSPS/source/Source.h"


ofxTSPS::OpenNI2::OpenNI2(){
    // type defaults to CAMERA_CUSTOM
    bCanTrackHaar = false;
    bDepthSetup = false;
    bDoProcessFrame = false;
    device = NULL;
    nearClipping = 0;
    farClipping = -1; // will be reset to max value
}

ofxTSPS::OpenNI2::~OpenNI2(){
    device->exit();
    delete device;
}

// core
bool ofxTSPS::OpenNI2::available(){
    if ( device == NULL ){
        device = new ofxNI2::Device;
        device->setup();
    }
    return (device->listDevices() >= 1);
}

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

unsigned char * ofxTSPS::OpenNI2::getPixels(){
    return getPixelsRef().getPixels();
}

ofPixels & ofxTSPS::OpenNI2::getPixelsRef(){
    static ofPixels retPix;
    depthRemapToRange(ofxNI2::DepthStream::getPixelsRef(), retPix, nearClipping, farClipping, false);
    return retPix;
}

bool ofxTSPS::OpenNI2::doProcessFrame(){
    bool bReturn = bDoProcessFrame;
    if ( bDoProcessFrame ) bDoProcessFrame = false;
    return bReturn;
}

// fixed invert...
inline void ofxTSPS::OpenNI2::depthRemapToRange(const ofShortPixels &src, ofPixels &dst, int near, int far, int invert)
{
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

bool ofxTSPS::OpenNI2::openSource(int width, int height, string etc){
    // setup device?
    if ( device == NULL ){
        device = new ofxNI2::Device;
        device->setup();
    }
    
    // only try to attach device once
    if ( !bDepthSetup ){
        bIsOpen = setup(*device);
        if ( farClipping == -1 ) farClipping = stream.getMaxPixelValue();
        //                setSize(320, 240);
        setFps(30);
        bDepthSetup  = bIsOpen;
    } else {
        bIsOpen = true;
    }
    
    if (bIsOpen)
    {
        start();
    }
    return bIsOpen;
}

void ofxTSPS::OpenNI2::closeSource(){
    stream.stop();
    bIsOpen = false;
}

// Be careful, might be null!
ofxNI2::Device * ofxTSPS::OpenNI2::getDevice(){
    return device;
}

void ofxTSPS::OpenNI2::setNearClipping( int near ){
    nearClipping = max(0,near);
}

void ofxTSPS::OpenNI2::setFarClipping( int far ){
    farClipping = min( far, bIsOpen ? stream.getMaxPixelValue() : 10000 );
}


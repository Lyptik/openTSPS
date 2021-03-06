//
//  OpenNI2.h
//  openTSPS
//
//  Created by BRenfer on 7/19/13.
//
//  AYB: I refactored this into a cpp/h file so we could work with it
//  This class represents a TSPS source

#pragma once


#include "ofxTSPS/source/Source.h"
#include "ofxNI2.h"
#include "ofxNiTE2.h"

namespace ofxTSPS {
    class OpenNI2 : public Source, public ofxNI2::DepthStream {
    
    public:
        OpenNI2();
        ~OpenNI2();
        
        ofxTSPS::SourceType getType();

        
        ofxNiTE2::UserTracker userTracker;
        

        
        bool available();
        void update();
        
        unsigned char * getPixels();
        
        ofPixels & getPixelsRef();
        
        // ayb dav : added access to 13bits depth maps
        inline ofShortPixels& getDepthPixelsRef() {return ofxNI2::DepthStream::getPixelsRef();}

        bool doProcessFrame();
        
        // AYB: fixed what?
        // fixed invert...
        inline void depthRemapToRange(const ofShortPixels &src, ofPixels &dst, int near, int far, int invert);
        
        bool openSource( int width, int height, string etc="");
        void closeSource();
        
        // Be careful, might be null!
        ofxNI2::Device * getDevice();
        
        void setNearClipping( int near );
        void setFarClipping( int far );
        
        ofShortPixels currentDepthBufferRef;
        
    private:
        ofxNI2::Device *device;
        bool bDepthSetup, bDoProcessFrame;
        int nearClipping, farClipping;
    };
}
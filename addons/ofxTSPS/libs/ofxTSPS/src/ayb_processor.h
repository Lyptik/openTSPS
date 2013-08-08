//
//  ayb_processor.h
//  openTSPS
//
//  Created by David Chanel on 8/7/13.
//
//

#ifndef __openTSPS__ayb_processor__
#define __openTSPS__ayb_processor__

#include "ofxTSPS/Processor.h"
#include "ofxCv.h"

namespace ofxTSPS {
    class AYB_processor : public Processor {
    public:
        
        AYB_processor();
        
        void exit();
        void draw();
        
        // step 0: set camera image
        void setCameraImage( ofBaseImage & image );
        
        // step 1: background
        void captureBackground( ofBaseImage & image );
        ofPixelsRef progressiveBackground( ofBaseImage & image, float amount );
        void blankBackground();
        void autoDepthBackground(ofBaseImage & image);
                
        // step 2: difference
        ofPixelsRef difference( ofBaseImage & image, TrackingType trackingType );
        
        // step 3: process
        ofPixelsRef process ( ofBaseImage & image );
        void processOpticalFlow( ofBaseImage & image );
        void processHaar( ofBaseImage & image );
        
        // settings
        void setOpticalflowMinMax( float min = 0.0, float max = 10.0 );
        void setHaarXMLFile( string xmlFile );
        
        // methods: utils
        void resize( int camWidth, int camHeight );
        
    protected:
        // called automatically by setup();
        // use for custom setup of your processor
        void setupProcessor();
        
        // images
        ofImage cameraImage, cameraBabyImage, cameraSmallImage, backgroundImage, progressiveBackgroundImage, differencedImage;
        
        // util images
        ofImage resizeImage, resizeBabyImage;
        
        // pixel utils
        ofPixels blackPixelsSmall, resizedPixels;
        
        // ofxCv stuff
        ofxCv::ContourFinder        contourFinder;
        ofxCv::CascadeClassifier    haarFinder;
        vector<cv::Rect>            haarObjects;
        
        // opticalflow
        ofxCv::FlowFarneback    flow;
        bool bFlowTrackedOnce, bFirstImage;
    };
}

#endif /* defined(__openTSPS__ayb_processor__) */

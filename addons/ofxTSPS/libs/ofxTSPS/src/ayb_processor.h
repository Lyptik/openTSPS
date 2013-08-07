//
//  ayb_processor.h
//  openTSPS
//
//  Created by David Chanel on 8/7/13.
//
//

#ifndef __openTSPS__ayb_processor__
#define __openTSPS__ayb_processor__

#include <iostream>
#include "ofxTSPS/Processor.h"
#include "ofxCv.h"

namespace ofxTSPS {

class AYB_processor : public Processor
{
    public :
        AYB_processor();
        void exit();
        void draw();
    
        // step 0: set camera image
        void setCameraImage( ofBaseImage & image );
    
    
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
        
        bool bFirstImage;
};
}

#endif /* defined(__openTSPS__ayb_processor__) */

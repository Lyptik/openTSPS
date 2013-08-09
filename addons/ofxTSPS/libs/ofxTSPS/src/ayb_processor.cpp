//
//  ayb_processor.cpp
//  openTSPS
//
//  Created by David Chanel on 8/8/13.
//
//

#include <iostream>
#include "ayb_processor.h"

using namespace ofxCv;
using namespace cv;

namespace ofxTSPS {
    
    AYB_processor::AYB_processor()
    {
        bAllocateBackgroundFirstTime = true;

    }
    
    // Automatic depth background substraction (Works only for depth)
    ofPixelsRef AYB_processor::autoDepthBackground(ofShortPixels& depth, int near, int far, float margin){

        if(bAllocateBackgroundFirstTime) {
            m_background.allocate(depth.getWidth(), depth.getHeight(), OF_IMAGE_GRAYSCALE);
            subtract(backgroundImage, backgroundImage, backgroundImage); // blank out background
            bAllocateBackgroundFirstTime = false;
        }
        
        const unsigned short *src_ptr = depth.getPixels();
        unsigned short *pBackground = m_background.getPixels();
        
        for (int i=0 ; i<(depth.getWidth()*depth.getHeight()) ; i++){
            
            // upgrade current background with the furthest value (maxvalue)
            
            if(src_ptr[i] != 0) // No data (black) -> we skip
            {
                if(src_ptr[i] - pBackground[i] >  margin) // update only if point move more than “margin“ in mm
                    pBackground[i] = MAX(pBackground[i], src_ptr[i]);
            }
        }
        

        // Remapping background image values to 8 bits scale
        depthRemapToRange(m_background, backgroundImage.getPixelsRef(), near, far, false);

        // Copying background to the progressiveBackground buffer
        progressiveBackgroundImage.setFromPixels( backgroundImage.getPixelsRef());
        
        // Returning 8bits background
        return backgroundImage.getPixelsRef();
    }

    // Convert depthmap value to range
    void AYB_processor::depthRemapToRange(const ofShortPixels &src, ofPixels &dst, int near, int far, int invert){
        
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
    
    void AYB_processor::getQuadSubImageCropped(ofImage& inputImage, ofImage& outputImage, vector <ofPoint>& quad, ofImageType imageType) {
        
        outputImage = inputImage; // Is this optimized, does pass a reference or does it copy ? Is there a better way with a ROI maybe ?
        // TODO :
        //outputImage.crop(quad[0].x, quad[0].y, quad[1].x - quad[0].x, quad[3].y - quad[0].y);
    }
}
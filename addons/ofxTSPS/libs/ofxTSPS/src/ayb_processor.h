//
//  ayb_processor.h
//  openTSPS
//
//  Created by David Chanel on 8/8/13.
//
//

#ifndef __openTSPS__ayb_processor__
#define __openTSPS__ayb_processor__

#include "ofxTSPS/cv/CvProcessor.h"

namespace ofxTSPS {
    
    class AYB_processor : public CvProcessor
    {
        public :
        
            AYB_processor();
        
            ofPixelsRef autoDepthBackground(ofShortPixels& depth, int near, int far, float margin);
        
        private:
        
            ofImage testImage;
            ofShortImage m_background;

            void depthRemapToRange(const ofShortPixels &src, ofPixels &dst, int near, int far, int invert);
        
            bool bAllocateBackgroundFirstTime;
        
    };
    
}

#endif /* defined(__openTSPS__ayb_processor__) */

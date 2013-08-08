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
        
            void autoDepthBackground(ofBaseImage& image, ofShortPixels& depth);
        
        private:
        
        
    };
    
}

#endif /* defined(__openTSPS__ayb_processor__) */

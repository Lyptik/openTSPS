//
//  ayb_processor2.cpp
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
        
    }
    
    // Automatic depth background substraction (Works only for depth)
    void AYB_processor::autoDepthBackground(ofBaseImage & image){
        
        // TODO : Do the algo !
        
        cout << "foobar !" << endl;
        
        backgroundImage.setFromPixels( image.getPixelsRef() );
        progressiveBackgroundImage.setFromPixels( image.getPixelsRef() );
    }

}

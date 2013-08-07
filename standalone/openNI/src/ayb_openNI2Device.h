//
//  ayb_openNI2Device.h
//  openTSPS
//
//  Created by Andrew on 8/7/13.
//
//

#ifndef __openTSPS__ayb_openNI2Device__
#define __openTSPS__ayb_openNI2Device__

#include <iostream>
#include "ofxNI2.h"

namespace ofxTSPS {

    class Ayb_openNI2Device :  public  ofxNI2::Device{
        
    public:
        openni::Device openniDevice();
        
    };
    
}

#endif /* defined(__openTSPS__ayb_openNI2Device__) */

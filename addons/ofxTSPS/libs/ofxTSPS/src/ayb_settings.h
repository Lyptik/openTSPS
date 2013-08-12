//
//  ayb_settings.h
//  openTSPS
//
//  Created by Andrew on 8/6/13.
//
//

#ifndef __openTSPS__ayb_settings__
#define __openTSPS__ayb_settings__

#include <iostream>
#include "OpenNI2.h"

enum ayb_projectionType{
	PROJ_NONE=0x01,
	PROJ_TOP=0x02,
	PROJ_BOTTOM=0x03,
    PROJ_LEFT=0x04,
    PROJ_RIGHT=0x05
};

class AYB_settings {
    public:
        AYB_settings();
    
    // Is depth source?
    bool f_currentSourceIsDepthSource;
    
        // Clipping
        int clip_near;
        int clip_far;
        int clip_max_possible;
        int clip_min_possible;
    
        // Warping on/off
        bool f_warp;
    
        // Send default data
        bool f_muteTSPSOsc;
    
        // Send projection data
        bool f_sendProjectionData;
    
        // Processing options
        bool toggle_autoBg;
    
        // Ground detection
        bool f_detectGround;
        bool ground_detected;
        float ground_confidence;
        nite::Plane ground_plane;
    
    
        // Algo
        float algo1_slider;
        float algo2_slider;
    
        // Syphon
        bool syphon_on;
        float syphon_overlayAlpha;
        std::string syphon_overlaySource;
        std::string syphon_serverName;
    
        // Which projection to display
        ayb_projectionType projectionOption;
};

#endif /* defined(__openTSPS__ayb_settings__) */

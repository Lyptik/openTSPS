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
    
        // Clipping
        int clip_near;
        int clip_far;
        int clip_max_possible;
        int clip_min_possible;
    
        // Processing options
        bool toggle_autoBg;
        bool f_detectGround;
    
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

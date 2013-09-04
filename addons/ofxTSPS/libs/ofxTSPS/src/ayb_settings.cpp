//
//  ayb_settings.cpp
//  openTSPS
//
//  Created by Andrew on 8/6/13.
//
//

#include "ayb_settings.h"

// Constructor
AYB_settings::AYB_settings(){

    f_currentSourceIsDepthSource=false;
    
    // Clipping settings
    clip_max_possible=10500;
    clip_min_possible=1;
    clip_near=clip_min_possible;
    clip_far=clip_max_possible;
    
    f_warp = false;
    
    // Processing options
    toggle_autoBg = false;
    
    // Ground detection
    f_detectGround=false;
    ground_detected = false;
    ground_confidence=0;
    
    // Algo
    algo1_slider = 0;
    algo2_slider = 0;
    
    // Syphon
    syphon_on=false;
    syphon_overlayAlpha =0.0;
    syphon_overlaySource = "";
    syphon_serverName = "";
    
    // Which projection to display
    projectionOption = PROJ_NONE;

    f_muteTSPSOsc=false;
    f_sendProjectionData=false;
    
}
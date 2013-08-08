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

    // In general access these like this:
    // settings.ayb_settings.VALUE
    
    // Clipping settings
    clip_max_possible=4000;
    clip_min_possible=1;
    clip_near=clip_min_possible;
    clip_far=clip_max_possible;
    
    
    // Processing options
    toggle_autoBg = false;
    f_detectGround=false;
    
    
    // Syphon
    syphon_on=false;
    syphon_overlayAlpha =0.0;
    syphon_overlaySource = "";
    syphon_serverName = "";
    
    // Which projection to display
    projectionOption = PROJ_NONE;

    
}
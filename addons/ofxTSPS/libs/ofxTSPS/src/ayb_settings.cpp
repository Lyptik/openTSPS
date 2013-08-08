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

    // Clipping settings
    clip_max_possible=4000;
    clip_min_possible=1;
    clip_near=clip_min_possible;
    clip_far=clip_max_possible;
    toggle_autoBg = false;
}
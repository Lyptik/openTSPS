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



class AYB_settings {
    public:
        AYB_settings();
        int clip_near;
        int clip_far;
        bool toggle_autoBg;
        int clip_max_possible;
        int clip_min_possible;
};

#endif /* defined(__openTSPS__ayb_settings__) */

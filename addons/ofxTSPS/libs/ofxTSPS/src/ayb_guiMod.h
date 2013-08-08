//
//  augmentation.h
//  openTSPS
//
//  Created by Andrew on 8/5/13.
//
//

#ifndef __openTSPS__augmentation__
#define __openTSPS__augmentation__

#include <iostream>
#include "ofxLabGui.h"
#include "ofxTSPS/gui/Settings.h"


class AYB_guiMod{
    public:
    
        // Constructor
        AYB_guiMod();
    
        // Inject changes to the GUI (called from setup)
        void injectGUI(ofxLabGui& panel,
                       map<string, vector<guiTypePanel *> >& panelGroups,
                       vector<string>& source_types,
                       ofxTSPS::Settings &settings);

        // Process the GUI based on our changes (called from update)
        void processGUIUpdates(ofxLabGui &panel,
                           ofxTSPS::Settings &settings);
};


#endif /* defined(__openTSPS__augmentation__) */
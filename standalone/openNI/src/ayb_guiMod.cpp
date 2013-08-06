//
//  augmentation.cpp
//  openTSPS
//
//  Created by Andrew on 8/5/13.
//
//

#include "ayb_guiMod.h"
#include "ofxTSPS/gui/GuiManager.h"



AYB_guiMod::AYB_guiMod(){
    //Constructor
}




// Injects some changes to the GUI
void AYB_guiMod::injectGUI(ofxLabGui& panel,
                           map<string, vector<guiTypePanel *> >& panelGroups,
                           vector<string>& source_types){
    
    
    
    // Some options should only appear if we have a depth source
    // Currently we're not enabling this because "depth source" could be a syphon, for example
    bool depthSource=true;
    if (depthSource){
        
        // SOURCE:VIDEO tab
        ///////////////////////////////////////////////////////////////
        panel.setWhichPanel("video");
        guiTypeGroup * clippingGroup = panel.addGroup("clipping");
        clippingGroup->setBackgroundColor(148,129,85);
        clippingGroup->setBackgroundSelectColor(148,129,85);
        clippingGroup->seBaseColor(244,136,136);
        clippingGroup->setShowText(false);
        panel.addToggle("Depth Clipping", "AYB_DEPTHCLIP_APPLY", false);
        panel.addSlider("Near:", "AYB_DEPTHCLIP_NEAR", 1, 1, 100, true);
        panel.addSlider("Far:", "AYB_DEPTHCLIP_FAR", 1, 1, 100, true);
    
    
        // SENSING:BACKGROUND Tab
        ///////////////////////////////////////////////////////////////
        panel.setWhichPanel("background");
        guiTypeGroup * autoGroup = panel.addGroup("Automatic Background Subtraction");
        autoGroup->setBackgroundColor(148,129,85);
        autoGroup->setBackgroundSelectColor(248,129,85);
        autoGroup->seBaseColor(180,87,128);
        autoGroup->setShowText(false);
        panel.addToggle("AYB Background Subtraction", "AYB_BGSUB_APPLY", false);
    
        
    
        // SENSING:DEPTH Tab
        ///////////////////////////////////////////////////////////////
        // Add new tab
        guiTypePanel * depthPanel = panel.addPanel("depth", 1, false);
        depthPanel->setDrawLock( false );
        depthPanel->setBackgroundColor(123,191,174);
        depthPanel->setBackgroundSelectColor(123,191,174);
        
        // Add options 
        panel.setWhichPanel("depth");
        guiTypeGroup * autoGroup2 = panel.addGroup("Detect Ground");
        autoGroup2->setBackgroundColor(148,129,85);
        autoGroup2->setBackgroundSelectColor(248,129,85);
        autoGroup2->seBaseColor(180,87,128);
        autoGroup2->setShowText(false);
        panel.addToggle("Detect Ground", "AYB_DETECT_GROUND", false);
        
        // Projection selection
        guiTypeGroup * projectionGroup = panel.addGroup("Projection View");
        projectionGroup->setBackgroundColor(148,129,85);
        projectionGroup->setBackgroundSelectColor(248,129,85);
        projectionGroup->seBaseColor(180,87,128);
        projectionGroup->setShowText(false);
        vector<string> multi;
        multi.push_back("top");
        multi.push_back("bottom");
        multi.push_back("left");
        multi.push_back("right");
        panel.addMultiToggle("Projection:", "AYB_PROJECTION_OPTION", 0, multi);

        
        // Algorithm 1
        guiTypeGroup * algo1Group = panel.addGroup("Algorithm 1");
        algo1Group->setBackgroundColor(148,129,85);
        algo1Group->setBackgroundSelectColor(248,129,85);
        algo1Group->seBaseColor(180,87,128);
        algo1Group->setShowText(false);
        panel.addToggle("Algorithm 1", "AYB_ALGO1_APPLY", false);
        panel.addSlider("Option:", "AYB_ALGO1_OPTION", 1, 1, 100, true);
        
        // Algorithm 2
        guiTypeGroup * algo2Group = panel.addGroup("Algorithm 2");
        algo2Group->setBackgroundColor(148,129,85);
        algo2Group->setBackgroundSelectColor(248,129,85);
        algo2Group->seBaseColor(180,87,128);
        algo2Group->setShowText(false);
        panel.addToggle("Algorithm 2", "AYB_ALGO2_APPLY", false);
        panel.addSlider("Option:", "AYB_ALGO2_OPTION", 1, 1, 100, true);

        
        
        // Make sure the depth tab is in third place
        std::vector<guiTypePanel *>::iterator it;
        it = panelGroups["sensing"].begin();
        it+=2;
        panelGroups["sensing"].insert(it, depthPanel);
        
    }
    
 
    
    // COMMUNICATION:SYPHON
    ///////////////////////////////////////////////////////////////
    // Add new tab
    guiTypePanel * syphonPanel = panel.addPanel("Syphon", 1, false);
    syphonPanel->setDrawLock( false );
    syphonPanel->setBackgroundColor(180,87,128);
    syphonPanel->setBackgroundSelectColor(180,87,128);
    panelGroups["communication"].push_back( syphonPanel );
    
    // Populate SYPHON tab
    panel.setWhichPanel("Syphon");
    panel.addToggle("Syphon ON/OFF", "AYB_SYPHON_ON", false);
    panel.addSlider("Overlay Alpha:", "AYB_SYPHON_ALPHA", 1, 1, 100, true);
    panel.addTextField("Syphon overlay:", "AYB_SYPHON_OVERLAYSOURCE", "syphon_1", 200, 20);
    panel.addTextField("Syphon send as:", "AYB_SYPHON_SERVERNAME", "syphon_2", 200, 20);
    source_types.push_back("Syphon");
    
    
 }

// Handle GUI updates (mutually exclusive options, ect)
/*

 AYB_DEPTHCLIP_APPLY
 AYB_DEPTHCLIP_NEAR
 AYB_DEPTHCLIP_FAR
 
 AYB_PROJECTION_OPTION
 
 AYB_DETECT_GROUND
 
 AYB_ALGO1_APPLY
 AYB_ALGO1_OPTION

 AYB_ALGO2_APPLY
 AYB_ALGO2_OPTION
 
 AYB_SYPHON_ON
 AYB_SYPHON_ALPHA
 AYB_SYPHON_OVERLAYSOURCE
 AYB_SYPHON_SERVERNAME

*/
void AYB_guiMod::processGUIUpdates(ofxLabGui &panel, ofxTSPS::Settings &settings){
    
    
    // BUTTON: Detect Ground
    if(panel.getValueB("AYB_DETECT_GROUND")){
        printf("*DETECT GROUND*");
        panel.setValueB("AYB_DETECT_GROUND", false);
    }
    
    
    // OPTION: DEPTH CLIPPING
    // Make sure depth clipping is in reasonable range
    if (panel.getValueF("AYB_DEPTHCLIP_FAR")< panel.getValueF("AYB_DEPTHCLIP_NEAR") ){
        panel.setValueF("AYB_DEPTHCLIP_FAR", (panel.getValueF("AYB_DEPTHCLIP_NEAR")+1.0));
    }
    // Apply Depth clipping option
    if (panel.getValueB("AYB_DEPTHCLIP_APPLY")){
        //settings.clipNear=
    }
    
 
    // OPTION: BACKGROUND SUBTRACT
    if (panel.getValueF("AYB_BGSUB_APPLY")){
        // Disable TSPS background recapture if our BG sub is on
        panel.setValueF("RELEARN", false);
    }
    
    
    
    
}

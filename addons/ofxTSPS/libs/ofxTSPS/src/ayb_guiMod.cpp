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
    
    // Prints out some debugging info about compiler defaults
    // Can be removed
#ifdef _LIBCPP_VERSION
    std::cout << "Using libc++\n";
#else
    std::cout << "Using libstdc++\n";
#endif
#ifdef __GXX_EXPERIMENTAL_CXX0X__
#if __cplusplus == 1
    std::cout << "Language mode = gnu++11\n";
#else
    std::cout << "Language mode = c++11\n";
#endif
#else
#if __cplusplus == 1
    std::cout << "Language mode = gnu++98\n";
#else
    std::cout << "Language mode = c++98\n";
#endif
#endif
}




// Injects some changes to the GUI
void AYB_guiMod::injectGUI(ofxLabGui& panel,
                           map<string, vector<guiTypePanel *> >& panelGroups,
                           vector<string>& source_types,
                           ofxTSPS::Settings &settings){
    
    
    // INPUT: ADJUSTMENT DEPTH
    ////////////////////////////////////////////////
    panel.setWhichPanel("adjustment:depth");
    
    // Clipping
    guiTypeGroup * clippingGroup = panel.addGroup("clipping");
    clippingGroup->setBackgroundColor(148,129,85);
    clippingGroup->setBackgroundSelectColor(148,129,85);
    clippingGroup->seBaseColor(244,136,136);
    clippingGroup->setShowText(false);
    panel.addToggle("Depth Clipping", "AYB_DEPTHCLIP_APPLY", false);
    panel.addSlider("Near:", "AYB_DEPTHCLIP_NEAR", settings.ayb_Settings.clip_min_possible, settings.ayb_Settings.clip_min_possible, settings.ayb_Settings.clip_max_possible, true);
    panel.addSlider("Far:", "AYB_DEPTHCLIP_FAR", settings.ayb_Settings.clip_max_possible, settings.ayb_Settings.clip_min_possible, settings.ayb_Settings.clip_max_possible, true);
    
    // Warp
    guiTypeGroup * warpGroup = panel.addGroup("warp");
    warpGroup->setBackgroundColor(148,129,85);
    warpGroup->setBackgroundSelectColor(148,129,85);
    warpGroup->seBaseColor(244,136,136);
    warpGroup->setShowText(false);
    panel.addToggle("Warp", "AYB_WARP_APPLY", false);
    
    // Ground detection button
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
    multi.push_back("none");
    multi.push_back("top");
    multi.push_back("bottom");
    multi.push_back("left");
    multi.push_back("right");
    panel.addMultiToggle("Projection:", "AYB_PROJECTION_OPTION", 0, multi);
        
    // SENSING:BACKGROUND Tab
    ///////////////////////////////////////////////////////////////
    panel.setWhichPanel("background");
    guiTypeGroup * autoGroup = panel.addGroup("Automatic Background Subtraction");
    autoGroup->setBackgroundColor(148,129,85);
    autoGroup->setBackgroundSelectColor(248,129,85);
    autoGroup->seBaseColor(180,87,128);
    autoGroup->setShowText(false);
    panel.addToggle("AYB Background Subtraction", "AYB_BGSUB_APPLY", false);
    
    // SENSING:PROCESSING
    ///////////////////////////////////////////////////////////////
    panel.setWhichPanel("processing");
    
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
    
    
    
    
    // COMMUNICATION:SYPHON
    ///////////////////////////////////////////////////////////////
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

    
    // DATA:Projection
    ///////////////////////////////////////////////////////////////
    guiTypePanel * projectionPanel = panel.addPanel("AYB:Data", 1, false);
    projectionPanel->setDrawLock( false );
    projectionPanel->setBackgroundColor(180,87,128);
    projectionPanel->setBackgroundSelectColor(180,87,128);
    panelGroups["data"].push_back( projectionPanel );
    panel.setWhichPanel("AYB:Data");
    
    
    guiTypeGroup * projGroup1 = panel.addGroup("Group1");
    projGroup1->setBackgroundColor(148,129,85);
    projGroup1->setBackgroundSelectColor(248,129,85);
    projGroup1->seBaseColor(180,87,128);
    projGroup1->setShowText(false);
    panel.addToggle("OSC: Mute TSPS", "AYB_MUTE_TSPSOSC", false);
    
    guiTypeGroup * projGroup2 = panel.addGroup("Group1");
    projGroup2->setBackgroundColor(148,129,85);
    projGroup2->setBackgroundSelectColor(248,129,85);
    projGroup2->seBaseColor(180,87,128);
    projGroup2->setShowText(false);
    panel.addToggle("OSC: Send projection data", "AYB_SEND_PROJECTION", false);
    
    // Is current source a depthsource? Default false
    settings.ayb_Settings.f_currentSourceIsDepthSource=false;

    
}


// Handle GUI updates (mutually exclusive options, ect)
// This is really a manual binding to settings, which is what we want to access
// (see ayb_Settings.cpp for the values you want to use)
void AYB_guiMod::processGUIUpdates(ofxLabGui &panel, ofxTSPS::Settings &settings){
    
    
    /*
    
     Disabled for now, this isn't how the GUI works...
     
    // Disable non-depth options unless depthsource
    if(settings.ayb_Settings.f_currentSourceIsDepthSource){
        panel.getElement("adjustment:depth")->enable();
        panel.getElement("AYB:Data")->enable();
        panel.getElement("processing")->enable();
    }else{
        panel.getElement("adjustment:depth")->disable();
        panel.getElement("AYB:Data")->disable();
        panel.getElement("processing")->disable();
    }
    */
    
    
    // MOMENTARY: Detect Ground
    if(panel.getValueB("AYB_DETECT_GROUND")){
        // Elsewhere, this flag gets picked up and consumed
        settings.ayb_Settings.f_detectGround=true;
        panel.setValueB("AYB_DETECT_GROUND", false);
    }
    
    
    // ON/OFF: Just pass these through
    settings.ayb_Settings.toggle_autoBg = panel.getValueB("AYB_BGSUB_APPLY");
    settings.ayb_Settings.syphon_on = panel.getValueB("AYB_SYPHON_ON");
    settings.ayb_Settings.f_muteTSPSOsc = panel.getValueB("AYB_MUTE_TSPSOSC");
    settings.ayb_Settings.f_sendProjectionData = panel.getValueB("AYB_SEND_PROJECTION");
    
    
    // ALGO 1 / ALGO 2: These are placeholders and should be replaced with real names before binding
    
    // Syphon
    settings.ayb_Settings.syphon_on=panel.getValueB("AYB_SYPHON_ON");
    settings.ayb_Settings.syphon_overlayAlpha=panel.getValueF("AYB_SYPHON_ALPHA");
    settings.ayb_Settings.syphon_overlaySource=panel.getValueS("AYB_SYPHON_OVERLAYSOURCE");
    settings.ayb_Settings.syphon_serverName=panel.getValueS("AYB_SYPHON_SERVERNAME");
    
    
    //Projection option (see ayb_settings for enum type)
    settings.ayb_Settings.projectionOption=(ayb_projectionType)panel.getValueI("AYB_PROJECTION_OPTION");
    
    
    
    // OPTION: DEPTH CLIPPING
    // Make sure depth clipping is in reasonable range
    if (panel.getValueI("AYB_DEPTHCLIP_FAR")<= 0 || panel.getValueI("AYB_DEPTHCLIP_NEAR") <= 0 ){
        panel.setValueI("AYB_DEPTHCLIP_NEAR", settings.ayb_Settings.clip_min_possible);
        panel.setValueI("AYB_DEPTHCLIP_FAR", settings.ayb_Settings.clip_max_possible);
        
    }
    if (panel.getValueI("AYB_DEPTHCLIP_FAR")<= panel.getValueI("AYB_DEPTHCLIP_NEAR") ){
        panel.setValueI("AYB_DEPTHCLIP_FAR", (panel.getValueI("AYB_DEPTHCLIP_NEAR")+1.0));
    }
    
    if (panel.getValueI("AYB_DEPTHCLIP_NEAR")>= panel.getValueI("AYB_DEPTHCLIP_FAR") ){
        panel.setValueI("AYB_DEPTHCLIP_NEAR", (panel.getValueI("AYB_DEPTHCLIP_NEAR")-1.0));
    }
    
    
    // Apply Depth clipping option
    if (!panel.getValueB("AYB_DEPTHCLIP_APPLY")){
        //panel.setValueI("AYB_DEPTHCLIP_NEAR", settings.ayb_Settings.clip_min_possible);
        //panel.setValueI("AYB_DEPTHCLIP_FAR", settings.ayb_Settings.clip_max_possible);
        settings.ayb_Settings.clip_near=settings.ayb_Settings.clip_min_possible;
        settings.ayb_Settings.clip_far=settings.ayb_Settings.clip_max_possible;
    }else{
        settings.ayb_Settings.clip_near=panel.getValueI("AYB_DEPTHCLIP_NEAR");
        settings.ayb_Settings.clip_far=panel.getValueI("AYB_DEPTHCLIP_FAR");
    }
    
    // OPTION: BACKGROUND SUBTRACT
    if (panel.getValueF("AYB_BGSUB_APPLY")){
        // Disable TSPS background recapture if our BG sub is on
        //panel.setValueF("RELEARN", false);
    }
    
    settings.ayb_Settings.f_warp=(ayb_projectionType)panel.getValueB("AYB_WARP_APPLY");
    
}

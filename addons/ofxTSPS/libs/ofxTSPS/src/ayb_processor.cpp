//
//  ayb_processor.cpp
//  openTSPS
//
//  Created by David Chanel on 8/7/13.
//
//

#include "ayb_processor.h"
#include "ofxTSPS/cv/CvPerson.h"

using namespace ofxCv;
using namespace cv;

bool bOnce = false;

namespace ofxTSPS {
    //------------------------------------------------------------------------
    AYB_processor::AYB_processor(){
        // capabilities
        bCanTrackHaar       = true;
        bCanTrackContours   = true;
        bCanTrackSkeleton   = false;
        bCanTrackOpticalFlow = true;
        
        // default settings
        bTrackHaar          = false;
        bTrackContours      = true;
        bTrackSkeleton      = false;
        bTrackOpticalFlow   = false;
        bFlowTrackedOnce    = false;
        
        setThreshold();
        setBlobSettings();
        setOpticalflowMinMax();
        setHaarPadding();
    }
    
    //------------------------------------------------------------------------
    void AYB_processor::setupProcessor(){
        resize(tspsWidth, tspsHeight);
        
        //setup contour finder
        contourFinder.setThreshold(15);
        contourFinder.getTracker().setPersistence(5);
        contourFinder.setAutoThreshold( false );
		contourFinder.getTracker().setMaximumDistance(300);
		contourFinder.setSimplify(false);
        
        //setup optical flow
        flow.setPyramidScale( .1 );
        flow.setNumLevels( 1 );
        flow.setWindowSize( 8 );
        flow.setNumIterations( 1 );
        flow.setPolyN( 5 );
        flow.setPolySigma( 1.1 );
        flow.setUseGaussian( false );
    }
    
    //------------------------------------------------------------------------
    void AYB_processor::exit(){}
    
    //------------------------------------------------------------------------
    void AYB_processor::draw(){
        if ( bTrackOpticalFlow && bFlowTrackedOnce ){
            flow.draw(0,0,tspsWidth, tspsHeight);
        }
    }
    
    //------------------------------------------------------------------------
    void AYB_processor::setCameraImage( ofBaseImage & image ){
        // update camera image
        cameraImage.setFromPixels(image.getPixelsRef());
        
        // update smaller image
        // pixel copy method is temporary... copying directly to image
        // via ofxCv crashes
		Mat srcMat = toCv(cameraImage), dstMat = toCv(resizeImage);
        cv::resize(srcMat, dstMat, dstMat.size(), 0, 0, INTER_NEAREST);
        toOf(dstMat, cameraSmallImage);
        cameraSmallImage.update();
        
		Mat dstMatBaby = toCv(resizeBabyImage);
        cv::resize(srcMat, dstMatBaby, dstMatBaby.size(), 0, 0, INTER_NEAREST);
        toOf(dstMatBaby, cameraBabyImage);
        cameraBabyImage.update();
    }
    
    //------------------------------------------------------------------------
    void AYB_processor::captureBackground( ofBaseImage & image ){
        backgroundImage.setFromPixels( image.getPixelsRef() );
        progressiveBackgroundImage.setFromPixels( image.getPixelsRef() );
    }
    
    //------------------------------------------------------------------------
    ofPixelsRef AYB_processor::progressiveBackground( ofBaseImage & image, float amount ){
        ofxCv::lerp(image, progressiveBackgroundImage, progressiveBackgroundImage, amount);
        //cv::addWeighted( toCv(backgroundImage), amount, toCv(progressiveBackgroundImage), 1.0f-amount,0, toCv(progressiveBackgroundImage) );
        backgroundImage = progressiveBackgroundImage;
        return backgroundImage.getPixelsRef();
    }
    
    //------------------------------------------------------------------------
    void AYB_processor::blankBackground(){
        subtract(backgroundImage, backgroundImage, backgroundImage);
    }

    //------------------------------------------------------------------------
    // Automatic depth background substraction (Works only for depth)
    void AYB_processor::autoDepthBackground(ofBaseImage & image){
        
        // TODO : Do the algo !
        
        cout << "foobar !" << endl;
        
        backgroundImage.setFromPixels( image.getPixelsRef() );
        progressiveBackgroundImage.setFromPixels( image.getPixelsRef() );
    }

    
    //------------------------------------------------------------------------
    ofPixelsRef AYB_processor::difference( ofBaseImage & image, TrackingType trackingType ){
        if(trackingType == TRACK_ABSOLUTE){
            absdiff( backgroundImage, (ofImage&) image, differencedImage);
        } else {
            if(trackingType == TRACK_DARK){
                subtract( (ofImage&) image, backgroundImage, differencedImage);
            } else if(trackingType == TRACK_LIGHT){
                subtract ( backgroundImage, (ofImage&) image, differencedImage);
            }
        }
        differencedImage.update();
        return differencedImage.getPixelsRef();
    }
    
    //------------------------------------------------------------------------
    ofPixelsRef AYB_processor::process ( ofBaseImage & image ){
        if ( bTrackHaar ){
            processHaar( cameraBabyImage );
        }
        
        if ( bTrackOpticalFlow ){
            processOpticalFlow( cameraSmallImage );
        }
        
        differencedImage.setFromPixels(image.getPixelsRef());
        ofxCv::threshold(differencedImage, threshold);
        
        // find contours
        contourFinder.setFindHoles( bFindHoles );
        contourFinder.setMinArea( minBlobArea * tspsWidth * tspsHeight );
        contourFinder.setMaxArea( maxBlobArea * tspsWidth * tspsHeight );
        contourFinder.findContours( differencedImage );
        
        // update people
        RectTracker& rectTracker    = contourFinder.getTracker();
        cv::Mat cameraMat           = toCv(cameraImage);
        
        //optical flow scale
        // float flowROIScale = tspsWidth/flow.getWidth();
        
        for(int i = 0; i < contourFinder.size(); i++){
            unsigned int id = contourFinder.getLabel(i);
            if(rectTracker.existsPrevious(id)) {
                CvPerson* p = (CvPerson *) getTrackedPerson(id);
                //somehow we are not tracking this person, safeguard (shouldn't happen)
                if(NULL == p){
                    ofLog(OF_LOG_WARNING, "Person::warning. encountered persistent blob without a person behind them\n");
                    continue;
                }
                p->oid = i; //hack ;(
                
                //update this person with new blob info
                // to-do: make centroid dampening dynamic
                p->update(true);
                
                
                //normalize simple contour
                for (int i=0; i<p->simpleContour.size(); i++){
                    p->simpleContour[i].x /= tspsWidth;
                    p->simpleContour[i].y /= tspsHeight;
                }
                
                //find peak in blob (only useful with depth cameras)
                cv::Point minLoc, maxLoc;
                double minVal = 0, maxVal = 0;
                cv::Rect rect;
                rect.x      = p->boundingRect.x;
                rect.y      = p->boundingRect.y;
                rect.width  = p->boundingRect.width;
                rect.height = p->boundingRect.height;
                cv::Mat roiMat(cameraMat, rect);
                cv::minMaxLoc( roiMat, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());
                
                // set depth
                p->depth = p->highest.z / 255.0f;
                
                // set highest and lowest points: x, y, VALUE stored in .z prop
                // ease vals unless first time you're setting them
                if ( p->highest.x == -1 ){
                    p->highest.set(  p->boundingRect.x + maxLoc.x,  p->boundingRect.y + maxLoc.y, maxVal);
                    p->lowest.set(  p->boundingRect.x + minLoc.x,  p->boundingRect.y + minLoc.y, minVal);
                } else {
                    p->highest.x = ( p->highest.x * .9 ) + ( p->boundingRect.x + maxLoc.x ) * .1;
                    p->highest.y = ( p->highest.y * .9 ) + ( p->boundingRect.y + maxLoc.y ) * .1;
                    p->highest.z = ( p->highest.z * .9) + ( maxVal ) * .1;
                    p->lowest.x = ( p->lowest.x * .9 ) + ( p->boundingRect.x + minLoc.x ) * .1;
                    p->lowest.y = ( p->lowest.y * .9 ) + ( p->boundingRect.y + minLoc.y ) * .1;
                    p->lowest.z = ( p->lowest.z * .9) + ( minVal ) * .1;
                }
                
                // cap highest + lowest
                p->highest.x = (p->highest.x > tspsWidth ? tspsWidth : p->highest.x);
                p->highest.x = (p->highest.x < 0 ? 0 : p->highest.x);
                p->highest.y = (p->highest.y > tspsHeight ? tspsHeight : p->highest.y);
                p->highest.y = (p->highest.y < 0 ? 0 : p->highest.y);
                
                p->lowest.x = (p->lowest.x > tspsWidth ? tspsWidth : p->lowest.x);
                p->lowest.x = (p->lowest.x < 0 ? 0 : p->highest.x);
                p->lowest.y = (p->lowest.y > tspsHeight ? tspsHeight : p->lowest.y);
                p->lowest.y = (p->lowest.y < 0 ? 0 : p->highest.y);
                
                // ROI for opticalflow
                ofRectangle roi = p->getBoundingRectNormalized(tspsWidth, tspsHeight);
                roi.x *= flow.getWidth();
                roi.y *= flow.getHeight();
                roi.width *= flow.getWidth();
                roi.height *= flow.getHeight();
                
                // sum optical flow for the person
                if ( bTrackOpticalFlow && bFlowTrackedOnce ){
                    // TO-DO!
                    p->opticalFlowVectorAccumulation = flow.getAverageFlowInRegion(roi);
                } else {
                    p->opticalFlowVectorAccumulation.x = p->opticalFlowVectorAccumulation.y = 0;
                }
                
                //detect haar patterns (faces, eyes, etc)
                if ( bTrackHaar ){
                    //find the region of interest, expanded by haarArea.
                    ofRectangle haarROI;
                    haarROI.x		= (p->boundingRect.x - haarAreaPadding/2) * haarTrackingScale > 0.0f ? (p->boundingRect.x - haarAreaPadding/2) * haarTrackingScale : 0.0;
                    haarROI.y		= (p->boundingRect.y - haarAreaPadding/2) * haarTrackingScale > 0.0f ? (p->boundingRect.y - haarAreaPadding/2) : 0.0f;
                    haarROI.width	= (p->boundingRect.width  + haarAreaPadding*2) * haarTrackingScale > cameraBabyImage.width ? (p->boundingRect.width  + haarAreaPadding*2) * haarTrackingScale : cameraBabyImage.width;
                    haarROI.height	= (p->boundingRect.height + haarAreaPadding*2) * haarTrackingScale > cameraBabyImage.height ? (p->boundingRect.height + haarAreaPadding*2) * haarTrackingScale : cameraBabyImage.height;
                    
                    bool haarThisFrame = false;
                    for(int j = 0; j < haarObjects.size(); j++) {
                        ofRectangle hr = toOf(haarObjects[j]);
                        
                        //check to see if the haar is contained within the bounding rectangle
                        if(hr.x > haarROI.x && hr.y > haarROI.y && hr.x+hr.width < haarROI.x+haarROI.width && hr.y+hr.height < haarROI.y+haarROI.height){
                            hr.x /= haarTrackingScale;
                            hr.y /= haarTrackingScale;
                            hr.width /= haarTrackingScale;
                            hr.height /= haarTrackingScale;
                            p->setHaarRect(hr);
                            haarThisFrame = true;
                            break;
                        }
                    }
                    if(!haarThisFrame){
                        p->noHaarThisFrame();
                    }
                }
                personUpdated(p, scene);
            } else {
                ofPoint centroid = toOf(contourFinder.getCentroid(i));
                CvPerson* newPerson = new CvPerson(id, i, contourFinder);
                personEntered(newPerson, scene);
            }
        }
        
        //reset scene
        if ( bTrackOpticalFlow && bFlowTrackedOnce ){
            scene->averageMotion = flow.getAverageFlow();
        } else {
            scene->averageMotion = ofPoint(0,0);
        }
        scene->update( trackedPeople, tspsWidth, tspsHeight );
        
        // delete old blobs
        for (int i=trackedPeople->size()-1; i>=0; i--){
            Person* p = (*trackedPeople)[i];
            EventArgs args;
            args.person = p;
            args.scene  = scene;
            
            if (p == NULL){
                personWillLeave(p, scene);
                trackedPeople->erase(trackedPeople->begin() + i);
            } else if ( !(rectTracker.existsPrevious( p->pid ) && rectTracker.existsCurrent(p->pid)) && !rectTracker.existsCurrent(p->pid) ){
                personWillLeave(p, scene);
                trackedPeople->erase(trackedPeople->begin() + i);
            }
        }
        return differencedImage.getPixelsRef();
    }
    
    //------------------------------------------------------------------------
    void AYB_processor::processOpticalFlow( ofBaseImage & image ){
        flow.calcOpticalFlow(image);
        bFlowTrackedOnce = true;
    }
    
    //------------------------------------------------------------------------
    void AYB_processor::processHaar( ofBaseImage & image ){
        // don't really need the image here, huh?
		//Mat graySmallMat = toCv(image);
        //equalizeHist(graySmallMat, graySmallMat);
		haarFinder.detectMultiScale(toCv(image), haarObjects, 1.06, 1,
                                    //CascadeClassifier::DO_CANNY_PRUNING |
                                    //CascadeClassifier::FIND_BIGGEST_OBJECT |
                                    //CascadeClassifier::DO_ROUGH_SEARCH |
                                    0);
    }
    
    //------------------------------------------------------------------------
    void AYB_processor::setOpticalflowMinMax( float min, float max ){
        // ...
    }
    
    //------------------------------------------------------------------------
    void AYB_processor::setHaarXMLFile( string xmlFile ){
        haarFinder.load(ofToDataPath(xmlFile));
    }
    
    //------------------------------------------------------------------------
    void AYB_processor::resize( int camWidth, int camHeight ){
        cameraImage.allocate(camWidth, camHeight, OF_IMAGE_GRAYSCALE);
        cameraBabyImage.allocate((int) camWidth * haarTrackingScale, (int) camHeight * haarTrackingScale, OF_IMAGE_GRAYSCALE);
        cameraSmallImage.allocate((int) camWidth * trackingScale, (int) camHeight * trackingScale, OF_IMAGE_GRAYSCALE);
        resizeImage.allocate((int) camWidth * trackingScale, (int) camHeight * trackingScale, OF_IMAGE_GRAYSCALE);
        resizeBabyImage.allocate((int) camWidth * haarTrackingScale, (int) camHeight * haarTrackingScale, OF_IMAGE_GRAYSCALE);
        
        backgroundImage.allocate(camWidth, camHeight, OF_IMAGE_GRAYSCALE);
        differencedImage.allocate(camWidth, camHeight, OF_IMAGE_GRAYSCALE);
        progressiveBackgroundImage.allocate(camWidth, camHeight, OF_IMAGE_GRAYSCALE);
    }
}

#include "testApp.h"

#include <cv.h>

#define W 640
#define H 480

using namespace cv;


ofPixels pix;
vector<vector<cv::Point> > contours;
ofFbo fbo;
float t0, t1, t2;


//--------------------------------------------------------------
void testApp::setup(){
    
    loadSettings();
    vidGrabber.setVerbose(true);
    vidGrabber.initGrabber(W,H);
    
    grayImage.allocate(W, H);
	grayAcc.allocate(W, H);
    backImg.allocate(W, H);
	grayBg.allocate(W,H);
	grayDiff.allocate(W,H);
    
  	individualTextureSyphonServer.setName("Shoot");
    mClient.setup();
    mClient.setApplicationName("MadMapper");
    
    
    //ofSetFrameRate(30);
    
    for(int i = 0; i < 255; i++)
        keys[i] = false;
    
    sender.setup("localhost", 9001);

    
    
//    recordContext.setup();
//	recordImage.setup(&recordContext);
    
  //  ofImage img;
  //  img.loadImage("back.png");
  //  backImg.setFromPixels(img.getPixels(), W, H);
  //  bProcess = true;
    
    soundStream.listDevices();
	
	//if you want to set a different device id
	//soundStream.setDeviceID(0); //bear in mind the device id corresponds to all audio devices, including  input-only and output-only devices.
	
	int bufferSize = 256;
	
	
	left.assign(bufferSize, 0.0);
	right.assign(bufferSize, 0.0);
	volHistory.assign(400, 0.0);
	
	bufferCounter	= 0;
	drawCounter		= 0;
	smoothedVol     = 0.0;
	scaledVol		= 0.0;
    
	soundStream.setup(this, 0, 2, 44100, bufferSize, 4);
}

//--------------------------------------------------------------
void testApp::update(){
    bool bNewFrame = false;
    
    vidGrabber.grabFrame();
    bNewFrame = vidGrabber.isFrameNew();
    
    // Get infrarred image from kinect and transform to OpenCV image
    //recordContext.update();
    //recordImage.update();
    
	if (bNewFrame){
        colorImg.setFromPixels(vidGrabber.getPixels(), W, H);
        grayImage = colorImg;
       
        if (bLearnBakground == true){
			grayBg = grayImage;		
			bLearnBakground = false;
		}
        
		// take the abs value of the difference between background and incoming and then threshold:
		grayDiff.absDiff(grayBg, grayImage);
        grayDiff.threshold(50);
        contourFinder.findContours(grayDiff, 20, (640*480)/3, 10, false);

        
    }
    
    
    // Save background
//    if(bBackground){
//        saveBackground();
//        bBackground = false;
//    }
//    
//    
    // ROI mask selection
//    drawRoiMask(grayImage);
//
// 
//    roi.x = MIN(roiMask[0].x, roiMask[3].x);
//    roi.y = MIN(roiMask[0].y, roiMask[1].y);
//    roiW = MAX(roiMask[1].x, roiMask[2].x) - roi.x;
//    roiH = MAX(roiMask[3].y, roiMask[2].y) - roi.y;
//    
//    grayImage.setROI(roi.x, roi.y, roiW, roiH);
//    backImg.setROI(roi.x, roi.y, roiW, roiH);
//    grayAcc.setROI(roi.x, roi.y, roiW, roiH);
//    
    
    // Opencv preprocessing
  /*
    t0 = ofGetElapsedTimeMillis();
    
    if(bProcess){
       // grayImage.absDiff(grayImage, backImg);
        grayImage.brightnessContrast(brightness, contrast);
        cvAdaptiveThreshold(grayImage.getCvImage(), grayImage.getCvImage(), 255);
        grayImage.blur();
        grayImage.erode();
        grayImage.dilate();
        
        grayImage.addWeighted(grayAcc, 0.1);
        grayAcc = grayImage;
        grayImage.canny(50, 150);
        
        Mat dst = grayImage.getCvImage();
        findContours(dst, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
        for (size_t i = 0; i < contours.size(); i++){
            CvPoint* pts = new CvPoint[contours[i].size()];
            for(int j = 0; j < contours[i].size(); j ++){
                pts[j].x = contours[i][j].x;
                pts[j].y = contours[i][j].y;
            }
            
            int nPts = contours[i].size();
            cvPolyLine( grayImage.getCvImage(), &pts, &nPts, 1, true, CV_RGB(255, 255, 255));
            delete[] pts;
        }
        grayImage.dilate();
        grayImage.erode();
    }*/
    t1 = ofGetElapsedTimeMillis() - t0;
     
    //---
    
    
//	// Send image to MadMapper
//    t0 = ofGetElapsedTimeMillis();
//    tex.allocate(roiW, roiH, GL_LUMINANCE);
//    tex.loadData(grayImage.getRoiPixels(), roiW, roiH, GL_LUMINANCE);
//    individualTextureSyphonServer.publishTexture(&tex);
//    //---
//    
//	// Get image from MadMapper
//    if(!fbo.isAllocated() || !(mClient.getWidth() == fbo.getWidth() && mClient.getHeight() == fbo.getHeight()))
//        fbo.allocate(mClient.getWidth(), mClient.getHeight());
//    
//    fbo.begin();
//    mClient.draw(0, 0);
//    fbo.end();
//    
//    fbo.readToPixels(pix);
//    
//    t2 = ofGetElapsedTimeMillis() - t0;
//    //---
//    
    
    //lets scale the vol up to a 0-1 range
	scaledVol = ofMap(smoothedVol, 0.0, 0.17, 0.0, 1.0, true);
    
	//lets record the volume into an array
	volHistory.push_back( scaledVol );
	
	//if we are bigger the the size we want to record - lets drop the oldest value
	if( volHistory.size() >= 400 ){
		volHistory.erase(volHistory.begin(), volHistory.begin()+1);
	}
    cout << scaledVol << endl;
    
    ofxOscMessage m;
    m.setAddress("/fire");
    if(scaledVol > 0.95)
        m.addIntArg(1);
    else
        m.addIntArg(0);
        
    sender.sendMessage(m);
}

//--------------------------------------------------------------
void testApp::draw(){
    ofSetHexColor(0xFFFFFF);
    ofBackground(0);

//    grayImage.draw(0, 0, 320, 240);
     grayDiff.draw(0, 0, 640, 480);
    //if(bShowInput)
     //   grayImage.drawROI(roi.x, roi.y);
    //if(bShowOutput) fbo.draw(0, 0);
    
    // then draw the contours:
    
	ofSetHexColor(0xff0000);
    
	// we could draw the whole contour finder
  //  contourFinder.draw(0, 0);
    
    for (int i = 0; i < contourFinder.nBlobs && i < 1; i++){
        ofVec2f p(contourFinder.blobs[i].centroid.x + offset.x, contourFinder.blobs[i].centroid.y + offset.y);
        ofEllipse(p.x, p.y, 4, 4);
        
        ofxOscMessage m;
        m.setAddress("/position");
        m.addFloatArg(float(p.x) / W);
        m.addFloatArg(float(p.y) / H) ;
        sender.sendMessage(m);
    
    }
    
    if(bInfo){
        ofSetHexColor(0xFF0000);
        char reportStr[1024];
    
        sprintf(reportStr, "[P] process on/off [F] snapshot [7 8 9 0] roi mask");
        ofDrawBitmapString(reportStr, 20, 10);
        sprintf(reportStr, "fps:%3.0f opencv:%3.2f madMapper:%3.2f", ofGetFrameRate(), t1, t2);
        ofDrawBitmapString(reportStr, 20, 25);
        sprintf(reportStr, "[1] show input [2] show output [i] info ");
        ofDrawBitmapString(reportStr, 20, 40);
        sprintf(reportStr, "[c] Contrast %.2f [b] Brightness %.2f ", contrast, brightness);
        ofDrawBitmapString(reportStr, 20, 55);
        sprintf(reportStr, "gray image [%4d, %4d] fbo [%4.f, %4.f] ",
                roiW, roiH, fbo.getWidth(), fbo.getHeight());
        
        int idx = (mouseY * pix.getWidth()+ mouseX) * pix.getBytesPerPixel();
        
        sprintf(reportStr, "pixels %d", pix.getPixels()[idx]);
        ofDrawBitmapString(reportStr, 20, 85);
    } 
}


//--------------------------------------------------------------
void testApp::keyPressed(int key){
    if(key > 0 && key <255)
        keys[key] = true;
    
	switch (key){
        case 357: //UP
            offset.y -= 5;

            if(keys['b']) brightness += 0.01;
            if(keys['c']) contrast += 0.01;
            
            if(keys['7']) roiMask[0].y -= 1;
            if(keys['8']) roiMask[1].y -= 1;
            if(keys['9']) roiMask[2].y -= 1;
            if(keys['0']) roiMask[3].y -= 1;
            
            break;
            
        case 359: //Down
            
            offset.y += 5;
            
            if(keys['b']) brightness -= 0.01;
            if(keys['c']) contrast -= 0.01;
            
            if(keys['7']) roiMask[0].y += 1;
            if(keys['8']) roiMask[1].y += 1;
            if(keys['9']) roiMask[2].y += 1;
            if(keys['0']) roiMask[3].y += 1;
            
            break;
        case 358: //RIGHT
            
            offset.x += 5;
            if(keys['7']) roiMask[0].x += 1;
            if(keys['8']) roiMask[1].x += 1;
            if(keys['9']) roiMask[2].x += 1;
            if(keys['0']) roiMask[3].x += 1;
            break;
        case 356: //LEFT
            
            offset.x -= 5;
            if(keys['7']) roiMask[0].x -= 1;
            if(keys['8']) roiMask[1].x -= 1;
            if(keys['9']) roiMask[2].x -= 1;
            if(keys['0']) roiMask[3].x -= 1;
            break;
            
        case 'F':
            bBackground = true;
            break;
        case 'R':
            resetRoiMask(grayImage);
            break;
        case 'P':
            bProcess = !bProcess;
            break;
        case 's':
            saveSettings();
            break;
        case 'l':
            loadSettings();
            break;
        case 'i':
            bInfo = !bInfo;
            break;
        case '1':
            bShowInput = !bShowInput;
            break;
        case '2':
            bShowOutput = !bShowOutput;
            break;
        case 'f':
            ofToggleFullscreen();
            break;
        case ' ':
			bLearnBakground = true;
            break;
	}
}


//--------------------------------------------------------------
void testApp::keyReleased(int key){
    if(key > 0 && key <255)
        keys[key] = false;
    }

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){
    
}

void testApp::loadSettings(){
    
    if( XML.loadFile("mySettings.xml") ){
		cout << "mySettings.xml loaded!" <<endl;
	}else{
		cout <<  "unable to load mySettings.xml check data/ folder" << endl;
	}
    
    XML.pushTag("SETTINGS");
    bProcess = XML.getValue("bPROCESS", 0);
    brightness = XML.getValue("BRIGHTNESS", 0.0);
    contrast = XML.getValue("CONTRAST", 0.0);
    bInfo = XML.getValue("bINFO", 1);
    bShowInput = XML.getValue("bSHOWINPUT", 1);
    bShowOutput = XML.getValue("bSHOWOUTPUT", 1);
    XML.popTag();
    
    
    XML.pushTag("ROIMASK");
    for(int i = 0; i < 4; i++){
        XML.pushTag("PT", i);
        roiMask[i].x = XML.getValue("X", 0);
        roiMask[i].y = XML.getValue("Y", 0);
        XML.popTag();
    }
    XML.popTag();

    cout << "Settings loaded" << endl;
    
}

//--------------------------------------------------------------
void testApp::saveSettings(){
    XML.clear();
    XML.setValue("SETTINGS:bPROCESS", bProcess);
        XML.setValue("SETTINGS:BRIGHTNESS", brightness);
    XML.setValue("SETTINGS:CONTRAST", contrast);
    XML.setValue("SETTINGS:bSHOWINPUT", bShowInput);
    XML.setValue("SETTINGS:bSHOWOUTPUT", bShowOutput);
    XML.setValue("SETTINGS:bINFO", bInfo);
     
    
    XML.addTag("ROIMASK");
    XML.pushTag("ROIMASK");
    for(int i = 0; i < 4; i++){
        XML.addTag("PT");
        XML.pushTag("PT", i);
        XML.setValue("X", roiMask[i].x,i);
        XML.setValue("Y", roiMask[i].y,i);
        XML.popTag();
    }
    XML.popTag();
    
    XML.saveFile("mySettings.xml");
    cout << "Settings saved" << endl;
}
void testApp::saveBackground(){
    ofPixels p;
    p.setFromPixels(grayImage.getPixels(), grayImage.getWidth(), grayImage.getHeight(), 1);
    ofSaveImage(p, "back.png");
    backImg = grayImage;
    cout << "Background saved" << endl;
}
//--------------------------------------------------------------
void testApp::drawRoiMask(ofxCvGrayscaleImage &img){
    
    int nPts = 4;
    int c = 0;
    int w = img.getWidth();
    int h = img.getHeight();
    
    CvPoint* pts = new CvPoint[nPts];
    
    pts[0].x = 0;
    pts[0].y = 0;
    pts[1].x = w - 1;
    pts[1].y = 0;
    pts[2].x = roiMask[1].x;
    pts[2].y = roiMask[1].y;
    pts[3].x = roiMask[0].x;
    pts[3].y = roiMask[0].y;
    
    cvFillPoly( img.getCvImage(), &pts, &nPts, 1,
               CV_RGB(c,c,c) );
    
    pts[0].x = 0;
    pts[0].y = 0;
    pts[1].x = roiMask[0].x;
    pts[1].y = roiMask[0].y;
    
    pts[2].x = roiMask[3].x;
    pts[2].y = roiMask[3].y;
    
    pts[3].x = 0;
    pts[3].y = h - 1;
    
    cvFillPoly( img.getCvImage(), &pts, &nPts, 1,
               CV_RGB(c,c,c) );
    
    
    pts[0].x = w - 1;
    pts[0].y = 0;
    pts[1].x = w - 1;
    pts[1].y = h - 1;
    pts[2].x = roiMask[2].x;
    pts[2].y = roiMask[2].y;
    pts[3].x = roiMask[1].x;
    pts[3].y = roiMask[1].y;
    
    cvFillPoly( img.getCvImage(), &pts, &nPts, 1,
               CV_RGB(c,c,c) );
    
    
    pts[0].x = w - 1;
    pts[0].y = w - 1;
    pts[1].x = 0;
    pts[1].y = h - 1;
    pts[2].x = roiMask[3].x;
    pts[2].y = roiMask[3].y;
    pts[3].x = roiMask[2].x;
    pts[3].y = roiMask[2].y;
    
    cvFillPoly( img.getCvImage(), &pts, &nPts, 1,
               CV_RGB(c,c,c) );
    
}
void testApp::resetRoiMask(ofxCvGrayscaleImage img){
    int w = img.getWidth();
    int h = img.getHeight();

    roiMask[0] = ofPoint(0,0);
    roiMask[1] = ofPoint(w-1,0);
    roiMask[2] = ofPoint(w-1,h-1);
    roiMask[3] = ofPoint(0,h-1);
    
}

//--------------------------------------------------------------
void testApp::audioIn(float * input, int bufferSize, int nChannels){
	
	float curVol = 0.0;
	
	// samples are "interleaved"
	int numCounted = 0;
    
	//lets go through each sample and calculate the root mean square which is a rough way to calculate volume
	for (int i = 0; i < bufferSize; i++){
		left[i]		= input[i*2]*0.5;
		right[i]	= input[i*2+1]*0.5;
        
		curVol += left[i] * left[i];
		curVol += right[i] * right[i];
		numCounted+=2;
	}
	
	//this is how we get the mean of rms :)
	curVol /= (float)numCounted;
	
	// this is how we get the root of rms :)
	curVol = sqrt( curVol );
	
	smoothedVol *= 0.93;
	smoothedVol += 0.07 * curVol;
	
	bufferCounter++;
	
}

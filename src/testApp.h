#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxSyphon.h"
#include "ofxXmlSettings.h"
#include "ofxOsc.h"


class testApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    
    ofxXmlSettings XML;
    
    void saveSettings();
    void loadSettings();
    
    void saveBackground();
    
    ofVideoGrabber 		vidGrabber;
    
    ofxCvGrayscaleImage 	grayImage, grayAcc, backImg;
    
    ofxCvColorImage			colorImg;
    ofxCvGrayscaleImage 	grayBg;
    ofxCvGrayscaleImage 	grayDiff;
    
    ofxSyphonServer individualTextureSyphonServer;
    ofxSyphonServer mainOutputSyphonServer;
    ofxSyphonClient mClient;
    ofTexture tex;
    
    
    
    
    ofPoint roiMask[4];
    void drawRoiMask(ofxCvGrayscaleImage &img);
    void resetRoiMask(ofxCvGrayscaleImage img);
    ofPoint roi;
    int roiW, roiH;
    
    
    
    bool keys[255];
    bool bBackground, bProcess, bInfo;
    float brightness, contrast;
    
    bool bShowInput, bShowOutput;
    
    bool				bLearnBakground;
    
    ofxCvContourFinder 	contourFinder;
    ofVec2f offset;
    ofxOscSender sender;
    
    
    void audioIn(float * input, int bufferSize, int nChannels);
	
    vector <float> left;
    vector <float> right;
    vector <float> volHistory;
    
    int 	bufferCounter;
    int 	drawCounter;
    
    float smoothedVol;
    float scaledVol;
    
    ofSoundStream soundStream;
};


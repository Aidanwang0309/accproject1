//ACC Project 1 Created by Alex Nathanson, Shuai Wang

#pragma once

#define WEBCAM

#include "ofMain.h"


class URLImage : public ofImage {
public:
	bool    bDoneLoading;
	string  url;
};

class ofApp : public ofBaseApp {
public:

	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	//    void mouseReleased(int x, int y, int button);
	//    void windowResized(int w, int h);
	//   void dragEvent(ofDragInfo dragInfo);
	//    void gotMessage(ofMessage msg);

	ofShader shader;

	ofImage image;
	ofImage imageMask;
	ofImage image2;
	ofImage screenshot;

	ofVideoGrabber camera;

	ofFbo fbo;
	ofFbo maskFbo;

	int camWidth;
	int camHeight;

	//for allocating textures
	string tex1;
	string tex2;
	string tex0;
	int caseNum;

	//alex's 
	ofImage resizeEven(ofImage input);
	float newX;
	float newY;

	ofImage tileIt(ofImage imgT);
	//the repetitions for tiling
	float rep;

	ofImage contrast(ofImage conIn, float factor);

	ofImage colorSep(ofImage input);

	int constrain(int clamp);

	//brightness function - used by tile
	ofImage brightness(ofImage img, float red, float green, float blue);
	float brightOp;

	//the input image - no changes are made to this image to avoid feedback
	ofImage original;

	//used to display the original
	ofImage toriginal;

	ofImage fxImage;

	//the output version
	ofImage copy;

	// X and Y dimensions of the original input image
	int origX;
	int origY;

	//google image stuff

	void              searchGoogleImages(string term);
	int               page;
	string            rawData;
	vector <string>   urls;
	vector <URLImage> images;
	string 			searchPhrase;

};

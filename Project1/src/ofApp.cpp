#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
#ifdef TARGET_OPENGLES
    shader.load("shadersES2/shader");
#else
    if(ofIsGLProgrammableRenderer()){
        shader.load("shadersGL3/shader");
    }else{
        shader.load("shadersGL2/shader");
    }
#endif
    
    int camWidth = 520;
    int camHeight = 240;
    
    camera.setVerbose(false);
    camera.initGrabber(camWidth, camHeight);
    
    image2.loadImage("image1.jpg");
    
    image.loadImage("image2.jpg");
    imageMask.loadImage("mask.jpg");

    fbo.allocate(camWidth, camHeight);
    maskFbo.allocate(camWidth, camHeight);
}

//--------------------------------------------------------------
void ofApp::update(){
    camera.update();
//    movie.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    //------------------------------------------- draw to mask fbo.
    maskFbo.begin();
    
    ofClear(255, 0, 0, 255);
    
    float imageMaskX = mouseX / (float)ofGetWidth();
    imageMaskX = ofClamp(imageMaskX, 0, 1);
    imageMaskX = -(imageMask.getWidth() - maskFbo.getWidth()) * imageMaskX;
    imageMask.draw(imageMaskX, 0);
    
    maskFbo.end();
    
    //------------------------------------------- draw to final fbo.
    fbo.begin();
    ofClear(0, 0, 0,255);
    
    shader.begin();
    shader.setUniformTexture("tex0", camera.getTextureReference(), 1);
    shader.setUniformTexture("tex1", image, 2);
    shader.setUniformTexture("tex2", image2.getTexture(), 3);
    shader.setUniformTexture("imageMask", maskFbo.getTextureReference(), 4);
    
    // we are drawing this fbo so it is used just as a frame.
    maskFbo.draw(0, 0);
    
    shader.end();
    fbo.end();
    
    fbo.draw(0,0);
    ofDrawBitmapString("Final FBO", 320+10+30,240*2+15+30);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key == 'x'){
        screenshot.grabScreen(0, 0 , ofGetWidth(), ofGetHeight());
        screenshot.save("screenshot.png");
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

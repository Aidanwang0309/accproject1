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
    
	//changed from 520 x 240
    int camWidth = 520;
    int camHeight = 520;
    
    camera.setVerbose(false);
    camera.initGrabber(camWidth, camHeight);
    
    image2.loadImage("image1.jpg");
	image2.resize(camWidth, camHeight);

    original.loadImage("myImage.jpg");
	original.resize(camWidth, camHeight);

   image.loadImage("image2.jpg");
   image.resize(camWidth, camHeight);


    fbo.allocate(camWidth, camHeight);
    maskFbo.allocate(camWidth, camHeight);

	rep = 20.0;
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
    
	imageMask = tileIt(original);

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

void ofApp::mousePressed(int x, int y, int button) {
	float newRep = ofMap(y, 0, 1080, 1, 60);

	rep = (int)newRep;

	std::cout << "REP: " << rep << "\n";

	float newBright = ofMap(x, 0, 1920, .0, 2., true);

	brightOp = newBright;
}

ofImage ofApp::brightness(ofImage img, float red, float green, float blue) {

	//must be returned to a different image than the one referenced by img, otherwise it will feedback
	ofImage outImg;

	ofColor newColor;
	ofColor oldColor;

	int imgX = img.getWidth();
	int imgY = img.getHeight();

	ofPixelsRef newPix = img.getPixels();

	//set pixels of specific copy
	for (int d = 0; d < imgX; d++) {
		for (int c = 0; c < imgY; c++) {

			ofColor oldColor = img.getColor(d, c);

			//will distort colors if over 255 - set a max and min gate
			newColor.r = oldColor.r * red;
			newColor.g = oldColor.g * green;// *green;
			newColor.b = oldColor.b * blue;// *blue;

			//set to multColor to see pixelated version. set to oldColor to see normal tiled version
			newPix.setColor(d, c, newColor);
		}
	}
	outImg.setFromPixels(newPix);
	return outImg;
}


ofImage ofApp::tileIt(ofImage imgT) {

	imgT = resizeEven(imgT);

	int imgX;
	int imgY;

	imgX = imgT.getWidth();
	imgY = imgT.getHeight();

	ofImage imgTclone;
	imgTclone.clone(imgT);
	ofPixelsRef tilePix = imgTclone.getPixels();

	imgT.resize(imgX / (float)rep, imgY / (float)rep);

	std::cout << "New dims: " << imgT.getWidth() << " x " << imgT.getHeight() << "\n";
	std::cout << "Remainders: " << fmod((float)origX, (float)imgT.getWidth()) << " x " << fmod(origY, imgT.getHeight()) << "\n";

	//create downsampled reference image
	ofImage refT;
	refT.clone(imgT);
	refT.resize(rep, rep);

	float frep = (float)rep;

	//ofImage tileOriginal;
	//tileOriginal.clone(original);

	//resize to an even split
	//tileOriginal.resize();

	ofImage lumaTemp;

	ofColor multColor;

	float red;
	float green;
	float blue;
	float avg;
	float alpha;

	//resize and draw new image
	//x

	for (int i = 0; i < rep; i++) {
		//y
		for (int a = 0; a < rep; a++) {

			float fi = (float)i;
			float fa = (float)a;

			multColor = refT.getColor(i, a);

			red = multColor.r / 256.0;
			green = multColor.g / 256.0;
			blue = multColor.b / 256.0;
			avg = ((red + blue + green) / 3.0) / 256.0;
			alpha = multColor.a / 256.0;

			//std::cout << red << green << blue << "\n";

			lumaTemp = brightness(imgT, red, green, blue);


			for (int t = 0; t < imgT.getWidth(); t++) {
				for (int z = 0; z < imgT.getHeight(); z++) {

					float fz = (float)z;
					float ft = (float)t;

					//oldest
					//tilePix.setColor((float)(ft * (float)origX / (float)rep), (fz * (float)origY / (float)rep), lumaTemp.getColor(ft, fz)); //remove the floats
					//old
					//tilePix.setColor((float)((fi * (float)((float)origX / frep)) + ft), (float)((fa * (float)((float)origY / frep)) + fz), lumaTemp.getColor(ft, fz)); //remove the floats

					float tx = ((newX / frep) * fi) + ft;
					float ty = ((newY / frep) * fa) + fz;

					tilePix.setColor(tx, ty, lumaTemp.getColor(t, z));
				}
			}
		}
	}

	// draw the downsampled image 
	//refT.draw(0, original.getHeight(), 200, 200);

	imgT.setFromPixels(tilePix);
	imgT.resize(origX, origY);

	return imgT;
}


ofImage ofApp::resizeEven(ofImage input) {

	float xDif;
	float yDif;

	float frep = rep;

	xDif = fmod(input.getWidth(), frep);
	yDif = fmod(input.getHeight(), frep);

	//std::cout << xDif << " " << yDif << "\n";

	input.resize((float)input.getWidth() - xDif, (float)input.getHeight() - yDif);

	newX = input.getWidth();
	newY = input.getHeight();

	return input;
}

ofImage ofApp::contrast(ofImage conIn) {

	ofColor moreContrast;
	
	int imgX = conIn.getWidth();
	int imgY = conIn.getHeight();

	ofPixelsRef newPix = conIn.getPixels();

	float factor;

	float contrast;

	contrast = 2.0 * 256;

	//set pixels of specific copy
	for (int d = 0; d < imgX; d++) {
		for (int c = 0; c < imgY; c++) {

			ofColor oldColor = conIn.getColor(d, c);

			factor = (259 * (contrast + 255)) / (255 * (259 - contrast));

			//will distort colors if over 255 - set a max and min gate
			moreContrast.r = constrain(factor * (oldColor.r - 128) + 128);	
			moreContrast.g = constrain(factor * (oldColor.r - 128) + 128);
			moreContrast.b = constrain(factor * (oldColor.r - 128) + 128);

										   //set to multColor to see pixelated version. set to oldColor to see normal tiled version
			newPix.setColor(d, c, moreContrast);
		}
	}
	conIn.setFromPixels(newPix);
	
	return conIn;
}

int ofApp::constrain(int clamp) {

	if (clamp > 255) {
		clamp = 255;
	}
	else if (clamp < 0) {
		clamp = 0;
	}

	return clamp;
}
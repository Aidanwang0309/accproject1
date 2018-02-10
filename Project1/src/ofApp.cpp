//ACC Project 1 Created by Alex Nathanson, Shuai Wang

#include "ofApp.h"

#include "Poco/RegularExpression.h"
using Poco::RegularExpression;
//--------------------------------------------------------------
void ofApp::setup() {

#ifdef TARGET_OPENGLES
	shader.load("shadersES2/shader");
#else
	if (ofIsGLProgrammableRenderer()) {
		shader.load("shadersGL3/shader");
	}
	else {
		shader.load("shadersGL2/shader");
	}
#endif

	//changed from 520 x 240
	camWidth = 640;
	camHeight = 480;

	camera.setVerbose(false);
	camera.initGrabber(camWidth, camHeight);

	page = 0;
	searchPhrase = "portraits";
	searchGoogleImages(searchPhrase);
	searchPhrase.clear();//clear our search phrase so we can type a new phrase

	image2.loadImage("image1.jpg");
	//image2 = images[1];
	image2.resize(camWidth, camHeight);

	original.loadImage("myImage.jpg");
	original.resize(camWidth, camHeight);


	imageMask.loadImage("myImage.jpg");
	//imageMask = images[0];
	imageMask.resize(camWidth, camHeight);

	//image = images[3];
	image.loadImage("image2.jpg");
	image.resize(camWidth, camHeight);


	fbo.allocate(camWidth, camHeight);
	maskFbo.allocate(camWidth, camHeight);

	rep = 20.0;
}

//--------------------------------------------------------------
void ofApp::update() {
	camera.update();
	//    movie.update();

	if (mouseX < 333) {
		caseNum = 1;
	}
	else if (mouseX >= 333 && mouseX < 666) {
		caseNum = 2;
	}
	else if (mouseX >= 666) {
		caseNum = 3;
	}

	switch (caseNum) {
	case 1:
		std::cout << "case 1! \n";
		tex0 = "tex0";
		tex1 = "tex1";
		tex2 = "tex2";
		break;
	case 2:
		std::cout << "case 2! \n";
		tex0 = "tex1";
		tex1 = "tex0";
		tex2 = "tex2";
		break;
	case 3:
		std::cout << "case 3! \n";
		tex0 = "tex2";
		tex1 = "tex1";
		tex2 = "tex0";
		break;
	}

	for (unsigned int i = 0; i<images.size(); i++) {
		if (!images[i].bDoneLoading) {
			images[i].load(images[i].url);
			images[i].bDoneLoading = true;
			break;
		}
	}

	imageMask = images[3];
	imageMask.resize(camWidth, camHeight);


	image = images[2];
	image.resize(camWidth, camHeight);

	image2 = images[0];
	image2.resize(camWidth, camHeight);

	fxImage = colorSep(contrast((camera.getPixels()), 0.1));
	fxImage = tileIt(fxImage);

}

//--------------------------------------------------------------
void ofApp::draw() {

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
	ofClear(0, 0, 0, 255);

	shader.begin();
	shader.setUniformTexture(tex0, maskFbo.getTexture(), 1);
	//was image
	shader.setUniformTexture(tex1, image2.getTexture(), 2);
	//was image2
	shader.setUniformTexture(tex2, image.getTexture(), 3);
	shader.setUniformTexture("imageMask", fxImage.getTexture(), 4);

	// we are drawing this fbo so it is used just as a frame.
	maskFbo.draw(0, 0);

	shader.end();
	fbo.end();

	fbo.draw(0, 0);
	ofDrawBitmapString("It me!", mouseX, mouseY);

	fxImage.resize(100, 100);
	fxImage.draw(0, 480);

	imageMask.resize(100, 100);
	imageMask.draw(100, 480);

	image.resize(100, 100);
	image.draw(200, 480);

	image2.resize(100, 100);
	image2.draw(300, 480);
	//images[0].draw(0, 480);

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	if (key == 'x') {
		screenshot.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
		screenshot.save("screenshot.png");
	}


	if (key == OF_KEY_DEL || key == OF_KEY_BACKSPACE) {
		searchPhrase = searchPhrase.substr(0, searchPhrase.length() - 1);
	}
	else if (key == OF_KEY_RETURN) {
		page += 22;
		searchGoogleImages(searchPhrase);
		searchPhrase.clear();
	}
	else {
		//we append our key character to the string searchPhrase
		ofAppendUTF8(searchPhrase, key);
	}


}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

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

	//std::cout << "New dims: " << imgT.getWidth() << " x " << imgT.getHeight() << "\n";
	//std::cout << "Remainders: " << fmod((float)origX, (float)imgT.getWidth()) << " x " << fmod(origY, imgT.getHeight()) << "\n";

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
	imgT.resize(camWidth, camHeight);

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

ofImage ofApp::contrast(ofImage conIn, float factor) {

	ofColor moreContrast;

	int imgX = conIn.getWidth();
	int imgY = conIn.getHeight();

	ofPixelsRef newPix = conIn.getPixels();

	//float factor;

	float contrast;

	contrast = factor * 256;

	//set pixels of specific copy
	for (int d = 0; d < imgX; d++) {
		for (int c = 0; c < imgY; c++) {

			ofColor oldColor = conIn.getColor(d, c);

			factor = (259 * (contrast + 255)) / (255 * (259 - contrast));

			//will distort colors if over 255 - set a max and min gate
			moreContrast.r = constrain(factor * (oldColor.r - 128) + 128);
			moreContrast.g = constrain(factor * (oldColor.g - 128) + 128);
			moreContrast.b = constrain(factor * (oldColor.b - 128) + 128);

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

ofImage ofApp::colorSep(ofImage input) {


	ofColor newColor;

	int imgX = input.getWidth();
	int imgY = input.getHeight();

	float colorAvg;

	ofPixels newPix = input.getPixels();

	//set pixels of specific copy
	for (int d = 0; d < imgX; d++) {
		for (int c = 0; c < imgY; c++) {

			newColor = newPix.getColor(d, c);

			//will distort colors if over 255 - set a max and min gate

			colorAvg = (newColor.r + newColor.g + newColor.b) / 3.0;
			if (colorAvg < (255 / 3)) {
				newColor.r = 255;
				newColor.g = 0;// *green;
				newColor.b = 0;// *blue;
			}
			else if (colorAvg >= (255 / 3) && colorAvg < (2 * (255 / 3))) {
				newColor.r = 0;
				newColor.g = 255;// *green;
				newColor.b = 0;// *blue;
			}
			else if (colorAvg >(2 * (255 / 3))) {
				newColor.r = 0;
				newColor.g = 0;// *green;
				newColor.b = 255;// *blue;
			}


			//set to multColor to see pixelated version. set to oldColor to see normal tiled version
			newPix.setColor(d, c, newColor);
		}
	}
	input.setFromPixels(newPix);
	return input;
}


//--------------------------------------------------------------
void ofApp::searchGoogleImages(string term) {

	// clear old imges    
	images.clear();
	urls.clear();

	// create the google url string
	ofStringReplace(term, " ", "%20");
	ofLogNotice() << term << endl;

	string googleImgURL = "http://www.google.com/search?q=" + term + "&tbm=isch&sout=1&tbs=isz&&start=" + ofToString(page);
	cout << "searching for " << googleImgURL << endl;

	ofHttpResponse res = ofLoadURL(googleImgURL);
	if (res.status > 0) {

		// copy over the response date fromt the url load
		rawData = res.data.getText();

		// first we want to get the search contents tag
		// in the webpage there is a table for all the images. we
		// want to get the content in the table using 
		// the <table> (.*?) </table> expression        
		RegularExpression regEx("<table class=\"images_table\" style=\"table-layout:fixed\" [^>]+>(.*?)</table>");
		RegularExpression::Match match;
		int found = regEx.match(rawData, match);


		// did we find the table tag with all the images
		// if so lets now start pulling out all the img tags
		if (found != 0) {

			// this is just the table content
			string contents = rawData.substr(match.offset, match.length);

			// setup the regex for img tags
			RegularExpression regEx("<img[^>]*src=\"([^\"]*)");
			RegularExpression::Match imgMatch;

			// now lets search for img tags in the content 
			// we keep search till we run out of img tags
			while (regEx.match(contents, imgMatch) != 0) {

				// we get the sub string from the content
				// and then trim the content so that we
				// can continue to search 
				string foundStr = contents.substr(imgMatch.offset, imgMatch.length);
				contents = contents.substr(imgMatch.offset + imgMatch.length);

				// setup the regex for src attribute in the img tag
				RegularExpression regImgEx("src=\"(.*?).$");
				RegularExpression::Match srcMatch;

				// if we found the src tag lets grab just
				// the url from the src attribute
				if (regImgEx.match(foundStr, srcMatch) != 0) {

					// save the img url
					string url = foundStr.substr(srcMatch.offset + 5, srcMatch.length);
					urls.push_back(url);

				}

			}
			// end while

		}
	}

	// load all the images
	for (unsigned int i = 0; i<urls.size(); i++) {
		images.push_back(URLImage());
		images.back().url = urls[i];
		images.back().bDoneLoading = false;
	}


	// just clean up for rendering to screen
	ofStringReplace(rawData, "\n", "");
	ofStringReplace(rawData, " ", "");
	ofStringReplace(rawData, "\t", "");
	string str;
	for (unsigned int i = 0; i<rawData.size(); i++) {
		str += rawData[i];
		if (i % 40 == 39) str += "\n";
	}
	rawData = str.substr(0, 2000) + "...";
}

#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {	
	ofSetFrameRate(frameRate);
    ofSetBackgroundAuto(true);
	//ofSetBackgroundColor(backgroundRgba[0], backgroundRgba[1], backgroundRgba[2], backgroundRgba[3]);
    ofBackground(0, 0, 0);
	ofSetVerticalSync(true);

	if (isLogToConsole) {
		ofLogToConsole();
	}

	if (isSilentLog) {
		ofSetLogLevel(OF_LOG_SILENT);
	}
    
    // setupCamera
    vector<ofVideoDevice> devices = cameras[0].listDevices();
	ofLog() << "Number of devices: " << devices.size();
    for (int i = 0; i < devices.size(); i++) {
        if (devices[i].bAvailable) {
			ofLog() << devices[i].id << ": " << devices[i].deviceName;
            cameras[i].setDeviceID(i);
            cameras[i].initGrabber(camW, camH);
            //Camera final Grab dimension set up:
            //cameras[i].setup(1280,720);
            int grabW = cameras[i].getWidth();
            int grabH = cameras[i].getHeight();
            ofLog()<<"asked for 120 by 240 - actual size is %i by %i"<< grabW << grabH ;
        } else {
			ofLog() << devices[i].id << ": " << devices[i].deviceName << " - unavailable";
        }
    }
    
    // setup button to camera ID
    // int camID = -1;
    
    /*
     for (int i = 0; i < totalBtn; i++) {

		ofLog() << "Button "<< i + 1 << "is going to Camera No. " << btnToCam[i];
    }
     */


    rawImg.allocate(camW, camH, OF_IMAGE_COLOR);
    rawImgInLoop.allocate(camW, camH, OF_IMAGE_COLOR);
    /* serial set up */    
	serial.listDevices();
    vector<ofSerialDeviceInfo> deviceList = serial.getDeviceList();
    serial.setup(0 , baud);
	/* end of serial set up */


	/* TCP set up */
	setupTcpClient();
	/* end of TCP set up */
    
    /*Set up Draw para,eter*/
    SetupSkeletonColor();
}

//--------------------------------------------------------------
void ofApp::update(){
	/* serial */
	if (isReadFromSerial) {
		readByteFromSerial();
	}
	/* end of serial */

	/* tcp */
	if (tcpClient.isConnected()) {
		//ofLog() << "TCP connected!";
		if (isWaitingForReply) {
			receiveTcpMsg();
		}
	}
	else {
		//ofLog() << "TCP disconnected!";
	}

	reconnectIfTimeoutInUpdate();
	/* end of tcp */


	/* cropping input image by body part data learnt from OpenPose */
	if (!isInputImageCropped) {
		cropCentreImageByBodyParts();
		isInputImageCropped = true;
	}
	/* end of cropping input image by body part data learnt from OpenPose */

	if (!isFlagUpdated && !isWaitingForReply && isInputImageCropped) {
		incrementFlag();
		isFlagUpdated = true;
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	if (croppedImg.isAllocated()) {
        ofSetColor(255);
        croppedImg.draw(firstTrimImgWInWindow, 0, croppedImgWHalf, windowH);
    }

	if (firstTrimImg.isAllocated()) {
        ofSetColor(255);
		firstTrimImg.draw(0, 0, firstTrimImgWInWindow, windowH);
	}

	//ofLog() << "People length: " << people.size();
	if (people.size() != 0) {		
		drawBodyParts();
		drawBodyPartConnections();
	}
    
    //animation Part
    if (Animating){
        AnimateNextFrame();
        callJsonLocal(flaginLoop);
        callLastJsonLocal(flaginLoop-1);
        drawAnimation();
        drawAnimationGIF();
        drawAnimationBodyPartLocus();
        //drawBodyPartConnections();
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	// https://stackoverflow.com/questions/628761/convert-a-character-digit-to-the-corresponding-integer-in-c
	//flrBtnPressed(key - '0');
    //============= for debug use ===============
    if (key == '0'){
        flrBtnPressed(0);
    }
    //--=========== end debug temp function =================-
    
    if (key == 'n'){
        callJsonLocal(flag);
        AnimateNextFrame();
    }
}


/* cameras and image manipulation */

// TriggerEvent
// Once Triggered,
// grab photo from camera[cameraID]
// CropA
void ofApp::flrBtnPressed(int btnID) { 
    //grab image from the designated camera according to button
    //int camID = btnToCam[btnID];
    int camID = ButtonTriggerToCam[btnID];
    cameras[camID].update();
    ofLog() << "Cam id: " << camID;
	ofLog() << "Current pressed Btn: " << btnID << "- is going to Camera No." << camID;
    rawImg.setFromPixels(cameras[camID].getPixels());
    
    //first trim
    //int trimStartX = btnToStartPoint[btnID];
	int trimStartX = 0;
    int trimStartY = 0;
    firstTrimImg.cropFrom(rawImg, trimStartX, trimStartY, firstTrimW, firstTrimH);
  
    if (flag < flagMAX) {
		//suppose at the end of here I plug the cropPhoto into the openPose function
		//openPose(cropPhoto);
		//save to a particular location
		string pathUnderData = imageInputDirUnderData + ofToString(flag) + imageExt;
		firstTrimImg.save(ofToDataPath(pathUnderData));

		string imgRelativePath = ofToDataPath(pathUnderData);		
		ofFile imgFile(imgRelativePath);
		string imgAbsolutePath = imgFile.getAbsolutePath();        
		ofLog() << "Image captured: " << imgAbsolutePath;

		/* base 64 encoding of image before sending via tcp */

		// Create an empty ofxIO::ByteBuffer.
		ofxIO::ByteBuffer buffer;

		// Load the file contents into the buffer.
		ofxIO::ByteBufferUtils::loadFromFile(imgRelativePath, buffer);

		// Base64 encode the images's bytes.
		//
		// Additional encoding option include URL-safety, chunking and padding.
		string base64Buffer = ofxIO::Base64Encoding::encode(buffer);

		/* end of base 64 encoding of image before sending via tcp */

		/* tcp */

		//setupTcpClient();
		if (tcpClient.isConnected()) {
			ofLog() << "Remarks: TCP connected!";
			tcpClient.send(base64Buffer);
			ofLog() << "String length of data sent: " << base64Buffer.length();
			//ofLog() << "Sent: " << base64Buffer;
			isWaitingForReply = true;
		}
		//tcpClient.close();

		/* end of tcp */
    }

	isFlagUpdated = false;
}

void ofApp::cropCentreImageByBodyParts() {  
	int leftBorder, rightBorder;
    float center = 0;
    float leftX = 0;
    float rightX = 0;

	ofxJSONElement firstPersonDetected = people[0];
	ofxJSONElement leftBodyPart = firstPersonDetected[leftBodyPartRef];
	ofxJSONElement rightBodyPart = firstPersonDetected[rightBodyPartRef];

	leftX = leftBodyPart[0].asFloat();
	rightX = rightBodyPart[0].asFloat();

    if (leftX > 0 && rightX > 0) {
        center = (leftX + rightX) * 0.5;
	}
	else {
		// TODO:
		ofLog() << "Can't Find Center: preliminary check";
		return;
	}

    // croppedImgWHalf is just an arbituary number for the final photo size
    leftBorder = center - croppedImgWHalf;
    rightBorder = center + croppedImgWHalf;
    ofLog() << "Crop result: ";
    ofLog() << "min: " << leftBorder << " max: "<< rightBorder;
    
	// crop optimal photo for display
    if (center > 0 && leftBorder > 0) {
		//ofLog() << "Crop begins";
        croppedImg.cropFrom(firstTrimImg, leftBorder, 0, rightBorder - leftBorder, firstTrimImg.getHeight());
		if (isSaveCroppedImgToFile) {
			//ofLog() << "Crop save starts";
			string pathUnderData = imageInputDirUnderData + ofToString(flag) + croppedImageFileSuffix + imageExt;
			croppedImg.save(ofToDataPath(pathUnderData));
			//ofLog() << "Crop save ends";
		}
		//ofLog() << "Crop ends";
    } else {
        ofLog() << "Can't Find Center: detail check";
    }
    
    
}

void ofApp::incrementFlag() {
	flag++;
	if (flag == totalBtn) {
		flag = 0;
	}
}

/* end of cameras and image manipulation */


/* serial */

void ofApp::readByteFromSerial() {
	int myByte = 0;
	if (serial.available()) {
		myByte = serial.readByte();

		string msgToLog = "";
		if (myByte == OF_SERIAL_NO_DATA)
			msgToLog = "no data was read";
		else if (myByte == OF_SERIAL_ERROR)
			msgToLog = "an error occurred";
		else
			msgToLog = "myByte is " + myByte;
		ofLog() << msgToLog;

		if (myByte <= 36) {
			// int TempBtnID = (myByte);
			int TempBtnID = (myByte + 1) / 2;
			ofLog() << "TempBtn is " << TempBtnID;

			/* Useful */
			// flrBtnPressed(0);
			// flrBtnPressed(myByte);
			//flrBtnPressed(TempBtnID);
            
            
            flrBtnPressed(myByte-1);
		}
	}
}

/* end of serial */


/* tcp */

void ofApp::setupTcpClient() {
	tcpClient.setup(hostIp, hostPort);
	tcpClient.setMessageDelimiter(tcpMsgDelimiter);
}

void ofApp::receiveTcpMsg() {
	string str = tcpClient.receive();
	if (str.length() > 0) {
		msgRx = str;
		ofLog() << "Received: " << msgRx;	\
		isWaitingForReply = false;

		if (isSaveJsonToFile) {
			saveJson();
		}
		
		isInputImageCropped = false;
	}
}

void ofApp::reconnectIfTimeoutInUpdate() {
	if (!tcpClient.isConnected()) {		
		deltaTime = ofGetElapsedTimeMillis() - connectTime;
		if (deltaTime > reconnectTimeMillis) {
			setupTcpClient();
			connectTime = ofGetElapsedTimeMillis();
		}
	}
}

/* end of tcp */


/* json */

void ofApp::saveJson() {
	// grab the data
	string data = msgRx;

	// parse it to JSON
	ofxJSONElement jsonResults;
	jsonResults.parse(data);
	people = jsonResults["people"];

	ofLog() << "People length: " << people.size();
	ofLog() << "Flag: " << flag;

	/*ofLog()  << "Nose 1: " << jsonResults["people"][0]["Nose"][0].asString()
	ofLog()  << "Nose 2: " << jsonResults["people"][0]["Nose"][1].asString()*/

	// output json text file
	string pathUnderData = jsonOutputDirUnderData + ofToString(flag) + jsonExt;;
	jsonResults.save(pathUnderData, isPrettifyJson);  // true means pretty
}

void ofApp::callJsonLocal(int JsonID){
    
    // get the absolute path of JSON file
    string pathJson = jsonOutputDirUnderData;
    pathJson += ofToString(JsonID) + jsonExt;
    string JsonRelativePath = ofToDataPath(pathJson);
    ofFile JsonFile(JsonRelativePath);
    string JsonAbsolutePath = JsonFile.getAbsolutePath();
    cout << "prcessed Json file path:  " << JsonAbsolutePath << endl;
    
    //
    resultInLoop = JsonFile;
    cout << "Current Json (animation Loop)  file path:  " << resultInLoop.getAbsolutePath() << endl;
    
    // parse data
    std::string file = resultInLoop.getAbsolutePath();
    
    
    // Now parse the JSON
    bool parsingSuccessful = resultInLoopJSON.open(file);
    
    if (parsingSuccessful){
        peopleInLoopJSON = resultInLoopJSON["people"];
        //ofxJSONElement jsonResults;
        //jsonResults.parse(file);
        
        
        ofLogNotice("ofApp::callJsonLocal : 1st PEOPLE -- JSON") << peopleInLoopJSON.getRawString(true);
        // now write pretty print
        
    }  else {
        ofLogNotice("ofApp::callJsonLocal") << "Failed to parse JSON.";
    }
}

//functiom to call JSON file of last frame in Local data folder
void ofApp::callLastJsonLocal(int LastJsonID){
    
    // get the absolute path of JSON file
    string pathJson = jsonOutputDirUnderData;
    pathJson += ofToString(LastJsonID) + jsonExt;
    string JsonRelativePath = ofToDataPath(pathJson);
    ofFile JsonFile(JsonRelativePath);
    string JsonAbsolutePath = JsonFile.getAbsolutePath();
    cout << "prcessed Json file path:  " << JsonAbsolutePath << endl;
    
    //
    resiltLastInLoop = JsonFile;
    cout << "Last Json (animation Loop) file path:  " << resiltLastInLoop.getAbsolutePath() << endl;
    
    // parse data
    std::string file = resiltLastInLoop.getAbsolutePath();
    
    
    // Now parse the JSON
    bool parsingSuccessful = resultLastInLoopJSON.open(file);
    
    if (parsingSuccessful){
        peopleLastInLoopJSON = resultLastInLoopJSON["people"];
        //ofxJSONElement jsonResults;
        //jsonResults.parse(file);
        
        
        ofLogNotice("ofApp::callLastJsonLocal : 1st PEOPLE -- JSON") << peopleLastInLoopJSON.getRawString(true);
        // now write pretty print
        
    }  else {
        ofLogNotice("ofApp::callLastJsonLocal") << "Failed to parse JSON.";
    }
}

/////////////////////////////

/////////////////////////////////////////
/* end of json */


/* draw */

//set the Skeleton Color
void ofApp::SetupSkeletonColor(){
    
    skeletonColors[0] = ofColor(114,0,157);
    skeletonColors[1] = ofColor(64,0,157);
    skeletonColors[2] = ofColor(167,0,102);
    skeletonColors[3] = ofColor(168,0,158);
    skeletonColors[4] = ofColor(168,0,42);
    skeletonColors[5] = ofColor(165,40,0);
    skeletonColors[6] = ofColor(82,159,0);
    skeletonColors[7] = ofColor(159,102,0);
    skeletonColors[8] = ofColor(159,158,0);
    skeletonColors[9] = ofColor(0,160,0);
    skeletonColors[10] = ofColor(0,140,0);
    skeletonColors[11] = ofColor(140,140,140);  // { "Neck", "RHip" }
    skeletonColors[12] = ofColor(0,160,37);     //{ "RHip", "RKnee" },
    skeletonColors[13] = ofColor(0,156,156);  //{ "RKnee", "RAnkle" },
    skeletonColors[14] = ofColor(140,140,140);  //{ "Neck", "LHip" },
    skeletonColors[15] = ofColor(14,31,158);  //{ "LHip", "LKnee" },
    skeletonColors[16] = ofColor(29,0,159);  //{ "LKnee", "LAnkle" },
    skeletonColors[17] = ofColor(29,0,189);  //  { "LHeel", "LBigToe"},
    skeletonColors[18] = ofColor(0,156,156);    // { "RHeel", "RBigToe"},
    skeletonColors[19] = ofColor(29,0,185);   //{"LAnkle","LHeel"},
    skeletonColors[20] = ofColor(0,156,156);    //{"RAnkle","RHeel"},
    skeletonColors[21] = ofColor(255,0,0);      //{"Neck","MidHip"},
    skeletonColors[22] = ofColor(0,99,157);     //{"LHip","MidHip"},
    skeletonColors[23] = ofColor(240,0,0);      //{"RHip","MidHip"}
    
}

// A function to draw humans body parts as circles
void ofApp::drawBodyParts() {
	for (int p = 0; p < people.size(); p++) {
		//int peopleSize = people.size();
		ofxJSONElement person = people[p];
		// Now that we have one human, let's draw its body parts
		for (int b = 0; b < person.size(); b++) {
			//int bodySize = person.size();
			ofxJSONElement body_part = person[POSE_BODY_25_BODY_PARTS.at(b)];
			// Body parts are relative to width and weight of the input
			float x = body_part[0].asFloat();
			float y = body_part[1].asFloat();

			if (x == 0 && y == 0) {
				continue;
			}

			ofDrawEllipse(x * firstTrimImgWInWindow / firstTrimW, y * windowH / firstTrimH, 10, 10);
		}
	}
}

void ofApp::drawBodyPartConnections() {
	for (int p = 0; p < people.size(); p++) {
		//int peopleSize = people.size();
		ofxJSONElement person = people[p];
		// Now that we have a human, let's draw its body
		// connections start by looping through all body
		// connections and matching only the ones we need.
		for (int c = 0; c < bodyPartConnections.size(); c++) {
			// Check if we have a pair in the current body parts
			ofxJSONElement start = person[bodyPartConnections[c][0]];
			ofxJSONElement end = person[bodyPartConnections[c][1]];

			if (start.size() > 0 && end.size() > 0) {
				float x1 = start[0].asFloat() * firstTrimImgWInWindow / firstTrimW;
				float y1 = start[1].asFloat() * windowH / firstTrimH;
				float x2 = end[0].asFloat() * firstTrimImgWInWindow / firstTrimW;
				float y2 = end[1].asFloat() * windowH / firstTrimH;

				if ((x1 == 0 && y1 == 0) || (x2 == 0 && y2 == 0)) {
					continue;
				}
				//ofSetColor(255, 0, 0);
				ofDrawLine(x1, y1, x2, y2);
			}
		}
	}
}
//======== DRAW AMO<ATION =========

void ofApp::drawAnimation(){
    int shiftDistance = 600;
    //DrawBody Points
    for (int p = 0; p < peopleInLoopJSON.size(); p++) {
        //int peopleSize = people.size();
        ofxJSONElement person = peopleInLoopJSON[p];
        // Now that we have one human, let's draw its body parts
        for (int b = 0; b < person.size(); b++) {
            //int bodySize = person.size();
            ofxJSONElement body_part = person[POSE_BODY_25_BODY_PARTS.at(b)];
            // Body parts are relative to width and weight of the input
            float x = body_part[0].asFloat();
            float y = body_part[1].asFloat();
            
            if (x == 0 && y == 0) {
                continue;
            }
            ofSetColor(255);
            ofDrawEllipse(x * firstTrimImgWInWindow / firstTrimW, y * windowH / firstTrimH+shiftDistance , 5, 5);
        }
    }
    
    // DrawBody Connection
    for (int p = 0; p < peopleInLoopJSON.size(); p++) {
        //int peopleSize = people.size();
        ofxJSONElement person = peopleInLoopJSON[p];
        // Now that we have a human, let's draw its body
        // connections start by looping through all body
        // connections and matching only the ones we need.
        for (int c = 0; c < bodyPartConnections.size(); c++) {
            // Check if we have a pair in the current body parts
            ofxJSONElement start = person[bodyPartConnections[c][0]];
            ofxJSONElement end = person[bodyPartConnections[c][1]];
            
            if (start.size() > 0 && end.size() > 0) {
                float x1 = start[0].asFloat() * firstTrimImgWInWindow / firstTrimW;
                float y1 = start[1].asFloat() * windowH / firstTrimH;
                float x2 = end[0].asFloat() * firstTrimImgWInWindow / firstTrimW;
                float y2 = end[1].asFloat() * windowH / firstTrimH;
                
                if ((x1 == 0 && y1 == 0) || (x2 == 0 && y2 == 0)) {
                    continue;
                }
                //ofSetColor(255, 0, 0);
                ofSetColor(skeletonColors[c]);
                ofDrawLine(x1, y1+shiftDistance, x2, y2+shiftDistance);
            }
        }
    }
    
    
    
}

void ofApp::drawAnimationBodyPartLocus() {
    int shiftDistance = 600;
    ofxJSONElement person = peopleInLoopJSON[0];
    ofxJSONElement personLast = peopleLastInLoopJSON[0];
    // Now that we have a human, let's draw its body
    // connections start by looping through all body
    // connections and matching only the ones we need.
    for (int c = 0; c < person.size(); c++) {
        //ofxJSONElement person[c];
        //ofxJSONElement personLast[c];
        // Check if we have a pair in the current body parts
        // TODO: double check bodyPartName == bodyPartConnections[c][0], maybe can optimize using dictionary, i.e. map
        
        ofxJSONElement body_part = person[POSE_BODY_25_BODY_PARTS.at(c)];
        ofxJSONElement body_part_pre = personLast[POSE_BODY_25_BODY_PARTS.at(c)];
        //  if (person[c].size() > 0 && personLast[c].size() > 0) {
        /*
         float x1 = body_part[0].asFloat() * windowW / camW;
         float y1 = body_part[1].asFloat() * windowH / camH;
         float x2 = body_part_pre[0].asFloat() * windowW / camW;
         float y2 = body_part_pre[1].asFloat() * windowH / camH;
         */
        float x1 = body_part[0].asFloat()* firstTrimImgWInWindow / firstTrimW;
        float y1 = body_part[1].asFloat()* firstTrimImgWInWindow / firstTrimW;
        float x2 = body_part_pre[0].asFloat()* firstTrimImgWInWindow / firstTrimW;
        float y2 = body_part_pre[1].asFloat()* firstTrimImgWInWindow / firstTrimW;
        
        if ((x1 == 0 && y1 == 0) || (x2 == 0 && y2 == 0)) {
            continue;
        }
        ofSetLineWidth(0.1);
        ofSetColor(200,200,200 ,128);
        ofDrawLine(x1, y1+shiftDistance, x2, y2+shiftDistance);
        
    }
    
    
}



void ofApp::drawAnimationGIF(){
    int Yshiftdistance = 600;
    //finding the centre
    int leftBorder, rightBorder;
    float center = 0;
    float leftX = 0;
    float rightX = 0;
    
    ofxJSONElement firstPersonDetected = peopleInLoopJSON[0];
    ofxJSONElement firstPersonDetectedLast = peopleLastInLoopJSON[0];
    
    ofxJSONElement leftBodyPart = firstPersonDetected[leftBodyPartRef];
    ofxJSONElement rightBodyPart = firstPersonDetected[rightBodyPartRef];
    
    leftX = leftBodyPart[0].asFloat();
    rightX = rightBodyPart[0].asFloat();
    
    if (leftX > 0 && rightX > 0) {
        center = (leftX + rightX) * 0.5;
    }
    else {
        // TODO:
        ofLog() << "Can't Find Center: preliminary check";
        return;
    }
    int ShiftDistance = (firstTrimW +croppedImgW/2 - center)/2;
    
    //display the image sequence
    if (AnimateingCroppedPhoto) {
        leftBorder = center - croppedImgWHalf;
        rightBorder = center + croppedImgWHalf;
        string pathUnderData = imageInputDirUnderData + ofToString(flaginLoop-1) + imageExt;
        rawImgInLoop.load(pathUnderData);
        ofLog() << "loading Image ", pathUnderData;
        
        croppedImgInLoop.cropFrom(rawImgInLoop, leftBorder, 0, rightBorder-leftBorder, rawImgInLoop.getHeight());
        
        //croppedImgInLoop.cropFrom(firstTrimImg, leftBorder, 0, rightBorder - leftBorder,
        ofSetColor(255);
        croppedImgInLoop.draw(firstTrimImgWInWindow,Yshiftdistance, croppedImgWHalf, windowH);
        
    }
    //DrawBody Points
    for (int p = 0; p < peopleInLoopJSON.size(); p++) {
        //int peopleSize = people.size();
        ofxJSONElement person = peopleInLoopJSON[p];
        // Now that we have one human, let's draw its body parts
        for (int b = 0; b < person.size(); b++) {
            //int bodySize = person.size();
            ofxJSONElement body_part = person[POSE_BODY_25_BODY_PARTS.at(b)];
            // Body parts are relative to width and weight of the input
            float x = body_part[0].asFloat();
            float y = body_part[1].asFloat();
            
            if (x == 0 && y == 0) {
                continue;
            }
            ofSetColor(255);
            ofDrawEllipse(x * firstTrimImgWInWindow / firstTrimW +ShiftDistance, y * windowH / firstTrimH+Yshiftdistance , 5, 5);
        }
    }
    
    // DrawBody Connection
    for (int p = 0; p < peopleInLoopJSON.size(); p++) {
        //int peopleSize = people.size();
        // Now that we have a human, let's draw its body
        // connections start by looping through all body
        // connections and matching only the ones we need.
        ofxJSONElement person =peopleInLoopJSON[0];
        for (int c = 0; c < bodyPartConnections.size(); c++) {
            // Check if we have a pair in the current body parts
            ofxJSONElement start = person[bodyPartConnections[c][0]];
            ofxJSONElement end = person[bodyPartConnections[c][1]];
            
            if (start.size() > 0 && end.size() > 0) {
                float x1 = start[0].asFloat() * firstTrimImgWInWindow / firstTrimW;
                float y1 = start[1].asFloat() * windowH / firstTrimH;
                float x2 = end[0].asFloat() * firstTrimImgWInWindow / firstTrimW;
                float y2 = end[1].asFloat() * windowH / firstTrimH;
                
                if ((x1 == 0 && y1 == 0) || (x2 == 0 && y2 == 0)) {
                    continue;
                }
                //ofSetColor(255, 0, 0);
                ofSetColor(skeletonColors[c]);
                //ofDrawLine(x1 + (animateWindowCentre-centerX)/2 +800, y1, x2+(animateWindowCentre-centerX)/2+800, y2);
                ofDrawLine(x1 +ShiftDistance, y1+Yshiftdistance, x2+ShiftDistance, y2+Yshiftdistance);
            }
        }
    }
    
    // Draw Locus of the body Point Cloud
    //peopleLastInLoopJSON
    
    for (int p = 0; p < peopleInLoopJSON.size(); p++){
         ofxJSONElement person = peopleInLoopJSON[p];
         ofxJSONElement personLast = peopleLastInLoopJSON[p];
        
        for (int c = 0; c < bodyPartConnections.size(); c++) {
        
        
        ofxJSONElement body_part = person[POSE_BODY_25_BODY_PARTS.at(c)];
        ofxJSONElement body_part_pre = personLast[POSE_BODY_25_BODY_PARTS.at(c)];
        //  if (person[c].size() > 0 && personLast[c].size() > 0) {
        float x1 = body_part[0].asFloat() * firstTrimImgWInWindow / firstTrimW;
        float y1 = body_part[1].asFloat() * windowH / firstTrimW;
        float x2 = body_part_pre[0].asFloat() * firstTrimImgWInWindow / firstTrimW;
        float y2 = body_part_pre[1].asFloat() * windowW / firstTrimW;
        
        if ((x1 == 0 && y1 == 0) || (x2 == 0 && y2 == 0)) {
            continue;
        }
        ofSetLineWidth(0.1);
        ofSetColor(200,200,200 ,128);
        //ofDrawLine(x1+ShiftDistance, y1+Yshiftdistance, x2+ShiftDistance, y2+Yshiftdistance);
            ofDrawLine(x1,y1,x2,y2);
        }
    }
  
}
/* end of draw */

//======== DRAW AMO<ATION =========

void ofApp::AnimateNextFrame(){
    ofDirectory PhotoFolder(jsonOutputDirUnderData);
    PhotoFolder.listDir();
    int NumberOfPhotoCaptured = PhotoFolder.size();
    if (flaginLoop<NumberOfPhotoCaptured){
        flaginLoop ++ ;}
    else{
        flaginLoop = 0;
    }
    cout << "Current Flag in Loop" << flaginLoop<< endl;
}

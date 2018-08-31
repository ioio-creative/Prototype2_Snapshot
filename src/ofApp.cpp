#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(60);
    ofSetBackgroundColor(0);
	ofSetVerticalSync(true);
    
    //setupCamera
    vector<ofVideoDevice> devices = cameras[0].listDevices();
    //cout << devices.size() << endl; 
    for(int i = 0; i < devices.size(); i++){
        if(devices[i].bAvailable){
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName;
            cameras[i].setDeviceID(i);
            cameras[i].initGrabber(camW,camH);
            
        }else{
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName << " - unavailable ";
        }
    }
    
    
    //setup button to camera ID
    //int camID = -1;
    for(int i = 0; i < totalBtn; i++){
    //    if(i % btnPerCam == 0){
    //      camID ++;
    // }
        //btnToCam[i] = camID;
        cout << "Button "<< i+1 << "is going to Camera No." <<btnToCam[i] << endl ;        
    }
    
    
    
    rawPhoto.allocate(1920, 1080, OF_IMAGE_COLOR);
    
    
    
    
    
    /* serial set up */    
	serial.listDevices();
    vector<ofSerialDeviceInfo> deviceList = serial.getDeviceList();
    serial.setup( 0 , baud );
	/* end of serial set up */


	/* TCP set up */
	setupTcpClient();
	/* end of TCP set up */
}

//--------------------------------------------------------------
void ofApp::update(){
//    getHumanFromOSC();
  
    int myByte = 0;
    if(serial.available()){
        myByte = serial.readByte();
//        if ( myByte == OF_SERIAL_NO_DATA )
//            printf("no data was read");
//        else if ( myByte == OF_SERIAL_ERROR )
//            printf("an error occurred");
//        else
            //printf("myByte is %d \n", myByte);
      
        if(myByte <= 36) {
            int TempBtnID = (myByte+1)/2 ;
           // int TempBtnID = (myByte);
            //printf("TempBtn is %d \n", TempBtnID);
          //  flrBtnPressed(0);
          //  flrBtnPressed(myByte);

			/* Useful */
            flrBtnPressed(TempBtnID);

            //supposedly you should call flrBtnPressed(btnID)
            
            //int TempBtnID =
            //flnBtnPressed(
        }
    }


	/* tcp */

	if (tcpClient.isConnected()) {
		//cout << "Remarks: TCP connected!" << endl;

		if (isWaitingForReply) {
			receiveTcpMsg();
		}
	}
	else {
		deltaTime = ofGetElapsedTimeMillis() - connectTime;
		
		if (deltaTime > reconnectTimeMillis) {
			setupTcpClient();

			connectTime = ofGetElapsedTimeMillis();
		}
	}

	/* end of tcp */
}

//--------------------------------------------------------------
void ofApp::draw() {    
    /*if(cropPhoto.isAllocated()){
        cropPhoto.draw(10,10);
    }

	if(optPhoto.isAllocated()){
        optPhoto.draw(10, 10, 500, 880);      
    }*/    

	//cout << "People length: " << people.size() << endl;

	if (people.size() != 0) {
		rawPhoto.draw(0, 0, windowW, windowH);
		drawBodyParts();
		drawBodyPartConnections();
	}    
}

// TriggerEvent
// Once Triggered,
// grab photo from camera[cameraID]
// CropA
void ofApp::flrBtnPressed(int btnID){
    
    //grab image from the designated camera according to button
    int camID = btnToCam[btnID];
    cameras[camID].update();
    ofLog() << camID;
    cout << "Current pressed Btn: "<< btnID << "- is going to Camera No." <<camID << endl ;
    rawPhoto.setFromPixels(cameras[camID].getPixels());
    
    //first crop
    int cropStartX = btnToStartPoint[btnID];
    int cropStartY = 0;
    //rawPhoto.crop(cropStartX,cropStartY,cropW,cropH);
    //cropPhoto.cropFrom(rawPhoto,cropStartX,cropStartY,cropW,cropH);
    cropPhoto.cropFrom(rawPhoto,0,0,cropW,cropH);
    
    //cropPhoto
    //suppose at the end of here I plug the cropPhoto into the openPos function
    //openPose(cropPhoto);
    //1. save to a particular location
    //string path = "/Users/chunhoiwong_ioio/Documents/OpenFramework X/of_v0.10.0_osx_release/apps/myApps/Prototype2/prototype-2/bin/data/nodeGetImage/public/";
	
	string pathUnderData = "input_images/";

    if (flag < 18) {
		pathUnderData += ofToString(flag) + ".jpg";
        cropPhoto.save(ofToDataPath(pathUnderData));

		string imgRelativePath = ofToDataPath(pathUnderData);
		
		ofFile imgFile(imgRelativePath);
		string imgAbsolutePath = imgFile.getAbsolutePath();
        
		cout << "Image captured: " << imgAbsolutePath << endl;
		


		/* osc */

        /*ofxOscMessage m;
        m.setAddress("/test");
        m.addIntArg(flag);
        sender.sendMessage(m, true);*/
        
		/* end of osc */


		/* tcp */

		//setupTcpClient();
		if (tcpClient.isConnected()) {
			cout << "Remarks: TCP connected!" << endl;
			tcpClient.send(imgAbsolutePath);
			cout << "Sent: " << imgAbsolutePath << endl;
			isWaitingForReply = true;			
		}
		//tcpClient.close();

		/* end of tcp */



		flag++;
		if (flag == 18) {
			flag = 0;
		}
    }
    
       //web server got OSC m, read photo from location
    
    //suppose it will pass the photo to runway
    //web server getSkeleton data JSON
    //web server send OSC of skeletonJSON
    
    
//OSC receive. Unpack JSON
//look for left border and right border
}

void ofApp::getHumanFromOSC(){
    // check for waiting messages
    //while(receiver.hasWaitingMessages()){
        ofLog() << "OSC got through" ;
        // get the next message
        ofxOscMessage m;
        //receiver.getNextMessage(m);
        
        //probably  insert the EXPORT JSON part here
        //============================================================
        
        
        
        //============================================================

       
        // check message from nodeGetImage
        if(m.getAddress() == "/ske"){
            // both the arguments are int32's
            float center = 0;
            float leftX = 0;
            float rightX = 0;
            for(int i=0; i< m.getNumArgs();i+=3){
                
                //the received array format as ["bodyPartA","bodyPartA-X-Coor","bodyPartA-Y-Coor","bodyPartB",..............]
                // and bodyPart coordinate is format as 0.0 - 1.0, 0.5 as the middle of the photo
                if(m.getArgAsString(i) == "Left_Hip"){
                    ofLog()<< m.getArgAsString(i);
                    ofLog()<< m.getArgAsFloat(i+1);
                    ofLog()<< m.getArgAsFloat(i+2);
                    leftX = m.getArgAsFloat(i+1);
                }else if (m.getArgAsString(i) == "Right_Hip"){
                    ofLog()<< m.getArgAsString(i);
                    ofLog()<< m.getArgAsFloat(i+1);
                    ofLog()<< m.getArgAsFloat(i+2);
                    rightX = m.getArgAsFloat(i+1);
                }
                
                //print every argument's x coordinate
                ofLog()<< m.getArgAsString(i) <<" 's X cordinate in Float" ;
                ofLog()<< m.getArgAsFloat(i+1);
                

    
            }
            if(leftX != 0 && rightX !=0){
                center = (leftX + rightX)/2;
            }else if(leftX == 0){
                center = rightX;
            }else if(rightX == 0){
                center = leftX;
            }
            //translating 0.0 - 1.0 to pixel coordinate, 250 is just an arbituary number for the final photo size
            leftBorder = center * cropW - 250;
            rightBorder = center * cropW + 250;
            //ofLog() << "min : " << minX << "  max : "<< maxX;
            ofLog() << "min : " << leftBorder << "  max : "<< rightBorder;
            //crop optimal photo for display
            if(center != 0){
                optPhoto.cropFrom(cropPhoto,leftBorder,0,rightBorder - leftBorder, cropPhoto.getHeight());
                optPhoto.save(ofToString(ofGetElapsedTimef())+".jpg");
            }else{
                ofLog() << "Can't Find Center";
            }
        }else{
            // unrecognized message: display on the bottom of the screen
            string msg_string;
            msg_string = m.getAddress();
            msg_string += ": ";
            for(int i = 0; i < m.getNumArgs(); i++){
                // get the argument type
                msg_string += m.getArgTypeName(i);
                msg_string += ":";
                // display the argument - make sure we get the right type
                if(m.getArgType(i) == OFXOSC_TYPE_INT32){
                    msg_string += ofToString(m.getArgAsInt32(i));
                }
                else if(m.getArgType(i) == OFXOSC_TYPE_FLOAT){
                    msg_string += ofToString(m.getArgAsFloat(i));
                }
                else if(m.getArgType(i) == OFXOSC_TYPE_STRING){
                    msg_string += m.getArgAsString(i);
                }
                else{
                    msg_string += "unknown";
                }
            }
           
        }

        
        
}

void ofApp::keyPressed(int key){
	// https://stackoverflow.com/questions/628761/convert-a-character-digit-to-the-corresponding-integer-in-c
	flrBtnPressed(key - '0');    
}


/* tcp */

void ofApp::setupTcpClient() {
	tcpClient.setup(hostIp, hostPort);
	tcpClient.setMessageDelimiter(tcpMsgDelimiter);
}

void ofApp::receiveTcpMsg() {
	string str = tcpClient.receive();
	if (str.length() > 0) {
		msgRx = str;
		cout << "Received: " << msgRx << endl;
		isWaitingForReply = false;
		
		processJsonResponse();
	}
}

/* end of tcp */


/* json */

void ofApp::processJsonResponse() {
	// grab the data
	string data = msgRx;

	// parse it to JSON
	jsonResults.clear();
	jsonResults.parse(data);
	people = jsonResults["people"];

	cout << "People length: " << people.size() << endl;
	cout << "Flag: " << flag << endl;

	/*cout << "Nose 1: " << jsonResults["people"][0]["Nose"][0].asString() << endl;
	cout << "Nose 2: " << jsonResults["people"][0]["Nose"][1].asString() << endl;*/

	// output json text file
	string pathUnderData = "output_jsons/" + ofToString(flag) + ".json";;
	jsonResults.save(pathUnderData, true);  // true means pretty
}

/* end of json */


/* draw */

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

			ofDrawEllipse(x * windowW / camW, y * windowH / camH, 10, 10);
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
			ofxJSONElement start;
			ofxJSONElement end;
			// Check if we have a pair in the current body parts
			// TODO: double check bodyPartName == bodyPartConnections[c][0], maybe can optimize using dictionary, i.e. map
			for (int b = 0; b < person.size(); b++) {
				string bodyPartName = POSE_BODY_25_BODY_PARTS.at(b);
				ofxJSONElement bodyPart = person[bodyPartName];				
				if (bodyPartName == bodyPartConnections[c][0]) {
					start = bodyPart;
				}
				else if (bodyPartName == bodyPartConnections[c][1]) {
					end = bodyPart;
				}
			}
			if (start.size() > 0 && end.size() > 0) {
				float x1 = start[0].asFloat() * windowW / camW;
				float y1 = start[1].asFloat() * windowH / camH;
				float x2 = end[0].asFloat() * windowW / camW;
				float y2 = end[1].asFloat() * windowH / camH;

				if ((x1 == 0 && y1 == 0) || (x2 == 0 && y2 == 0)) {
					continue;
				}

				ofDrawLine(x1, y1, x2, y2);
			}
		}
	}
}

/* end of draw */

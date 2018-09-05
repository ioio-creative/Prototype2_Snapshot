#pragma once

#include "ofMain.h"
#include "ofxJSON.h"
#include "ofxNetwork.h"

//==============================

/* app */
#define isLogToConsole true
#define isSilentLog true
#define frameRate 60
/* end of app */

/* cameras and image manipulation */
#define totalCamera 5
#define totalBtn 18
#define btnPerCam 6

#define camDeviceA 0
#define camDeviceB 1
#define camDeviceC 4
#define camDeviceD 2

#define camW 1920
#define camH 1080

#define firstTrimW 1920
#define firstTrimH 880
#define firstTrimWHalf (firstTrimW/2)
#define firstTrimHHalf (firstTrimH/2)

#define croppedImgW 500
#define croppedImgWHalf (croppedImgW/2)
 
#define imageExt ".jpg"
#define imageInputDirUnderData "input_images/"
#define croppedImageFileSuffix "_cropped"
#define isSaveCroppedImgToFile true

#define leftBodyPartRef "LHip"
//#define leftBodyPartRef "LEye"
#define rightBodyPartRef "RHip"
//#define rightBodyPartRef "REye"
/* end of cameras and image manipulation */


/* main window */
#define firstTrimImgWInWindow (firstTrimW/2)
#define windowW (firstTrimImgWInWindow+croppedImgWHalf)
#define windowH firstTrimHHalf
/* end of main window */

/* serial */
#define isReadFromSerial true
#define baud 9600
/* end of serial */

/* tcp */
#define hostIp "127.0.0.1"
#define hostPort 27156
#define tcpMsgDelimiter "[TCP]"
#define reconnectTimeMillis 5000
/* end of tcp */

/* json */
#define isPrettifyJson true
#define jsonExt ".json"
#define jsonOutputDirUnderData "output_jsons/"
#define isSaveJsonToFile true
/* end of json */

//==============================


class ofApp : public ofBaseApp{

	private:
		/* ofx life cycle hooks */
		void setup();
		void update();
		void draw();
		/* end of ofx life cycle hooks */
		
		/* ofx events */
		void keyPressed(int key);
		/* end of ofx events */

		/* main window */
		int backgroundRgba[4] = {0, 0, 0, 1};
		/* end of main window */
    
		/* cameras and image manipulation */

        // TriggerEvent
        // Once Triggered,
        // grab photo from camera[cameraID]
        // CropA
        void flrBtnPressed(int ID);
    
        int flag = 0;
        // cropPhoto TriggerEvent
        // Once Updated,
        // send cropPhoto to openPose
        // openPose crop
        // return optPhoto
        
        // optPhoto TriggerEvent
        // Once Updated,
        // Display        
        //cameras array
        ofVideoGrabber cameras[totalCamera];
        
        //buttons to Cam array [buttonID] i.e.{0,0,0,1,1,1,2,2,2}
        //int btnToCam[totalBtn] = {4,4,4,4,4,0,0,0,0,1,1,1,1,1,2,2,2,2};
		int btnToCam[totalBtn] = {0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1};
		//int btnToCam[totalBtn] = { 0,0,0,0,0,1,1,1,1,3,3,3,3,3,4,4,4,4 };
	    //int btnToCam2[36]={camDeviceD,camDeviceA,camDeviceA,camDeviceA,camDeviceA,camDeviceA,camDeviceA,camDeviceA,camDeviceA,camDeviceB,camDeviceB,camDeviceB,camDeviceB,camDeviceB,camDeviceB,camDeviceB,camDeviceB,camDeviceC,camDeviceC,camDeviceC,camDeviceC,camDeviceC,camDeviceC,camDeviceC,camDeviceC,camDeviceD,camDeviceD,camDeviceD,camDeviceD,camDeviceD,camDeviceD,camDeviceD};
        
        //Dimension array [buttonID] = {startPoint0, StartPoint1,...,startPoint17}
        //btnToStartPoint[] = {0,320,640,960,1280,1600,0,320,640,960,1280,1600,0,320,640,960,1280,1600}
        int btnToStartPoint[totalBtn] = {0,1200,1000,800,600,400,200,320,640,960,1280,1600,0,320,640,960,1280,1600};

		ofImage rawImg;
		ofImage firstTrimImg;
		ofImage croppedImg;

		bool isFlagUpdated = true;
			
		void cropCentreImageByBodyParts();
		void incrementFlag();

		/* end of cameras and image manipulation */

		/* serial */		
		ofSerial serial;
		void readByteFromSerial();
		/* end of serial */

		/* tcp */
		ofxTCPClient tcpClient;		
		int connectTime = 0;
		int deltaTime = 0;
		string msgRx = "";
		bool isWaitingForReply = false;

		void setupTcpClient();
		void receiveTcpMsg();
		void reconnectIfTimeoutInUpdate();
		/* end of tcp */

		/* json */
		const map<unsigned int, string> POSE_BODY_25_BODY_PARTS {
			{ 0,  "Nose" },
			{ 1,  "Neck" },
			{ 2,  "RShoulder" },
			{ 3,  "RElbow" },
			{ 4,  "RWrist" },
			{ 5,  "LShoulder" },
			{ 6,  "LElbow" },
			{ 7,  "LWrist" },
			{ 8,  "MidHip" },
			{ 9,  "RHip" },
			{ 10, "RKnee" },
			{ 11, "RAnkle" },
			{ 12, "LHip" },
			{ 13, "LKnee" },
			{ 14, "LAnkle" },
			{ 15, "REye" },
			{ 16, "LEye" },
			{ 17, "REar" },
			{ 18, "LEar" },
			{ 19, "LBigToe" },
			{ 20, "LSmallToe" },
			{ 21, "LHeel" },
			{ 22, "RBigToe" },
			{ 23, "RSmallToe" },
			{ 24, "RHeel" },
			{ 25, "Background" }
		};
		// This array will hold the results parsed from the OSC message string
		ofxJSONElement people;
		void saveJson();
		/* end of json */

		/* draw */
		// Skeleton Tracking part
		vector<vector<string>> bodyPartConnections = {
			{ "Nose", "LEye" },
			{ "LEye", "LEar" },
			{ "Nose", "REye" },
			{ "REye", "REar" },
			{ "Nose", "Neck" },
			{ "Neck", "RShoulder" },
			{ "Neck", "LShoulder" },
			{ "RShoulder", "RElbow" },
			{ "RElbow", "RWrist" },
			{ "LShoulder", "LElbow" },
			{ "LElbow", "LWrist" },
			{ "Neck", "RHip" },
			{ "RHip", "RKnee" },
			{ "RKnee", "RAnkle" },
			{ "Neck", "LHip" },
			{ "LHip", "LKnee" },
			{ "LKnee", "LAnkle" }
		};
		bool isInputImageCropped = true;
		void drawBodyParts();
		void drawBodyPartConnections();
		/* end of draw */
};

#pragma once

#include "ofMain.h"
#include "ofxJSON.h"
#include "ofxNetwork.h"
#include "ofxIO.h"

//==============================

/* app */
#define isLogToConsole true
#define isSilentLog false
#define frameRate 30
/* end of app */

/* cameras and image manipulation */
#define totalCamera 5
#define totalBtn 18
#define TotalTrigger 36
#define btnPerCam 6

#define camDeviceA 0
#define camDeviceB 0
#define camDeviceC 0
#define camDeviceD 0

//#define camDeviceA 1
//#define camDeviceB 2
//#define camDeviceC 4
//#define camDeviceD 0

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
#define imageCroppedUnderData "prcessed_croppedimages/"
#define croppedImageFileSuffix "_cropped"
#define isSaveCroppedImgToFile true

//#define leftBodyPartRef "LHip"
#define leftBodyPartRef "LEye"
//#define rightBodyPartRef "RHip"
#define rightBodyPartRef "REye"
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
//#define hostIp "127.0.0.1"
#define hostIp "10.0.1.10"
#define hostPort 27156
#define tcpMsgDelimiter "[TCP]"
#define reconnectTimeMillis 5000
#define recvWindBufSize 65536
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
        int flaginLoop = 0;
        #define flagMAX 36
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
		int btnToCam[totalBtn] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        // New Triggering To Camera alignment
        int ButtonTriggerToCam[TotalTrigger] = {
            camDeviceA, //STEP NO.1 == Step ON Button #0 | from Button #18
            camDeviceA, //STEP NO.4 == Step OFF Button #0 | to Button #1
            camDeviceA, //STEP NO.3 == Step ON Button #1 |
            camDeviceA, //STEP NO.6 == Step OFF Button #1 |
            camDeviceA, ///STEP NO.5 == Step ON Button #2 |
            camDeviceA, ///STEP NO.8 == Step OFF Button #2 | to Button #1
            camDeviceA, //STEP NO.7 == Step ON Button #3 | from Button #0,
            camDeviceB, //STEP NO.10 == Step OFF Button #3 | to Button #2
            camDeviceA, //STEP NO.9 == Step ON Button #4 | from Button #18
            camDeviceB, //STEP NO.12 == Step OFF Button #4 | to Button #1
            camDeviceB, //STEP NO.11 == Step ON Button #5 | from Button #0,
            camDeviceB, //STEP NO.14 == Step OFF Button #5 | to Button #2
            camDeviceB, //STEP NO.13 == Step ON Button #6 | from Button #18
            camDeviceB, //STEP NO.16 == Step OFF Button #6 | to Button #1
            camDeviceB, //STEP NO.15 ==  Step ON Button #7 | from Button #0,
            camDeviceB, //STEP NO.18 == Step OFF Button #7 | to Button #2
            camDeviceB, //STEP NO.17 == Step ON Button #8 | from Button #18
            camDeviceC, //STEP NO.20 == Step OFF Button #8 | to Button #1
            camDeviceC, //STEP NO.19 ==  Step ON Button #9 | from Button #0,
            camDeviceC, //STEP NO.22 == Step OFF Button #9 | to Button #2
            camDeviceC, //STEP NO.21 == Step ON Button #10 | from Button #18
            camDeviceC, //STEP NO.24 == Step OFF Button #10 | to Button #1
            camDeviceC, //STEP NO.23 ==  Step ON Button #11 | from Button #0,
            camDeviceC, //STEP NO.26 == Step OFF Button #1` | to Button #2
            camDeviceC, //STEP NO.25 == Step ON Button #12 | from Button #18
            camDeviceD, //STEP NO.28 == Step OFF Button #12 | to Button #1
            camDeviceC, //STEP NO.27 ==  Step ON Button #13 | from Button #0,
            camDeviceD, //STEP NO.30 == Step OFF Button #13 | to Button #2
            camDeviceD, //STEP NO.29 == Step ON Button #14 | from Button #18
            camDeviceD, //STEP NO.32 == Step OFF Button #14 | to Button #1
            camDeviceD, //STEP NO.31 ==  Step ON Button #15 | from Button #0,
            camDeviceD, //STEP NO.34 == Step OFF Button #15 | to Button #2
            camDeviceD, //STEP NO.33 == Step ON Button #16 | from Button #18
            camDeviceD, //STEP NO.36 == Step OFF Button #16 | to Button #1
            camDeviceD, //STEP NO.35 ==  Step ON Button #17 | from Button #0,
            camDeviceA, //STEP NO.2 == Step OFF Button #17 | to Button #2
        };
        
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
            void callJsonLocal(int);
            void callLastJsonLocal(int);
		/* end of json */
    
    //new added by hoi9 -- animation loop
    ofFile resultInLoop;
    ofFile resiltLastInLoop;
    ofxJSONElement resultInLoopJSON;
    ofxJSONElement resultLastInLoopJSON;
    ofxJSONElement peopleInLoopJSON;
    ofxJSONElement peopleLastInLoopJSON;
    // * end new added
    

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
			{ "LKnee", "LAnkle" },
            { "LHeel", "LBigToe"},
            { "RHeel", "RBigToe"},
            {"LAnkle","LHeel"},
            {"RAnkle","RHeel"},
            {"Neck","MidHip"},
            {"LHip","MidHip"},
            {"RHip","MidHip"}
		};
    
        void SetupSkeletonColor();
        ofColor skeletonColors[24];
    
		bool isInputImageCropped = true;
		void drawBodyParts();
		void drawBodyPartConnections();
		/* end of draw */
    
        /* draw animation LOOP */
        #define Animating false
        #define AnimateingCroppedPhoto false
        ofImage rawImgInLoop;
        ofImage croppedImgInLoop;
            void drawAnimation();
            void drawAnimationBodyPartLocus();
            void drawAnimationGIF();
                /* END draw animation LOOP */
    
    void AnimateNextFrame();
};

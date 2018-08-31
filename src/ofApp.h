#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxJSON.h"
#include "ofxNetwork.h"

//==============================
#define totalCamera 5
#define totalBtn 18
#define camDeviceA 0
#define camDeviceB 1
#define camDeviceC 4
#define camDeviceD 2

#define camW 1920
#define camH 1080
#define windowW 960
#define windowH 540
#define btnPerCam 6

#define cropW 1920
#define cropH 880

//==============================


#define RECEIVE_PORT 3334
#define NUM_MSG_STRINGS 20


#define HOST "localhost"
#define SEND_PORT 12000


class ofApp : public ofBaseApp{

	private:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
    
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
		int btnToCam[totalBtn] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
		//int btnToCam[totalBtn] = { 0,0,0,0,0,1,1,1,1,3,3,3,3,3,4,4,4,4 };
	    //int btnToCam2[36]={camDeviceD,camDeviceA,camDeviceA,camDeviceA,camDeviceA,camDeviceA,camDeviceA,camDeviceA,camDeviceA,camDeviceB,camDeviceB,camDeviceB,camDeviceB,camDeviceB,camDeviceB,camDeviceB,camDeviceB,camDeviceC,camDeviceC,camDeviceC,camDeviceC,camDeviceC,camDeviceC,camDeviceC,camDeviceC,camDeviceD,camDeviceD,camDeviceD,camDeviceD,camDeviceD,camDeviceD,camDeviceD};
        //int btnToCam[totalBtn] ;
        
        //Dimension array [buttonID] = {startPoint0, StartPoint1,...,startPoint17}
        //btnToStartPoint[] = {0,320,640,960,1280,1600,0,320,640,960,1280,1600,0,320,640,960,1280,1600}
        int btnToStartPoint[totalBtn] = {0,1200,1000,800,600,400,200,320,640,960,1280,1600,0,320,640,960,1280,1600};
        
    
        int leftBorder, rightBorder;
        

    
    
        //OSC reveice and final crop pro8l
        void getHumanFromOSC();

        
        



		/* serial */		
		ofSerial serial;
		int baud = 9600;
		/* end of serial */


		/* TCP networking */
		const string hostIp = "127.0.0.1";
		const int hostPort = 27156;
		const string tcpMsgDelimiter = "[TCP]";
		const int reconnectTimeMillis = 5000;

		ofxTCPClient tcpClient;		
		int connectTime = 0;
		int deltaTime = 0;
		string msgRx = "";
		bool isWaitingForReply = false;

		void setupTcpClient();
		void receiveTcpMsg();
		void processJsonResponse();
		/* end of TCP networking */


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
		ofxJSONElement jsonResults;
		ofxJSONElement people;
		/* end of json */


		/* images */
		ofImage rawPhoto;
		ofImage cropPhoto;
		ofImage optPhoto;
		/* end of images */


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
		void drawBodyParts();
		void drawBodyPartConnections();
		/* end of draw */
};

/********************************************************************
CODE TO SEND OVER POSE DATA THROUGH TCP SOCKET TO ROS
Written by Zonghe Chua 2/15/19
This code is based of the MTC SimpleDemoC application with added TCP
socket functionality. While not ideal, we send the pose from the micron
tracker using strings. This prevents problems with intricacies in byte 
conversion such as "endian-ness" that are processor dependent. Currently
this version of the code only takes input from one marker.
********************************************************************/

#include "string.h"
#include "stdlib.h"
#include <stdio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include "../Dist/MTC.h" //MTC.h need to be in the local directory or include path

#pragma comment(lib, "ws2_32.lib")
#define DEFAULT_PORT "27015" // This port has to be set to correspond to the same port that the receiver is using.
#define SAMPLING_RATE 45
#define NUM_BYTES 5 // This defines number of bytes we send over for each numerical value

//#define DEBUG 
//#define DEBUG_TRACKER

//Macro to check for and report MTC usage errors.
#define MTC(func) {int r = func; if (r!=mtOK) printf("MTC error: %s\n",MTLastErrorString()); };

#ifdef WIN32
int getMTHome (  char *sMTHome, int size ); //Forward declaration
#endif

// Functions and Class declarations
void write(char *, int, double);

class Timer {
public:
	Timer() {
		reset();
	}
	/// reset() makes the timer start over counting from 0.0 seconds.
	void reset() {
		unsigned __int64 pf;
		QueryPerformanceFrequency((LARGE_INTEGER *)&pf);
		freq_ = 1.0 / (double)pf;
		QueryPerformanceCounter((LARGE_INTEGER *)&baseTime_);
	}
	/// seconds() returns the number of seconds (to very high resolution)
	/// elapsed since the timer was last created or reset().
	double seconds() {
		unsigned __int64 val;
		QueryPerformanceCounter((LARGE_INTEGER *)&val);
		return (val - baseTime_) * freq_;
	}
	/// seconds() returns the number of milliseconds (to very high resolution)
	/// elapsed since the timer was last created or reset().
	double milliseconds() {
		return seconds() * 1000.0;
	}
private:
	double freq_;
	unsigned __int64 baseTime_;
};

class tcpSocket {

public:

	int iSendResult;
	int iResult;
	char msg[512];
	int msgbuflen;
	WSAData wsa;
	SOCKET ListenSocket;
	SOCKET ClientSocket;

	bool initialize(PCSTR port_address) {

		ListenSocket = INVALID_SOCKET;
		ClientSocket = INVALID_SOCKET;

		msgbuflen = 512;

		printf("\nInitialising Winsock...");
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		{
			printf("Failed. Error Code : %d", WSAGetLastError());
			return 1;
		}

		printf("Initialised.\n");

		struct addrinfo *result = NULL, *ptr = NULL, hints;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		// Resolve the local address and port to be used by the server
		iResult = getaddrinfo(NULL, port_address, &hints, &result);
		if (iResult != 0) {
			printf("getaddrinfo failed: %d\n", iResult);
			WSACleanup();
			return 1;

		}

		ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

		if (ListenSocket == INVALID_SOCKET) {
			printf("Error at socket(): %ld\n", WSAGetLastError());
			freeaddrinfo(result);
			WSACleanup();
			return 1;
		}

		printf("Socket created.\n");

		// Setup the TCP listening socket
		iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			printf("bind failed with error: %d\n", WSAGetLastError());
			freeaddrinfo(result);
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		freeaddrinfo(result);

		printf("Listening for connections...");

		if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
			printf("Listen failed with error: %ld\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		printf("Client socket connected!");
		closesocket(ListenSocket);
		return 0;

	}

	bool send_msg(char msg[28],int msg_length) {

		iSendResult = send(ClientSocket, msg, msg_length , 0);
		if (iSendResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}
		//printf("Bytes sent: %d\n", iSendResult);

	}

	bool kill_server() {

		iResult = shutdown(ClientSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			printf("shutdown failed: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}

		// cleanup
		closesocket(ClientSocket);
		WSACleanup();

		return 0;

	}
};

// Global Variables
Timer t;
tcpSocket s;
double StartTime = 0;
double ReferenceTime = 0;
double CurrentTime = 0;

double pose[7] = { 0, 0, 0, 0, 0, 0, 0 }; // double variable to read in the output from Micron Tracker
float poseF[7] = { 0, 0, 0, 0, 0, 0, 0 }; // float array that holds casted double from pose array
char poseC[sizeof(poseF)]; // the char array we use to hold byte data to send over tcp


/********************************************************************/
int main(int argc, char* argv[])
/********************************************************************/
{	
	bool exit = false;
	double * angle_ptr = &pose[3]; // define the address at which we store angles


	#if (defined(DEBUG) || defined(DEBUG_TRACKER))
	#else
		s.initialize(DEFAULT_PORT);
	#endif

	Sleep(1000);
	StartTime = t.seconds();


	printf("\n Simple MicronTracker app");
	printf("\n==========================\n");

	//Connect to the available cameras, and report on what was found
	//The first camera is designated as the "current" camera - we will use its coordinate
	//space in reporting pose measurements.
	//char MTHome[512];
	char calibrationDir[512];
	char markerDir[512];
	

	int result = 0;
        int getMTHome (  char *sMTHome, int size );
    	if ( getMTHome (calibrationDir, sizeof(calibrationDir)) < 0 ) {
		// No Environment
		return result;
	} else {
		sprintf(markerDir,"%s/Markers",calibrationDir);
		sprintf(calibrationDir,"%s/CalibrationFiles",calibrationDir);
	}
        
//#ifdef WIN32
//    if ( getMTHome (MTHome, sizeof(MTHome)) < 0 ) {
//		// No Environment
//		printf("MTHome environment variable is not set!\n");
//		return 0;
//	} else {
//		sprintf(calibrationDir,"%s\\CalibrationFiles",MTHome);
//		sprintf(markerDir,"%s\\Markers",MTHome);
//	}
//#else  //Linux & Mac OSX
	//sprintf(calibrationDir,"../../CalibrationFiles");
	//sprintf(markerDir,"../../Markers");
//#endif

	MTC( Cameras_AttachAvailableCameras(calibrationDir) ); //Path to directory where the calibration files are
	if (Cameras_Count() < 1) {
		printf("No camera found!\n");
		return 0;
	}
	mtHandle CurrCamera, IdentifyingCamera;
	int		CurrCameraSerialNum;
	MTC( Cameras_ItemGet(0, &CurrCamera) ); //Obtain a handle to the first/only camera in the array
	MTC( Camera_SerialNumberGet(CurrCamera, &CurrCameraSerialNum) ); //obtain its serial number
	printf("Attached %d camera(s). Curr camera is %d\n", Cameras_Count(), CurrCameraSerialNum);

	int x, y;
	MTC (Camera_ResolutionGet(CurrCamera, &x, &y));
	printf("the camera resolution is %d, x %d", x, y);

	XPoints_SensitivitySet(60);
	XPoints_MisalignmentSensitivitySet(25);
	Markers_JitterFilterEnabledSet(true);
	Markers_SmallerXPFootprintSet(true);
	Markers_KalmanFilterEnabledSet(true);

	bool IsBackGroundProcessingEnabled = true;
	if (IsBackGroundProcessingEnabled) {
		MTC(XPoints_BackGroundProcessSet(true));
		MTC(Markers_BackGroundProcessSet(true));
		printf("Background processing enabled \n");
	}

	//Load the marker templates (with no validation).
	MTC( Markers_LoadTemplates(markerDir) ); //Path to directory where the marker templates are
	printf("Loaded %d marker templates\n",Markers_TemplatesCount());

	//Create objects to receive the measurement results
	mtHandle IdentifiedMarkers = Collection_New();
	mtHandle PoseXf = Xform3D_New();
	int i, j;

	Camera_ShutterMsecsSet(CurrCamera, 19); // Shutter settings under the console
	Camera_GainFSet(CurrCamera, 1.0);

	for (i = 0; i < 20;){ //the first 20 frames are auto-adjustment frames
		CurrentTime = t.seconds();

		if ((CurrentTime - ReferenceTime) > 1.0 / SAMPLING_RATE) { // This sets the sampling rate of the loop

			printf("Fs: %.3f Hz \r", 1 / (CurrentTime - ReferenceTime));
			ReferenceTime = CurrentTime;
			if (IsBackGroundProcessingEnabled) {
				Markers_GetIdentifiedMarkersFromBackgroundThread(CurrCamera);
			}
			else {
				MTC(Cameras_GrabFrame(NULL)); //Grab a frame (all cameras together)
				MTC(Markers_ProcessFrame(NULL)); //Process the frame(s) to obtain measurements
			}

			i++;

		}
	}

	while (exit == false) {

		CurrentTime = t.seconds();

		if ((CurrentTime - ReferenceTime) > 1.0 / SAMPLING_RATE) { // This sets the sampling rate of the loop
		
		//printf("Fs: %.3f Hz \r", 1 / (CurrentTime - ReferenceTime));
		ReferenceTime = CurrentTime;
		
		if (IsBackGroundProcessingEnabled) {
			Markers_GetIdentifiedMarkersFromBackgroundThread(CurrCamera);
		}
		else {
			MTC(Cameras_GrabFrame(NULL)); //Grab a frame (all cameras together)
			MTC(Markers_ProcessFrame(NULL)); //Process the frame(s) to obtain measurements
		}

		/*Here, MTC internally maintains the measurement results.
		Those results can be accessed until the next call to Markers_ProcessFrame, when they 
		are updated to reflect the next frame's content. */

		//First, we will obtain the collection of the markers that were identified.
		MTC( Markers_IdentifiedMarkersGet(NULL,IdentifiedMarkers));
		
		#ifdef DEBUG
			//printf("%d: identified %d marker(s)\n", i, Collection_Count(IdentifiedMarkers));
		#endif

		//Now we iterate on the identified markers (if any), and report their name and their pose
		for (j = 1; j <= Collection_Count(IdentifiedMarkers); j++) {

			// Obtain the marker's handle, and use it to obtain the pose in the current camera's space
			//  using our Xform3D object, PoseXf.
			mtHandle Marker = Collection_Int(IdentifiedMarkers, j);
			MTC(Marker_Marker2CameraXfGet(Marker, CurrCamera, PoseXf, &IdentifyingCamera));

			//We check the IdentifyingCamera output to find out if the pose is, indeed,
			//available in the current camera space. If IdentifyingCamera==0, the current camera's
			//coordinate space is not registered with any of the cameras which actually identified
			//the marker.
			if (IdentifyingCamera != 0) {
				char MarkerName[MT_MAX_STRING_LENGTH];
				//We will also check and report any measurement hazard
				mtMeasurementHazardCode Hazard;
				MTC(Marker_NameGet(Marker, MarkerName, MT_MAX_STRING_LENGTH, 0));
				MTC(Xform3D_ShiftGet(PoseXf, pose));
				MTC(Xform3D_RotQuaternionsGet(PoseXf, angle_ptr));
				MTC(Xform3D_HazardCodeGet(PoseXf, &Hazard));
				
				// Copy our position and pose to our pose array

				// first we have to cast our doubles into floats and store them in the pose_f variable
				for (int i = 0; i++; i < 7)
					poseF[i] = (float)pose[i];

				// copy our representation into chars(bytes) array
				memcpy(poseC, poseF, sizeof(poseF)); 

				#ifdef DEBUG
				printf("cart_pos: \n");
				#endif // DEBUG

				#ifdef DEBUG_TRACKER
				//Print the report
				printf(">> %s at (%0.2f, %0.2f, %0.2f) , (angle:%0.2f, %0.2f, %0.2f, %0.2f), %s  \r", 
					MarkerName,
					pose[0], pose[1], pose[2], 
					pose[3], pose[4], pose[5], pose[6],
					MTHazardCodeString(Hazard));
				#endif
			}
		}

		#if (defined(DEBUG) || defined(DEBUG_TRACKER))
		#else
		exit = s.send_msg(poseC,sizeof(poseC));
		#endif

		}
	}

	//free up all resources taken
	Collection_Free(IdentifiedMarkers);
	Xform3D_Free(PoseXf);
	Cameras_Detach(); //important - otherwise the cameras will continue capturing, locking up this process.
	s.kill_server();
	printf("exiting...");

	return 0;
}

//#ifdef WIN32
/********************************************************************/
int getMTHome (  char *sMTHome, int size )
/********************************************************************/
{
#ifdef _WIN32
    LONG err;
    HKEY key;
    char *mfile = "MTHome";
    DWORD value_type;
    DWORD value_size = size;

    /* Check registry key to determine log file name: */
    if ( (err = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment", 0,
	    KEY_QUERY_VALUE, &key)) != ERROR_SUCCESS ) {
		return(-1);
	}

    if ( RegQueryValueEx( key,
			mfile,
			0,	/* reserved */
			&value_type,
			(unsigned char*)sMTHome,
			&value_size ) != ERROR_SUCCESS || value_size <= 1 ){
		/* size always >1 if exists ('\0' terminator) ? */
		return(-1);
	}
#else
	char *localNamePtr = getenv("MTHome");
	if ( localNamePtr) {
		strncpy(sMTHome, localNamePtr, size-1);
		sMTHome[size] = '\0';
	} else {
		//sprintf(sMTHome,"/Developer/MicronTracker");
		return(-1);
	}
#endif

    return(0);
}



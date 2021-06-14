#pragma once
#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include "opencv2/objdetect/objdetect.hpp"

#include "opencv2/core/cuda.hpp" 
#include <mutex>
#include "SerialPort.h"


class camera
{
public:
	cv::Size resDetect; // res for mutiscale
	// Resize Factors for x pixels and y pixels, if the frame got resized for computition
	double resizeFactor[2];
	camera(bool pcuda, SerialPort* parduino, int pdevice = 0) {
		//overgive arduino objekt
		arduino = parduino;

		//for threading
		shouldRun = true;
			
		//if the frame should be shown
		show = true;
		//First frame
		firstFrame = true;
		//If cuda should be used -> not working
		iscuda = pcuda;
		//device id
		this->device = pdevice;
		//Downscaled resolution for Face detection
		
		resDetect = cv::Size(960, 540);
		std::cout << "Computingres  ";
		std::cout << "width: " << resDetect.width << " height: " << resDetect.height << std::endl;
		//Path of cascades
		cascadePath = "C:/haarcascades/haarcascade_frontalface_alt.xml";
		cascadePathCuda = "C:/opencv4.5.1/opencv/sources/data/haarcascades_cuda/haarcascade_frontalface_default.xml";
		
		loadCascade();

		int apiId = cv::CAP_ANY;
		this->device = device;
		
		//Try to open Camera, exit if it failed
		cam.open(device, apiId);
		 
		if (!cam.isOpened()) {
			std::cout << "Couln't open Camera. Exit." << std::endl;
			exit(1);
		}
		else {
			std::cout << "Camera opened! " << std::endl;
		}
		
		double resizeFactor[2] = { 1,1 };

		setViewingAngle(50, 50);
		
		setMaxResolution();
		readResolution();
		createResizeFactor();
		
	}

	void setShow(bool value) {
		//Setter for show
		show = value;
	}

	int getHeigth() {
		return resolution.height;
	}

	int getWidth() {
		return resolution.width;
	}

	int *getViewAngle() {
		return this->viewAngle;
	}

	
	//Destructor
	~camera() {
		std::cout << "Enterfne camera";
		//close all cv windows
		cv::destroyAllWindows();

	}
	void readImage(cv::Mat& frameOut, int mode) {
		//Mode 0 just restrieves frame and copies to frameOut
		if(mode == 0){
			readImg(frameOut);
		}
		//Mode 1 Copies the last Frame to frameOut
		if (mode == 1) {
			//std::cout << "lastFrame: " << lastFrame.empty() << std::endl;
			if(lastFrame.empty() == true){
				throw std::exception("there is no last Frame");
			}
			lastFrame.copyTo(frameOut);

		}
		//Mode 2 retrieves new Frame, copies it to frameOut and lastFrame
		if (mode == 2) {
			readImg(frameOut);
			frameOut.copyTo(lastFrame);
		}
	}
	
	void readImg(cv::Mat& frameOut) {
		std::lock_guard<std::mutex> guard(camMutex);
		try {
			cam.grab();
			cam.retrieve(frameOut, 0);
		}
		catch (cv::Exception e) {
			std::cout << "Fehler!: " << e.what();
			exit(0);
		}
	}

	
	void detectFaces(cv::Mat& input, std::vector<cv::Rect>& facesExt) {
		//Detect faces from input and stores them into this->faces.
		try {
			cascade.detectMultiScale(input, facesExt, 1.1, 3, 0, cv::Size(30, 30), cv::Size(300, 300));
			
		}
		catch (cv::Exception e) {
			std::cout << e.what() << std::endl;
		}
	}

	void static drawFaces(std::vector<cv::Rect>& faces2draw, cv::Mat& frame) {
		//Draws every rect in faces2draw to frame.
		for (int i = 0; i < faces2draw.size(); i++) {
			cv::rectangle(frame, faces2draw[i], cv::Scalar(255, 255, 255), 2, 4, 0);
		}
	}
	int static getBiggesFace(std::vector<cv::Rect> pfaces) {
		//Calculates Biggesface from this->faces and stores the index to this->biggesFace
		int bigFace = -1;
		float area = 0;
		for (int i = 0; i < pfaces.size(); i++) {
			if (pfaces[i].area() > area) {
				area = pfaces[i].area();
				bigFace = i;
			}

		}
		return bigFace;
	}
	std::chrono::system_clock::rep static time_since_epoch() {
			//For messuring fps. returns time 
			static_assert(
				std::is_integral<std::chrono::system_clock::rep>::value,
				"Repressentation of ticks isn't an integral value."
				);
			auto now = std::chrono::system_clock::now().time_since_epoch();
			return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
			//return now.count();
	}

	void setViewingAngle(int x, int y) {
		viewAngle[0] = x;
		viewAngle[1] = y;
	}

	void moveCam(int x, int y) 
	//Converts the distance in px to an angle 
	{
		double ax, ay; // Angle x and y

		//Convert the pixel to angles
		ax = (1.0* viewAngle[0]/  resolution.width) * x;
		ay = (-1.0 * viewAngle[1] / resolution.height) * y;
		std::cout << "Angle: " << ax << std::endl;

		// Move the Cam 
		arduino->moveR((int)round(ax), (int)round(ay));

	}

	void static getMidFace(cv::Point &midFace, cv::Rect &face) {
		//returns y-Pixel from Facemid
		midFace.y = face.y+0.5*face.height;
		midFace.x = face.x + 0.5 * face.width;

	}
	
	int getDistanceFacemidToMiddle_Y(int x) {
		//resturns distance from x to the height/2
		return x - this->resolution.height / 2;
	 }
	int getDistanceFacemidToMiddle_X(int x) {
		//returns distance from x to width/2
		return x - this->resolution.width / 2;
	}
	
private:
	bool firstFrame;
	bool show;
	bool shouldRun;
	bool iscuda;
	//cam Nr
	int device;

	//Mutex for MUltithreading
	std::mutex  camMutex;//For Cam
	std::mutex proFrameMutex; //Mutex for Frame for Processing
	std::mutex frameMutex;
	//Mat for MUltithreading Processing
	cv::Mat lastFrame;
	cv::Mat proFrame;
	cv::Mat proFrameGS;
	//Local lates frame
	cv::Mat frame;
	cv::Mat frameGS; // Grayscale
	
	//resolution
	cv::Size resolution;
	//cascadeIdentifier setup
	std::string cascadePath;
	std::string cascadePathCuda;
	cv::CascadeClassifier cascade ;
	int bigFace;
	//for moving camera
	int viewAngle[2]; //viewing anlge x, y axis
	//Camera
	cv::VideoCapture cam;

	//Arduino 
	SerialPort* arduino;


	void createResizeFactor() {
		resizeFactor[0] =  (1.0 * resolution.width) / resDetect.width;
		resizeFactor[1] =  (1.0 * resolution.height) / resDetect.height;
	}


	void loadCascade() {
		
		if (iscuda != true) {
			if (!cascade.load(cascadePath)) {
				std::cout << "Couldn't load face_cascade" << std::endl;
				exit(-1);
			}
			else {
				std::cout << "Cascade loaded succesfully" << std::endl;
			}
		}
		else {

		}
	}

	void setMaxResolution() {
		//Sets the Resolution to sth thats way beyond the limit. The cam uses the highest Resolution
		try{
			cam.set(cv::CAP_PROP_FRAME_WIDTH, 100000);
			cam.set(cv::CAP_PROP_FRAME_HEIGHT, 100000);
		}
		catch (cv::Exception e) {
			std::cout << "Fehler!: " << e.what();
		}

	}

	void readResolution() {
		//reads width and heigth of the cam
		resolution = cv::Size((int) cam.get( cv::CAP_PROP_FRAME_WIDTH),(int) cam.get(cv::CAP_PROP_FRAME_HEIGHT));
	}

	

};


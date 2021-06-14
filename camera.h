#pragma once
#include <opencv2/opencv.hpp>
#include <stdlib.h>

class camera
{
public:
	camera(int device) {
		//if the frame should be shown
		show = true;
		
		this->device = device;
		this->cam = cv::VideoCapture(device);
		cv::namedWindow("Webcam", cv::WINDOW_AUTOSIZE);
	}

	void setShow(bool value) {
		//Setter for show
		show = value;
	}

	void showImage() {
		//readimg
		if (readImg()) {
			//show img to window if read was succesfull
			cv::imshow("Webcam", frame);
		}
		
		
		
	}


private:
	bool show;
	//cam Nr
	int device;
	//Local lates frame
	cv::Mat frame = cv::Mat();

	//Camera
	cv::VideoCapture cam;

	bool readImg() {
		if (!cam.isOpened()) {
			std::cerr << "ERROR: Could not open camera" << std::endl;
			return false;
		}
		cam >> frame;
		return true;
	}


};


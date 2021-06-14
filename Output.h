#pragma once
#include "Kamera.cpp"
#include <opencv2/opencv.hpp>
#include <stdlib.h>

struct config
{
	std::string name = "Default";
	bool detectFaces = false;
	bool drawFaces = false;
	bool resize = false;
	bool showFps = true;
	int modus = 0;

};

class output
{
public:
	camera* cam;
	output(std::string pname, camera* pcam, bool pdetectFaces, bool pdrawFaces, bool presize, bool showFps, int pmodus = 0);
	output(camera* pcam);
	output(camera* pcam, config pconfig);
	~output();
	void showWebcam(bool move);
	void setMaxFps(double fps);
	void setModus(int pm);
	void showFrame(bool debug = false);
	void moveCam();

private:
	std::string name;
	double fps;

	//Settings
	bool isdetectFaces;
	bool isdrawFaces;
	bool isresize;
	int modus;
	double maxFps;
	long maxDelay; //Max delay between frames
	bool isShowFps;

	//for fps calculation
	std::chrono::system_clock::rep t1;
	std::chrono::system_clock::rep t2;

	//Needed Mats
	cv::Mat frameGS;
	cv::Mat frameOut;
	cv::Mat frameIn;
	//Index of the bigges Face in faces:vector
	int idxBigFace;
	cv::Point midFace;
	std::vector<cv::Rect> faces;
	int distanceMidfaceToMid[2]; //[0] Distance x | [1] Distance y 

	output();
	void calculateDelay();
	void outputFrame();
};


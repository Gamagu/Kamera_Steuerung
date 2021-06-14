#include "Kamera.cpp"
#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include "Output.h"
     
output::output(std::string pname, camera* pcam, bool pdetectFaces, bool pdrawFaces, bool presize, bool showFps, int pmodus) : output() {

	modus = pmodus;
	name = pname;
	isdetectFaces = pdetectFaces;
	isdrawFaces = pdrawFaces;
	isresize = presize;
	cam = pcam;
	isShowFps = showFps;


}
output::output(camera* pcam) : output() {

	name = "Standart";
	isdetectFaces = false;
	isdrawFaces = false;
	isresize = false;
	cam = pcam;
}
output::output(camera* pcam, config pconfig) : output(){
	cam = pcam;
	modus = pconfig.modus;
	name = pconfig.name;
	isdetectFaces = pconfig.detectFaces;
	isresize = pconfig.resize;
	isShowFps = pconfig.showFps;
	isdrawFaces = pconfig.drawFaces;

}

output::~output() {
	cv::destroyAllWindows();
}


void output::showWebcam(bool move) {
	std::cout << "show webcam started: "<< name << std::endl;
	long deltaT;

	try {
		while (1) {
			t1 = camera::time_since_epoch();// For fps calculation and limiter


			showFrame(false); // output one frame
			if (cv::waitKey(5) >= 0) {
				std::cout << "exit" << std::endl;
				break;
			}

			t2 = camera::time_since_epoch();// For fps calculation and limiter
			deltaT = (t2 - t1);


			//frame limiter
			if (deltaT < maxDelay) {
				std::this_thread::sleep_for(std::chrono::milliseconds(maxDelay - deltaT));

			}

			if (isShowFps) { t2 = camera::time_since_epoch(); fps = 1.0 / ((t2 - t1) * 0.001); }// For fps calculation
		}
	}
	catch (cv::Exception e) {
		std::cout << e.what() << std::endl;
		exit(-1);

	}
}

void output::setMaxFps(double fps) {
	maxFps = fps;
	calculateDelay();
}

void output::setModus(int pm) {
	/*Sets modus of the Cam.

	m <- 0 = Triggers to read camera
	m <- 1 = Copys the last image
	*/
	modus = pm;
}

void output::showFrame(bool debug) {
	//Outputs one frame
	if (debug) {
		std::cout << "start  read image" << std::endl;
	}
	try {
		cam->readImage(frameIn, modus);
	}
	catch (std::exception e) {

		if (debug) { std::cout << "Error while reading Image: " << e.what() << std::endl; }
		return;
	}
	if (debug) {
		std::cout << "image read" << std::endl;
	}
	frameIn.copyTo(frameOut);
	if (this->isdetectFaces) {
		//create grayscale
		cv::cvtColor(frameIn, frameGS, cv::COLOR_RGB2GRAY);
		//resize it if it should
		if (isresize) {
			cv::resize(frameGS, frameGS, cam->resDetect);
		}


		//detect faces
		cam->detectFaces(frameGS, faces);
		
		//scaleup the result to the right frame if it was resized
		if (isresize) {
			for (int i = 0; i < faces.size(); i++) {
				faces[i].height = faces[i].height * cam->resizeFactor[1];
				faces[i].width = faces[i].width * cam->resizeFactor[0];
				faces[i].x = faces[i].x * cam->resizeFactor[0];
				faces[i].y = faces[i].y * cam->resizeFactor[1];
			}
		}
		camera::drawFaces(faces, frameOut);
		//Draw the bigFace
	
		idxBigFace = camera::getBiggesFace(faces);


		//if a big face is found
		if (idxBigFace != -1){
			//store the cordinates(pixels) from the mid
			camera::getMidFace(midFace, faces[idxBigFace]);

			//Store the difference between the mid and middle of the face
			distanceMidfaceToMid[0] = cam->getDistanceFacemidToMiddle_X(midFace.x);
			distanceMidfaceToMid[1] = cam->getDistanceFacemidToMiddle_Y(midFace.y);
			std::cout << "Distance middle tofacemid: " << distanceMidfaceToMid[0] << "|" << distanceMidfaceToMid[1] << std::endl;
			//draw the greatest face
			cv::rectangle(frameOut, faces[idxBigFace], cv::Scalar(255, 0, 0), 2, 4, 0);
			moveCam();
			//Testing purposes
			cv::line(frameOut, midFace, cv::Point(cam->getWidth()/2,cam->getHeigth()/2 ), cv::Scalar(255, 0, 0), 2, 4, 0);
		}

	}

	outputFrame();
}

void output::moveCam()
// Moves the cam depending on distanceMidfacetoMid																																
{
	cam->moveCam(distanceMidfaceToMid[0], distanceMidfaceToMid[1]);
}



output::output() {
	frameGS = cv::Mat();
	frameIn = cv::Mat();
	frameOut = cv::Mat();
	fps = 0;
	maxFps = 64;
	calculateDelay();
}

void output::calculateDelay() {
	//Calculates max delay between frames
	maxDelay = (1 / maxFps) * 1000;
}

void output::outputFrame() {
	try {
		if (isShowFps) {
			cv::putText(frameOut, std::to_string(fps), cv::Point(cam->getWidth() - 90, 100), 20, 1, cv::Scalar(0, 0, 255), 1);
		}
		cv::imshow(name, frameOut);
	}
	catch (cv::Exception e) {
		std::cout << "Could not display the frame: " << e.what();
		exit(-1);
	}
}
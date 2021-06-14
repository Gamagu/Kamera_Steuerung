// Kamera_Steuerung.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//

#include <iostream>
#include <opencv2/opencv.hpp>
#include "Kamera.cpp"
#include "Connection.cpp"
#include "SerialPort.h"
#include <opencv2/core/cuda.hpp>
#include <stdio.h>
#include "Output.h"
using namespace cv;
using namespace std;


void mypause()
{
    std::cout << "Press [Enter] to continue . . .";
    std::cin.get();
}

void test() {
    std::cout << "Test!\n" << std::endl;
    //exit(0);
    
}   


int main()
{
    //test();
    //Start things
    std::cout << "Start!\n" << std::endl;
    //std::printf("OpenCV: %s", cv::getBuildInformation().c_str());

    //Cuda device info
    cv::cuda::DeviceInfo yikes = cv::cuda::DeviceInfo();
    std::cout << "Cuda compatible: " << yikes.isCompatible() << std::endl;

    //init Arduino
    bool isConnected = false;
    bool useStdPort = true;
    std::string port;
    std::string stdPort = "\\\\.\\COM10";
    char data[255];
    SerialPort* arduino = nullptr;
    try {
        //Select Port
        if(!useStdPort){
            SerialPort::getPort(port, 0, 10);
            arduino = new SerialPort(port.c_str());
        }
        else {
            arduino = new SerialPort(stdPort.c_str());
        }
        isConnected = true;
        arduino->getValues(data);
        std::cout << data << std::endl;
        std::cout << "Arduino is connected!" << std::endl;
    }
    catch (IOException e) {
        std::cout << e.what();
        std::cout << "Arduino is not connected!" << std::endl;
        isConnected = false;
    }
    
    arduino->move(90, 80);
    //Init camera
    camera test(false,arduino, 0);
    std::cout << "Widht:" << test.getWidth() << std::endl;
    std::cout << "Heigth:" << test.getHeigth() << std::endl;
   
    //init outputs
    config con1;
    con1.detectFaces = true;
    con1.drawFaces = true;
    con1.modus = 2;
    con1.name = "Processing";
    con1.resize = true;
    con1.showFps = true;

    config con2;
    con2.name = "Defaul Output";
    con2.modus = 2;



    output output1(&test,con1);
    output output2(&test,con2);

    output1.setMaxFps(10);
    //output2.showWebcam();

    //start Webcams
    
    std::thread webcamThread(&output::showWebcam, &output2, false);
    std::thread webcamThread1(&output::showWebcam, &output1, true);
    
    //std::thread webcamThread(&output::showWebcam, output1);

    //wait until both threads are finished
    webcamThread.join();
    webcamThread1.join();




    std::cout << "End!\n" << std::endl;

}



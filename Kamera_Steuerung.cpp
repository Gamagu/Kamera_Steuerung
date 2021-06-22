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
    // Test funktion for random stuff
    std::cout << "Test!\n" << std::endl;
    //exit(0);  //Just for testing
    
}   


int main()
{
    //test();
    // 
    //Start things
    std::cout << "Start!\n" << std::endl;
    //Cuda device info
    //cv::cuda::DeviceInfo yikes = cv::cuda::DeviceInfo();
    //std::cout << "Cuda compatible: " << yikes.isCompatible() << std::endl;

    //init Arduino
    bool isConnected = false; 
    bool useStdPort = true; // faster than first seaching for a port -> testing
    std::string port; 
    std::string stdPort = "\\\\.\\COM10"; // change if it's plugged in another port
    char data[255]; //buffer
    SerialPort* arduino = nullptr; 


    try {
        //Select Port
        if(!useStdPort){
            SerialPort::getPort(port, 0, 10); // find out port
            arduino = new SerialPort(port.c_str());
        }
        else {
            //use stdPort
            arduino = new SerialPort(stdPort.c_str());
        }

        isConnected = true;

        //try out if its rly working
        arduino->getValues(data);
        std::cout << data << std::endl;
        std::cout << "Arduino is connected!" << std::endl;
    }
    catch (IOException e) {
        std::cout << e.what();
        std::cout << "Arduino is not connected!" << std::endl;
        isConnected = false;
    }
    
    //Move to start position. Range between 0°-180°
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


    // create two outputs
    output output1(&test,con1); // con1 for facedetection and camera movement
    output output2(&test,con2); // just video output

    output1.setMaxFps(10); //fps cap

    //start Webcams
    

    //start Both in their own thread
    std::thread webcamThread(&output::showWebcam, &output2, false);
    std::thread webcamThread1(&output::showWebcam, &output1, true);
   
    //wait until both threads are finished
    webcamThread.join();
    webcamThread1.join();

    std::cout << "End!\n" << std::endl;

}



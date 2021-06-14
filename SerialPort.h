/*
* Author: Manash Kumar Mandal
* Modified Library introduced in Arduino Playground which does not work
* This works perfectly
* LICENSE: MIT
*/

#pragma once

#define ARDUINO_WAIT_TIME 2000
#define MAX_DATA_LENGTH 255

#include <windows.h>
#include <iostream>
#include <stdio.h>
class SerialPort
{
private:
    HANDLE handler;
    bool connected;
    COMSTAT status;
    DWORD errors;
    void controllData(int& x, int& y);
    int x, y;
    void decodeData(char data[], int& x, int& y);
public:
    explicit SerialPort(const char* portName);
    ~SerialPort();
    int getValues(char* buffer);
    bool writeData(int x, int y);
    int getData(char* buffer );
    int readSerialPort(const char* buffer, unsigned int buf_size);
    bool writeSerialPort(const char* buffer, unsigned int buf_size);
    bool isConnected();
    void closeSerial();
    static bool getPort(std::string &buffer, int start, int end);
    void moveR(int x, int y);
    void move(int x, int y);
    friend std::ostream& operator<<(std::ostream&, const SerialPort& port);
    bool clear();

}; 

class IOException : public std::exception
{
public:
    IOException(const char* msg) ;
};





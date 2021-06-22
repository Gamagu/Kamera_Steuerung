/*
* Author: Manash Kumar Mandal
* Modified Library introduced in Arduino Playground which does not work
* This works perfectly
*/
// I modified his version to work with my arduino code
#include "SerialPort.h"
#include <stdio.h>
#include <thread>

SerialPort::SerialPort(const char* portName)
{
    x = 90;
    y = 90;
    this->connected = false;

    this->handler = CreateFileA(static_cast<LPCSTR>(portName),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (this->handler == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            std::cerr << "ERROR: Handle was not attached.Reason : " << portName << " not available\n";
        }
        else
        {
            std::cerr << "ERROR!!!\n";
        }
    }
    else
    {
        //basic settings
        DCB dcbSerialParameters = { 0 };

        if (!GetCommState(this->handler, &dcbSerialParameters))
        {
            std::cerr << "Failed to get current serial parameters\n";
        }
        else
        {
            dcbSerialParameters.BaudRate = CBR_115200;
            dcbSerialParameters.ByteSize = 8;
            dcbSerialParameters.StopBits = ONESTOPBIT;
            dcbSerialParameters.Parity = NOPARITY;
            dcbSerialParameters.fDtrControl = DTR_CONTROL_ENABLE;

            if (!SetCommState(handler, &dcbSerialParameters))
            {
                std::cout << "ALERT: could not set serial port parameters\n";
            }
            else
            {
                this->connected = true;
                PurgeComm(this->handler, PURGE_RXCLEAR | PURGE_TXCLEAR);
                Sleep(ARDUINO_WAIT_TIME);
            }

        }
    
        //Set timeouts
        COMMTIMEOUTS timeouts = { 0 };

        if (!GetCommTimeouts(this->handler, &timeouts))
        {
            printf("error in get timeout\n\n");
            

        }
        else {
            timeouts.ReadIntervalTimeout         = 10; // in milliseconds
            timeouts.ReadTotalTimeoutConstant = 10;   //MAXDWORD; // in milliseconds
            timeouts.ReadTotalTimeoutMultiplier  = 10; // in milliseconds
            timeouts.WriteTotalTimeoutConstant   = 50; // in milliseconds
            timeouts.WriteTotalTimeoutMultiplier = 10; // in milliseconds
        }

        if (!SetCommTimeouts(this->handler, &timeouts))
        {
            printf("Error! in Setting Time Outs\n\n");
        }
        else{
            this->connected = true;
            PurgeComm(this->handler, PURGE_RXCLEAR | PURGE_TXCLEAR);
            Sleep(ARDUINO_WAIT_TIME);
        }
    }
}

SerialPort::~SerialPort()
{
    if (this->connected)
    {
        this->connected = false;
        CloseHandle(this->handler);
    }
}




int SerialPort::readSerialPort(const char* buffer, unsigned int buf_size)
{
    // Reading bytes from serial port to buffer;
    // returns read bytes count, or if error occurs, returns 0
    DWORD bytesRead{};
    unsigned int toRead = 0;

    //GetLastError
    if (!ClearCommError(this->handler, &this->errors, &this->status)) {
        std::cout << "CleaComError failed" << std::endl;
    }
   

    if (this->status.cbInQue > 0)
    {
        if (this->status.cbInQue > buf_size)
        {
            toRead = buf_size;
        }
        else
        {
            toRead = this->status.cbInQue;
        }
    }

    memset((void*)buffer, 0, buf_size);
    std::cout << "to read:" << toRead << std::endl;
    if (ReadFile(this->handler, (void*)buffer, toRead, &bytesRead, NULL))
    {
        std::cout << "read bytes:" <<bytesRead << std::endl;
        return bytesRead;
    }

    return 0;
}


bool SerialPort::writeSerialPort(const char* buffer, unsigned int buf_size)
{   
    // Sending provided buffer to serial port;
    // returns true if succeed, false if not
    DWORD bytesSend;

    if (!WriteFile(this->handler, (void*)buffer, buf_size, &bytesSend, 0))
    {
        
        ClearCommError(this->handler, &this->errors, &this->status);
        return false;
    }
    std::cout <<"written:"<< "|" << buffer << "|" << std::endl;
    return true;
}


bool SerialPort::isConnected()
{
       // Checking if serial port is connected
    if (!ClearCommError(this->handler, &this->errors, &this->status))
    {
        this->connected = false;
    }

    return this->connected;
}

void SerialPort::closeSerial()
{
    CloseHandle(this->handler);
}

bool SerialPort::writeData(int x, int y) {
    bool succses;
    controllData(x, y); // limit range only from 10 to 170
    this->x = x;
    this->y = y;
    char buffer[MAX_DATA_LENGTH];
    char in[MAX_DATA_LENGTH] = "";
    //Creates Sting to send for arduino
    sprintf_s(buffer, "%d:%d\n", x, y);
    
    succses =  this->writeSerialPort(buffer, MAX_DATA_LENGTH);
    std::this_thread::sleep_for(std::chrono::milliseconds(75));
    if (succses) {
        // TODO: Fix error: the recieved(?) string is twice in chararray 'in'
        this->getData(in);
        decodeData(in, this->x, this->y);
    }
    return succses;
}

int SerialPort::getData(char* buffer) {
    return this->readSerialPort(buffer, MAX_DATA_LENGTH);
}

int SerialPort::getValues(char* buffer) {
    bool iswritten;
    bool result;
    iswritten = writeSerialPort("get\n", MAX_DATA_LENGTH);
    
    //Error handling. If i was unable to write data, nothing could come back
    if (iswritten == false) {
        throw IOException("Failed to write, while getValues!");
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(75));
    result = this->readSerialPort(buffer, MAX_DATA_LENGTH);
    std::cout << "Bytes read: " << result << std::endl;
    return result;
}

void SerialPort::controllData(int& x, int& y) {
    //Checks if the values are valid 
    //10< x,y < 170
    if (x < 10) { x = 10; }
    if (x > 170) { x = 170; }
    if (y < 10) { y = 10; }
    if (y > 170) { y = 170; }
}

void SerialPort::decodeData(char data[], int &x, int &y)
{
    //Example: "X:90Y:90" should set x to 90 and y to 90

    int idx[3];
    std::string helpString;
    std::string helpstring1;
    int yeet;
    helpString = data;

    //Get  ":" indexes
    idx[0] = helpString.find_first_of(':');
    idx[1] = helpString.find_first_of(':', idx[0]+1);
    idx[2] = helpString.find_first_of('\n');
    //x equals string from idx[0] to idx[1]-1 cuz there is the char 'y'.
    //same for y
    yeet = idx[1] - idx[0] - static_cast <unsigned __int64>(2);
    helpstring1 = helpString.substr(idx[0]+1, yeet);
    x = std::stoi(helpstring1);
    yeet = idx[2] - idx[1]-1; //Space between ':' and escapesequence '\n'
    y = std::stoi(helpString.substr(idx[1]+1, yeet ));

}

bool SerialPort::getPort(std::string &buffer,int start, int end ) {
    //Just trys SerialPort::getData, if it returns sth and does not fail, it returns a working port as a String under Windows.
    
    //Standartportpath for windows
    char portBase[] = "\\\\.\\COM";
    
    std::string testString;
    char helpBuffer[255];
    SerialPort* test;
    for (int i = start; i < end; i++) {

        //helpbuffer : char <- portbase : str + i : int
        testString = portBase + std::to_string(i);
        std::cout << "Portname: " << testString << std::endl;
        strcpy_s(helpBuffer, testString.c_str());

        //Init Port
        test = new SerialPort(helpBuffer);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        //if there is a connection
        if (test->isConnected() == false) {
            test->closeSerial();
            continue;
        }
        //if i could write the get command 
        try {
            //if 0 bytes are read continue
            if (test->getValues(helpBuffer) == 0) {
                test->closeSerial();
                continue;
            }
            else {
                //if the SerialConnection return sth to get command return the string of the portName
                buffer = testString;
                test->closeSerial();
                
                
                return true;
            }
        }
        catch (IOException e) {
            //try next if writing failed
            test->closeSerial();
            continue;
        }
        
    }
    return false;


}
void SerialPort::moveR(int x, int y) {
    //moves the cam to relative angles to where the cam was.
    this->move(this->x + x, this->y + y);
}

void SerialPort::move(int x, int y) {
    //Moves the cam to static cords.
    this->writeData(x, y);
}

bool SerialPort::clear()
{
    PurgeComm(this->handler, PURGE_RXCLEAR | PURGE_TXCLEAR);
    ClearCommError(this->handler, &this->errors, &this->status);
    return false;
}


//Controctor for a Exception depending to the IO to arduino
IOException::IOException(const char* msg) : std::exception(msg)
{

}

std::ostream& operator<<(std::ostream& os, const SerialPort& port)
{
    // TODO: insert return statement here
    os << "x:" << port.x << "y:" << port.y;
    return os;
}

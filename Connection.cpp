#include "SerialPort.h"
namespace serial{
class Connection : SerialPort
{
public:
	Connection(char* portName) : SerialPort(portName)
	{
		//just calls the SerialPort instructor
	}

	void writeData(int x, int y) {
		controllData(x, y);
		char buffer[50];
		sprintf_s(buffer, "%d:%d\n", x, y);
		std::cout << buffer << std::endl;

	}


private:

	void controllData(int& x, int& y) {
		//x and y should be in between 10 and 170 degrees;
		if (x < 10) { x = 10; }
		if (x > 170) { x = 170; }
		if (y < 10) { y = 10; }
		if (y > 170) { y = 170; }
	}


}; 
}
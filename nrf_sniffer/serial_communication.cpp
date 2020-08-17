#include "serial_communication.h"


void comInit(HANDLE hComm) {
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    // retrieve current serial settings
    BOOLEAN status = GetCommState(hComm, &dcbSerialParams);

    // set values for Baud rate, byte size, number of start/stop bits
    // Arduino: 8 data bits, no parity, 1 stop bit
    dcbSerialParams.BaudRate = BAUD_RATE;   // setting BaudRate = 115200
    dcbSerialParams.ByteSize = 8;           // setting ByteSize = 8
    dcbSerialParams.StopBits = ONESTOPBIT;  // setting StopBits = 1
    dcbSerialParams.Parity = NOPARITY;      // setting Partiy = None

    SetCommState(hComm, &dcbSerialParams);
}

void setTimeouts(HANDLE hComm) {
    COMMTIMEOUTS timeouts = {0};
    
    // values below are in milliseconds
    // total timeouts are not used by read operations
    // log: ReadTotalTimeoutMultiplier changed from 0 to 32
    // log: ReadTotalTimeoutConstant changed from 0 to 1000
    timeouts.ReadIntervalTimeout = 1000;        // specifies the maximum time interval between arrival of two bytes 
    timeouts.ReadTotalTimeoutConstant = 1000;      // used to calculate the total time-out period for read operations, constant part
    timeouts.ReadTotalTimeoutMultiplier = 32;    // used to calculate the total time-out period for read operations, muliplied by requested number of bytes to read
    timeouts.WriteTotalTimeoutConstant = 50;    // similar to ReadTotalTimeoutConstant, but for write
    timeouts.WriteTotalTimeoutMultiplier = 10;  // similar to ReadTotalTimeoutMultiplier, but for write

    SetCommTimeouts(hComm, &timeouts);
}

void serialWrite(HANDLE hComm) {
    int status = 0;
    char toSend = 0x31;                         // '1'
    DWORD bytesToWrite = sizeof(toSend);
    DWORD bytesWritten = 0;

    status = WriteFile(hComm, &toSend, bytesToWrite, &bytesWritten, NULL);

    if (status != 0) {
        cout << "Write to serial port successful" << endl;
    } else {
        cout << "Failed to write to serial port" << endl;
    }
}

void serialWriteInteger(HANDLE hComm, int8_t frequency) {
    char* buffer = int_to_charArray(frequency);
    size_t size = numberOfDigit(frequency);

    int status = 0;
    DWORD bytesToWrite = sizeof(frequency);
    DWORD bytesWritten = 0;

    status = WriteFile(hComm, buffer, size + 1, &bytesWritten, NULL);

    if (status != 0) {
        cout << "Write to serial port successful" << endl;
    } else {
        cout << "Failed to write to serial port" << endl;
    }
}

char* int_to_charArray(int8_t number) {
    int noDigit = numberOfDigit(number);

    static char* ret = (char *)malloc(noDigit + 1);     // +1 for new line character
    itoa(number, ret, 10);
    ret[noDigit] = '\n';                                // change '\0' to '\n'
    return ret;
}

size_t numberOfDigit(int8_t num) {
    size_t ret = 0;
    
    while (num != 0) {
        num = num / 10;
        ret++;
    }

    return ret;
}

// change: added append option
void readSerialWriteToFile(HANDLE hComm, string fileName) {
    char tempChar;
    DWORD noBytesRead;

    // txt file for output
    std::fstream file (fileName, std::fstream::out | std::fstream::app);

    do {
        // handle of the serial port, temporary character, size of tempChar, number of bytes read
        ReadFile(hComm, &tempChar, sizeof(tempChar), &noBytesRead, NULL);
        if (tempChar != 0) {
            file << tempChar;
        }
    } while (noBytesRead > 0);

    file.close();
}

// this method is copied from stackoverflow
bool selectComPort() {  //added function to find the present serial 
    char lpTargetPath[5000]; // buffer to store the path of the COMPORTS
    bool gotPort = false; // in case the port is not found

    for (int i = 0; i < 255; i++) // checking ports from COM0 to COM255
    {
        string str = "COM" + std::to_string(i); // converting to COM0, COM1, COM2
        DWORD test = QueryDosDevice(str.c_str(), lpTargetPath, 5000);

        // Test the return value and error if any
        if (test != 0) //QueryDosDevice returns zero if it didn't find an object
        {
            std::cout << str << ": " << lpTargetPath << std::endl;
            gotPort = true;
        }

        if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
        }
    }

    return gotPort;
}
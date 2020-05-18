#include <windows.h>
#include <stdio.h>

#define COM_PORT            "\\\\.\\COM10"
#define BAUD_RATE           CBR_115200

// function prototyes
void comInit(HANDLE hComm);
void setTimeouts(HANDLE hComm);
void serialWrite(HANDLE hComm);
void readSerial(HANDLE hComm);

int main() {
    HANDLE hComm;
    char comPort[] = COM_PORT;

    // port name, read/write, no sharing, no security, open existing port only, non overlapped io, null for com devices
    hComm = CreateFile(comPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (hComm == INVALID_HANDLE_VALUE) {
        printf("Error in opening serial port\n");
    } else {
        printf("Opening serial port successful\n");
    }

    comInit(hComm);
    setTimeouts(hComm);
    serialWrite(hComm);
    printf("Reading from serial port.\n");
    readSerial(hComm);
    CloseHandle(hComm);
    printf("Serial port closed.\n");

    return 0;
}

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
    timeouts.ReadIntervalTimeout = 1000;        // specifies the maximum time interval between arrival of two bytes 
    timeouts.ReadTotalTimeoutConstant = 0;      // used to calculate the total time-out period for read operations, constant part
    timeouts.ReadTotalTimeoutMultiplier = 0;    // used to calculate the total time-out period for read operations, muliplied by requested number of bytes to read
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
        printf("Write to serial port successful!\n");
        printf("Scan initiated\n");
    }
}

void readSerial(HANDLE hComm) {
    char tempChar;
    DWORD noBytesRead;

    // txt file for output
    FILE *fp;
    fp = fopen("serial.txt", "w");

    do {
        // handle of the serial port, temporary character, size of tempChar, number of bytes read
        ReadFile(hComm, &tempChar, sizeof(tempChar), &noBytesRead, NULL);
        fputc(tempChar, fp);
    } while (noBytesRead > 0);

    fclose(fp);
}

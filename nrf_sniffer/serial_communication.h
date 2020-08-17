#ifndef SERIAL_COMMUNICATION_H
#define SERIAL_COMMUNICATION_H

#include <windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdint>

using std::cout;
using std::cin;
using std::endl;
using std::string;

#define COM_PORT            "\\\\.\\COM10"
#define BAUD_RATE           CBR_115200

// function prototyes
void comInit(HANDLE hComm);
void setTimeouts(HANDLE hComm);
void serialWrite(HANDLE hComm);
void serialWriteInteger(HANDLE hComm, int8_t frequency);
void readSerialWriteToFile(HANDLE hComm, string fileName);
bool selectComPort();
std::wstring string_to_wstring(const std::string &text);
char* int_to_charArray(int8_t number);
size_t numberOfDigit(int8_t num);

#endif
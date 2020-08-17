#ifndef     PACKET_PROCESSOR_H
#define     PACKET_PROCESSOR_H

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "debug.h"
using namespace std;

// function prototypes
std::string findOccurrence(std::vector<std::string> &address, int channel);
void exportMapValues(std::unordered_map<std::string, int> &m);
bool checkIfValid(std::pair<string, int> &max1, std::pair<string, int> &max2, std::pair<string, int> &max3);
void checkID(std::string address, std::unordered_set<string> &candidate);
void addToBlock(std::string address);

#endif
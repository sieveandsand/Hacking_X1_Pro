#ifndef _INLCUDE_GUARD_
#define _INCLUDE_GUARD_

#include <string>
#include <string.h>
#include <unistd.h>
#include <unordered_set>
#include "Frequency.h"
#include "serial_communication.h"
#include "packet_processor.h"


// function prototypes
static void show_usage();
void getFrequency(char*, int&, int&);
void printInfo(const char*, int&, int&, int&, int&);
void findAllOccurringChannels(Frequency &radio, std::string &address, std::unordered_set<int> &candidateChannel);

#endif
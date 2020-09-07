// To find the target address, I'm guessing:
// 1. Occurrence of correct address have to be higher or equal to 5% of all unique addresses
// 2. False positives have very similar addresses (i.e. 53A0ACFAAE and 53A0ACFAB6)

// the channel hopping algorithm of the remote depends on how much
// traffic is in the air. if there is no traffic (i.e. midnight), 
// it will stay on channel 52

// what we know:
// when forward
// 0x0000 mode 1
// 0x0080 mode 2
// 0x0100 mode 3
// 0x0180 mode 4

// when backward 
// 0x0400 mode 1
// 0x0480 mode 2
// 0x0500 mode 3
// 0x0580 mode 4


// update:
// pretty sure the first two bytes of the address are used as identification number
// they are the same across two controllers, both start with 3932
// ie. the first controller has the address 393271107A
// the second controller has the address 39326C1877


#include "include.h"
using namespace std;

// global variable
int numAddress;
std::vector<string> blocklist;

int main(int argc, char* argv[]) {
    char c;
    char* cvalue = NULL;

    // flags
    int sflag = 0;

    // frequencies & increment
    char* frequency = NULL;
    int start_frequency;
    int end_frequency;
    int increment;

    // com port
    char* com;

    if (argc < 2) {
        show_usage();
        return 1;
    }

    while ((c = getopt(argc, argv, "sf:i:c:")) != -1) {
        switch(c) {
            case 's':
                sflag = 1;
                break;
            case 'f':
                frequency = optarg;
                break;
            case 'i':
                increment = atoi(optarg);
                break;
            case 'c':
                com = optarg;
                break;
            case '?':
                show_usage();
                return 1;
            default:
                abort();
        }

    }

    // get the start and end frequency
    getFrequency(frequency, start_frequency, end_frequency);

    // Frequency object(start frequency, end frequency, increment)
    Frequency radio (start_frequency, end_frequency, increment);

    
    

    /*
    --------------------------------------------------------------
        Start of Serial Communication
    --------------------------------------------------------------
    */

   if (sflag) {
        printInfo(com, start_frequency, end_frequency, increment, radio.num_channel);

        string portNumber = com;
        HANDLE hComm;
        string comPort = "\\\\.\\COM" + portNumber;

        // port name, read/write, no sharing, no security, open existing port only, non overlapped io, null for com devices
        hComm = CreateFile(comPort.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

        if (hComm == INVALID_HANDLE_VALUE) {
            cout << "Error in opening serial port" << endl;
        } else {
            cout << "Opening serial port successful" << endl;
        }

        comInit(hComm);
        setTimeouts(hComm);

        system("rmdir /q /s scans");
        system("mkdir scans");

        for (int c = 0; c < 4; c++) {
            while (radio.hasNext()) {
                uint8_t i = radio.next();
                serialWriteInteger(hComm, i);
                string fileName = "./scans/" + std::to_string(i) + ".txt";
                cout << "Scanning channel " << std::to_string(i) << endl;
                readSerialWriteToFile(hComm, fileName);
                cout << "Finished scanning channel " << std::to_string(i) << endl;
            }
            radio.reset();
        }

        CloseHandle(hComm);
        cout << "Serial Port Closed" << endl;
   }
    

    /*
    ----------------------------------------------------------------
        End of Serial Communication
    ----------------------------------------------------------------
    */


    /*
    ----------------------------------------------------------------
        Start of Packet Processing
    ----------------------------------------------------------------
    */

    // initialize set
    unordered_set<string> candidateAddress;
    unordered_set<int> candidateChannel;

    string validAddress;

    // configure blocklist
    addToBlock("5555555555");
    addToBlock("AAAAAAAAAA");


    radio.reset();
    while (radio.hasNext()) {
        int i = radio.next();
        // reads input file
        string fileName = "./scans/" + to_string(i) + ".txt";
        ifstream file(fileName);
        string line;
        vector<string> address;

        while (getline(file, line)) {
            string temp = line.substr(0, line.find(" "));
            if (temp.size() == 10) {            // 10 characters --> 5 byte address
                address.push_back(temp);
            }
        } 

        file.close();

        validAddress = findOccurrence(address, i);
        cout << i << endl;
        cout << validAddress << endl;
        checkID(validAddress, candidateAddress);
    }

    // finds all channels that the target address has occurred
    unordered_set<string>::iterator it;
    for (it = candidateAddress.begin(); it != candidateAddress.end(); ++it) {
        string s = *it;
        findAllOccurringChannels(radio, s, candidateChannel);
    }

    unordered_set<int>::iterator it1;
    for (it1 = candidateChannel.begin(); it1 != candidateChannel.end(); ++it1) {
        cout << *it1 << endl;
    }

    unordered_set<string>::iterator it2;
    for (it2 = candidateAddress.begin(); it2 != candidateAddress.end(); ++it2) {
        string temp = *it2;
        cout << temp << endl;
    }

    /*
    ----------------------------------------------------------------
        End of Packet Processing
    ----------------------------------------------------------------
    */

    
    return 0;
}

static void show_usage() {
    cout << "usage message here" << endl;
}

void getFrequency(char* frequency, int& start_frequency, int& end_frequency) {
    // process start and end frequency
    char* middle = strchr(frequency, ',');
    if (middle != NULL) {
        end_frequency = atoi(middle + 1);
        char* start_string = (char *)malloc(middle - frequency + 1);
        memcpy(start_string, frequency, middle - frequency);
        start_string[middle - frequency] = '\0';
        start_frequency = atoi(start_string);
    }
}

void findAllOccurringChannels(Frequency &radio, string &address, unordered_set<int> &candidateChannel) {
    radio.reset();
    while(radio.hasNext()) {
        int i = radio.next();
        // reads input file
        string fileName = "./scans/" + to_string(i) + ".txt";
        ifstream file(fileName);
        string line;

        bool flag = false;  // go to the next file if current file contains target
        while (getline(file, line) && !flag) {
            const string temp = line.substr(0, line.find(" "));
            int num = temp.compare(address);
            if (num == 0) {
                candidateChannel.insert(i);
                flag = true;
            }
        }
    }
}

void printInfo(const char* com, int& start_frequency, int& end_frequency, int& increment, int& numChannel) {
    cout << "--------------------------------------------" << endl;
    cout << "Scanning on COM port " << com << endl;
    cout << "Scanning frequency from " << start_frequency << " MHz to " << end_frequency << " MHz" << endl;
    cout << "Scanning with channel width set to " << increment << " MHz" << endl;
    cout << "Will need to scan " << numChannel << " channels" << endl;
    cout << "--------------------------------------------" << endl;
}
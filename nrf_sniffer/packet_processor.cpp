#include "packet_processor.h"

extern int numAddress;
extern std::vector<string> blocklist;      // addresses to be explicitly blocked


// finds the address with the highest occurrence
// returns string address that is of the highest occurrence
string findOccurrence(vector<string> &address, int channel) {
    unordered_map<string, int> m;

    // count occurrence of every string
    for (int i = 0; i < address.size(); i++) {
        unordered_map<string, int>::iterator it = m.find(address[i]);

        if (it == m.end()) {
            m.insert(pair<string, int>(address[i], 1));
        } else {
            m[address[i]]++;
        }
    }

    numAddress = m.size();

    // find the max
    pair<string, int> max1 (string(), 0);
    for (unordered_map<string, int>::iterator it2 = m.begin(); it2 != m.end(); ++it2) {
        if (it2 -> second > max1.second) {
            max1 = *it2;
        }
    }

    // find the second to max
    pair<string, int> max2 (string(), 0);
    for (unordered_map<string, int>::iterator it4 = m.begin(); it4 != m.end(); ++it4) {
        if ((it4 -> second < max1.second) && (it4 -> second > max2.second)) {
            max2 = *it4;
        }
    }

    // find the third to max
    pair<string, int> max3 (string(), 0);
    for (unordered_map<string, int>::iterator it6 = m.begin(); it6 != m.end(); ++it6) {
        if ((it6 -> second < max2.second) && (it6 -> second > max3.second)) {
            max3 = *it6;
        }
    }

    // prints debug information
    #ifdef DEBUG
        cout << "Address with the highest occurrence is: " + it1 -> first << endl;
        cout << to_string(it1 -> second) + " occurrences out of " + to_string(m.size()) + 
                " unique addresses" << endl;
        cout << "Address with the second highest occurrence is: " + it3 -> first << endl;
        cout << to_string(it3 -> second) + " occurrences out of " + to_string(m.size()) + 
                " unique addresses" << endl;
        cout << "Address with the third highest occurrence is: " + it5 -> first << endl;
        cout << to_string(it5 -> second) + " occurrences out of " + to_string(m.size()) + 
                " unique addresses" << endl;
    #endif

    if (checkIfValid(max1, max2, max3)) {
        return max1.first;
    } else {
        return std::string();
    }
}


bool checkIfValid(pair<string, int> &max1, pair<string, int> &max2, pair<string, int> &max3) {
    // initialize bool values
    bool check1 = false;
    bool check2 = false;
    bool check3 = true;

    cout.precision(3);
    float percent = (float)max1.second / (float)numAddress;

    // Guess 1
    // target has occurrence higher than 5%
    if (percent >= 0.05) {
        check1 = true;
    }

    // Guess 2
    // check if first and second have similar address
    string addressFirst = max1.first;
    string addressSecond = max2.first;
    if (addressFirst.compare(0, 5, addressSecond.substr(0, 5)) != 0) {
        check2 = true;
    }

    // Check 1
    // check if address is on blocklist;
    vector<string>::iterator it_block;;
    for (it_block = blocklist.begin(); it_block != blocklist.end(); ++it_block) {
        // if the top address matches the blocklist
        if (*it_block == max1.first) {
            check3 = false;
        }
    }

    return check1 && check2 && check3;
}

// check if the first two most significant two bytes matches the ID
void checkID(string address, unordered_set<string> &candidate) {
    // only add to the set if address is not empty
    if (!address.empty()) {
        // checks if ID matches
        const string segment = address.substr(0,4);
        // magic id that occurs in valid controller address
        const string magic_ID = "3932";

        if (segment.compare(magic_ID) == 0) {
            candidate.insert(address);
        }
    }
}


// exports number of frequency for each address to a txt file
void exportMapValues(unordered_map<string, int> &m) {
    // write to a new file
    ofstream file("frequency.txt");

    file << "frequency\n";

    unordered_map<string, int>::iterator it = m.begin();
    for (it; it != m.end(); ++it) {
        file << to_string(it -> second) << endl; 
    }

    file.close();
}

void addToBlock(string address) {
    blocklist.push_back(address);
}
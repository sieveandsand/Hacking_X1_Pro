// To find the target address, I'm guessing:
// 1. Occurrence of correct address have to be higher or equal to 5% of all unique addresses
// 2. False positives have very similar addresses (i.e. 53A0ACFAAE and 53A0ACFAB6)
// could be wrong --> 3. False positives with similar addresses have very close occurrences



#include "include.h"
using namespace std;

// global variable
int numAddress;

int main(int argc, char* argv[]) {
    // reads input file
    ifstream file("serial.txt");
    string line;
    vector<string> address;
    int index = 0;

    while (getline(file, line)) {
        string temp = line.substr(0, line.find(" "));
        if (temp.size() == 10) {
            address.push_back(temp);
        }
        index++;
    } 

    file.close();

    // print the address vector
    for (int i = 0; i < address.size(); i++) {
        cout << address[i] << endl;
    }
    cout << endl;

    map<string, int> m = findOccurrence(address);
    // exportMapValues(m);

    Frequency exway;

    while(exway.hasNext()) {
        
    }
    

    return 0;
}

// finds the address with the highest occurrence
// returns a map with address as the key and number of occurrence as value
map<string, int> findOccurrence(vector<string> &address) {
    map<string, int> m;

    // count occurrence of every string
    for (int i = 0; i < address.size(); i++) {
        map<string, int>::iterator it = m.find(address[i]);

        if (it == m.end()) {
            m.insert(pair<string, int>((address)[i], 1));
        } else {
            m[(address)[i]]++;
        }
    }

    numAddress = m.size();

    // find the max
    map<string, int>::iterator it = m.begin();
    for (map<string, int>::iterator it2 = m.begin(); it2 != m.end(); ++it2) {
        if (it2 -> second > it -> second) {
            it = it2;
        }
    }

    // find the second to max
    map<string, int>::iterator it3 = m.begin();
    for (map<string, int>::iterator it4 = m.begin(); it4 != m.end(); ++it4) {
        if ((it4 -> second < it -> second) && (it4 -> second > it3 -> second)) {
            it3 = it4;
        }
    }

    // find the third to max
    map<string, int>::iterator it5 = m.begin();
    for (map<string, int>::iterator it6 = m.begin(); it6 != m.end(); ++it6) {
        if ((it6 -> second < it3 -> second) && (it6 -> second > it5 -> second)) {
            it5 = it6;
        }
    }

    cout << "Address with the highest occurrence is: " + it -> first << endl;
    cout << to_string(it -> second) + " occurrences out of " + to_string(m.size()) + 
            " unique addresses" << endl;
    cout << "Address with the second highest occurrence is: " + it3 -> first << endl;
    cout << to_string(it3 -> second) + " occurrences out of " + to_string(m.size()) + 
            " unique addresses" << endl;
    cout << "Address with the third highest occurrence is: " + it5 -> first << endl;
    cout << to_string(it5 -> second) + " occurrences out of " + to_string(m.size()) + 
            " unique addresses" << endl;

    if (checkIfValid(it, it3, it5)) {
        cout << it -> first << " is very likely to be target" << endl;
    } else {
        cout << it -> first << " not likely to be target" << endl;
    }

    return m;
}


bool checkIfValid(map<string, int>::iterator it_first, map<string, int>::iterator it_second, map<string, int>::iterator it_third) {
    bool ret = false;
    double percent = it_first -> second / numAddress;

    // Guess 1
    // target has occurrence higher than 5%
    if (percent >= 0.05) {
        ret = true;
    } else {
        ret = false;
    }

    // Guess 2
    // check if first and second have similar address
    string addressFirst = it_first -> first;
    string addressSecond = it_second -> first;
    if (addressFirst.compare(0, 5, addressSecond.substr(0, 5)) == 0) {
        ret = false;
    } else {
        ret = true;
    }

    return ret;
}


// exports number of frequency for each address to a txt file
void exportMapValues(map<string, int> &m) {
    // write to a new file
    ofstream file("frequency.txt");

    file << "frequency\n";

    map<string, int>::iterator it = m.begin();
    for (it; it != m.end(); ++it) {
        file << to_string(it -> second) << endl; 
    }

    file.close();
}


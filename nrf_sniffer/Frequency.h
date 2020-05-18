#ifndef Frequency_H
#define Frequency_H

#include <iostream>
#include <assert.h>
using namespace std;

class Frequency {
    private:
    int next_frequency;

    public:
    int start_frequency;            // in MHz
    int end_frequency;              // in MHz
    int increment;                  // aka how wide each channel is
    int channel;                    // current channel number
    int num_channel;                // number of channels available

    Frequency() {
        start_frequency     = 2402;
        end_frequency       = 2480;
        increment           = 2;
        channel             = 1;
        num_channel         = 40;
        next_frequency   = 2402;
    }

    Frequency(int start_frequency, int end_frequency, int increment) {
        this -> start_frequency = start_frequency;
        this -> end_frequency = end_frequency;
        this -> increment = increment;
        channel = 1;
        num_channel = (end_frequency - start_frequency) / increment + 1;
        next_frequency = start_frequency;
        check_inv();
    }

    bool check_inv() const {
        assert(start_frequency <= end_frequency);
        assert(increment > 0 && increment <= (end_frequency - start_frequency));
        assert(channel <= num_channel);
    }
    
    int next() {
        if (next_frequency > end_frequency) {
            return NULL;
        }

        int ret = next_frequency;
        next_frequency += increment;
        ++channel;
        return ret;
    }

    bool hasNext() {
        return next_frequency <= end_frequency;
    }

};
#endif
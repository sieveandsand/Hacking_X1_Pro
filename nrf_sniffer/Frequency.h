#ifndef Frequency_H
#define Frequency_H

#include <iostream>
#include <assert.h>
using namespace std;

/*
    nRF24L01 has 126 channels
    channel number: 0 - 125
    0   --> 2400 MHz
    1   --> 2401 MHz
    ......
    124 --> 2524 MHz
    125 --> 2525 MHz
*/

#define NRF_FREQUENCY_MIN       2400
#define NRF_FREQUENCY_MAX       2525
#define NRF_CHANNEL_MIN         0
#define NRF_CHANNEL_MAX         125

class Frequency {
    private:
    int start_frequency;            // in MHz
    int end_frequency;              // in MHz
    int increment;                  // aka how wide each channel is
    int channel;                    // current channel number

    public:
    int num_channel;                // number of channels available

    Frequency(int start_frequency, int end_frequency, int increment);

    void check_inv() const;
    
    int next();

    bool hasNext();

    void reset();

    int getChannel();

    int getFrequency();
};
#endif
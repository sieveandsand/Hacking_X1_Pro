#include "Frequency.h"

// input:
// start_frequency: start frequency of the device, inclusive
// end_frequency:   end frequency of the device, inclusive
// increment:       number of MHz between each channel
Frequency::Frequency(int start_frequency, int end_frequency, int increment) {
    this -> start_frequency = start_frequency;
    this -> end_frequency = end_frequency;
    this -> increment = increment;
    channel = start_frequency - NRF_FREQUENCY_MIN;                      // current channel
    num_channel = (end_frequency - start_frequency) / increment + 1;    // number of all working channels for the device
    check_inv();
}



void Frequency::check_inv() const {
    // start frequency has to be bigger than min frequency
    assert(start_frequency >= NRF_FREQUENCY_MIN);

    // end frequency has to be smaller than max frequency
    assert(end_frequency <= NRF_FREQUENCY_MAX);

    // start frequency has to be smaller than end frequency
    assert(start_frequency <= end_frequency);

    // increment has to be bigger than 0 && smaller than the diff between start and end
    assert(increment > 0 && increment <= (end_frequency - start_frequency));
}

// return: the next channel
int Frequency::next() {
    if (!Frequency::hasNext()) {
        return 0;
    }

    // store the return value
    int ret = Frequency::channel;

    // update current channel
    Frequency::channel += Frequency::increment;

    return ret;
}


// return: true while there is more channels
bool Frequency::hasNext() {
    return (Frequency::channel + NRF_FREQUENCY_MIN) <= Frequency::end_frequency;
}

void Frequency::reset() {
    Frequency::channel = Frequency::start_frequency - NRF_FREQUENCY_MIN;
}

// return: current channel nrf is scanning on
int Frequency::getChannel() {
    return Frequency::channel;
}

// return: current frequency nrf is scanning on
int Frequency::getFrequency() {
    return NRF_FREQUENCY_MIN + Frequency::channel;
}
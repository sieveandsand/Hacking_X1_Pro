#include "sniffer.h"

SNIFFER::SNIFFER(uint8_t ce, uint8_t csn) : _dev(ce, csn) {
    _radio.increment = EXWAY_INCREMENT;
    _radio.start_frequency = EXWAY_MIN_FREQUENCY;
    _radio.end_frequency = EXWAY_MAX_FREQUENCY;
}

SNIFFER::SNIFFER(uint8_t ce, uint8_t csn, uint16_t start_frequency, 
                uint16_t end_frequency, 
                uint8_t increment) : _dev(ce, csn) {
                    try {
                        if (start_frequency < LAW_MIN_FREQUENCY) {
                            throw FrequencyUnderflow;
                        }

                        if (end_frequency > LAW_MAX_FREQUENCY) {
                            throw FrequencyOverflow;
                        }

                        if (start_frequency > end_frequency) {
                            throw FrequencyMismatch;
                        }

                        if (increment != 1 || increment != 2) {
                            throw FrequencyIncrementWrong;
                        }

                        _radio.increment = increment;
                        _radio.start_frequency = start_frequency;
                        _radio.end_frequency = end_frequency;
                    } 
                    catch (int pError){
                        switch (pError) {
                            case FrequencyUnderFlow:
                                _radio.start_frequency = LAW_MIN_FREQUENCY;
                                #ifdef ARDUINO
                                    Serial.print("Error: start frequency should be between ");
                                    Serial.print(LAW_MIN_FREQUENCY);
                                    Serial.print(" and ");
                                    Serial.println(LAW_MAX_FREQUENCY);
                                #elif
                                    std::cout << "ERROR: start frequency should be between " << LAW_MIN_FREQUENCY <<
                                                " and " << LAW_MAX_FREQUENCY << std::endl;
                                #endif
                                break;
                            case FrequencyOverflow:
                                _radio.end_frequency = LAW_MAX_FREQUENCY;
                                #ifdef ARDUINO
                                    Serial.print("Error: end frequency should be between ");
                                    Serial.print(LAW_MIN_FREQUENCY);
                                    Serial.print(" and ");
                                    Serial.println(LAW_MAX_FREQUENCY);
                                #elif
                                    std::cout << "ERROR: end frequency should be between " << LAW_MIN_FREQUENCY <<
                                                " and " << LAW_MAX_FREQUENCY << std::endl;
                                #endif
                                break;
                            case FrequencyMismatch:
                                _radio.start_frequency = LAW_MIN_FREQUENCY;
                                _radio.end_frequency = LAW_MAX_FREQUENCY;
                                #ifdef ARDUINO
                                    Serial.println("Error: start frequency should be lower than end frequency");
                                #elif
                                    std::cout << "Error: start frequency should be lower than end frequency" << std::endl;
                                #endif
                                break;
                            case FrequencyIncrementWrong;
                                _radio.increment = EXWAY_INCREMENT;
                                #ifdef ARDUINO
                                    Serial.print("Error: frequency increment should be the same as data rate ");
                                    Serial.println("and should be either 1 or 2");
                                #elif
                                    std::cout << "Error: frequency increment should be the same as data rate" <<
                                                " and should be either 1 or 2" << std::endl;
                                #endif
                                break;
                            default:
                                break;
                        }
                    }
                    
}

SNIFFER::~SNIFFER() {

}

uint8_t SNIFFER::init() {
    // the 'illegal' 2-byte address 0x0055
    const byte address[2] = {0x55, 0x00};

    _dev.begin();
    _dev.openReadingPipe(0, address);
    _dev.setPALevel(RF24_PA_MIN);
    _dev.setDataRate(RF24_2MBPS);
    _dev.setAddressWidth(-10);
    _dev.setAutoAck(0);
    _dev.disableCRC();
    _dev.disableDynamicPayloads();

    return 0;
}

static SNIFFER::magicID = 3932;

static void SNIFFER::setMagicID(uint16_t id) {
    SNIFFER::_magicID = id;
}

Iterator SNIFFER::begin() {
    return Iterator();
}

Iterator SNIFFER::end() {
    return Iterator(nullptr);
}

SNIFFER::Iterator::Iterator() {
    channel = _radio.start_frequency - HARDWARE_MIN_FREQUENCY;
}

SNIFFER::Iterator::Iterator(uint16_t channel) {
    try {
        if (channel < 0)
            throw ChannelNumberUnderflow;
        
        if (channel > 125)      // channel number max
            throw ChannelNumberOverflow;

        this -> channel = channel;
    }
    catch (int pError) {
        switch(pError) {
            case ChannelNumberUnderflow:
                this -> channel = LAW_MIN_FREQUENCY - LAW_MIN_FREQUENCY;
                #ifdef ARDUINO
                    Serial.println("Error: channel number cannot be smaller than 0");
                #elif
                    std::cout << "Error: channel number cannot be smaller than 0" << std::endl;
                #endif
                break;
            case ChannelNumberOverflow:
                this -> channel = LAW_MAX_FREQUENCY - LAW_MIN_FREQUENCY;
                #ifdef ARDUINO
                    Serial.print("Error: channel number cannnot be higher than ");
                    Serial.println(LAW_MAX_FREQUENCY - LAW_MIN_FREQUENCY);
                #elif
                    std::cout << "Error: channel number cannot be higher than " << LAW_MAX_FREQUENCY - LAW_MIN_FREQUENCY << std::endl;
                #endif
                break;
            default:
                break;
        }
    }
}

Iterator& SNIFFER::Iterator::operator++() {
    if ((channel + _radio.increment) > (_radio.end_frequency - LAW_MAX_FREQUENCY))
        return nullptr;

    channel += _radio.increment;
    return &this;
}

Iterator SNIFFER::Iterator::operator++(int) {
    Iterator iterator = *this;
    ++*this;
    return iterator;
}

bool SNIFFER::Iterator::operator!=(const Iterator &iterator) {
    return this -> channel != iterator -> channel;
}

bool SNIFFER::Iterator::operator==(const Iterator &iterator) {
    return !SNIFFER::Iterator::operator!=(iterator);
}

uint16_t SNIFFER::Iterator::operator*() {
    return channel;
}


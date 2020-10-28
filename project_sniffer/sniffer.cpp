#include "sniffer/sniffer.h"

Radio_t SNIFFER::_radio = {EXWAY_INCREMENT, EXWAY_MIN_FREQUENCY, EXWAY_MAX_FREQUENCY};

int SNIFFER::_testIfValidExway() {
    // properly sets up the device
    _dev.setAddressWidth(5);

    // connects to each address and test if the packet format is valid
    #if !defined(NDEBUG) && defined(ARDUINO) && ARDUINO >= 100
        Serial.println("Contains possible addresses: ");
        for (int i = 0; i < _possible_address_index; ++i) {
            Serial.print(_possible_address[i][0], HEX);
            Serial.print(_possible_address[i][1], HEX);
            Serial.print(_possible_address[i][2], HEX);
            Serial.print(_possible_address[i][3], HEX);
            Serial.println(_possible_address[i][4], HEX);
            Serial.print("above has occurred in ");
            Serial.print(_channels_index[i]);
            Serial.println(" channels");
        }
    #endif
    
    for (int i = 0; i < _possible_address_index; ++i) {     
        for (int j = 0; j < _channels_index[i]; ++j) {
            // need to reverse the address array here
            unsigned char littleEndianAddress[5];
            for (int k = 0; k < 5; ++k) {
                littleEndianAddress[k] = _possible_address[i][4 - k];
            }
            _dev.openReadingPipe(0, littleEndianAddress);
            _dev.setChannel(_channels_with_occurrence[i][j]);
            _dev.startListening();

            unsigned long start_time = millis();
            while((millis() - start_time) < (SNIFF_DURATION * 2)) {
                if (_dev.available()) {
                        unsigned char payload[32];
                        _dev.read(&payload, sizeof(payload));
                        // retrieve gear info from the packet
                        unsigned char gear_lower_byte = payload[GEAR_LOWER_BYTE];
                        unsigned char gear_higher_byte = payload[GEAR_HIGHER_BYTE];
                        // combines the lower half of the higher byte and higher half of the lower byte
                        unsigned char gear = ((gear_higher_byte & HIGHER_MASK) << 4) | ((gear_lower_byte & LOWER_MASK) >> 4);
                        if (gear == forward1 || gear == forward2 || gear == forward3 || gear == forward4
                            || gear == backward1 || gear == backward2 || gear == backward3 || gear == backward4) {
                                memcpy(_valid_address, littleEndianAddress, sizeof(_valid_address));
                                _possible_address_index = i;
                                #if !defined(NDEBUG) && defined(ARDUINO) && ARDUINO >= 100
                                    Serial.print("Address of the device (in hex) is: ");
                                    Serial.print(_valid_address[4], HEX);
                                    Serial.print(" ");
                                    Serial.print(_valid_address[3], HEX);
                                    Serial.print(" ");
                                    Serial.print(_valid_address[2], HEX);
                                    Serial.print(" ");
                                    Serial.print(_valid_address[1], HEX);
                                    Serial.print(" ");
                                    Serial.println(_valid_address[0], HEX);
                                #endif 
                                return 1;
                        }
                    _dev.flush_rx();
                }
            }
        }
    }

    return -1;
}

int SNIFFER::_listenWithChannelHopping() {
    // check if we have a valid address
    if (_valid_address[4] == 0) return -1;
    // set device to listening mode
    _dev.stopListening();
    _dev.setAddressWidth(5);
    _dev.openReadingPipe(0, _valid_address);
    _dev.startListening();

    int numToHop = _channels_index[_possible_address_index];

    int i = 0;
    while(1) {              // read until we get a good signal
        _dev.setChannel(_channels_with_occurrence[_possible_address_index][i]);

        #if !defined(NDEBUG) && defined(ARDUINO) && ARDUINO >= 100
            Serial.print("Reading channel: ");
            Serial.println(_channels_with_occurrence[_possible_address_index][i]);
        #endif

        if (_dev.available()) {
            _dev.read(&_payload, sizeof(_payload));
            unsigned char gear_lower_byte = _payload[GEAR_LOWER_BYTE];
            unsigned char gear_higher_byte = _payload[GEAR_HIGHER_BYTE];
            // combines the lower half of the higher byte and higher half of the lower byte
            _prevGear = ((gear_higher_byte & HIGHER_MASK) << 4) | ((gear_lower_byte & LOWER_MASK) >> 4); 
            _dev.stopListening();
            return 1;
        }

        i = (i < numToHop)? i + 1 : 0;
        _dev.flush_rx();
    }
}

unsigned char SNIFFER::_getDirectionGear() {
    // retrieve gear info from the packet
    unsigned char gear_lower_byte = _payload[GEAR_LOWER_BYTE];
    unsigned char gear_higher_byte = _payload[GEAR_HIGHER_BYTE];
    // combines the lower half of the higher byte and higher half of the lower byte
    unsigned char gear = ((gear_higher_byte & HIGHER_MASK) << 4) | ((gear_lower_byte & LOWER_MASK) >> 4);

    unsigned char throttle_lower_byte = _payload[THROTTLE_LOWER_BYTE];
    unsigned char throttle_middle_byte = _payload[THROTTLE_MIDDLE_BYTE];
    unsigned char throttle_higher_byte = _payload[THROTTLE_HIGHER_BYTE];
    uint32_t throttle = ((throttle_higher_byte & HIGHER_MASK) << 16) | (throttle_middle_byte << 8) | ((throttle_lower_byte & LOWER_MASK) >> 4);

    if (throttle > 65535) {     // 0xffff, throttle overflows to gear
        return _prevGear;
    } else {
        _prevGear = gear;
        return gear;
    }
}

SNIFFER::SNIFFER(uint8_t ce, uint8_t csn) : _dev(ce, csn) {
    _possible_address_index = 0;
    _channels_index = (uint16_t *)calloc(10, 2);
}

SNIFFER::SNIFFER(uint8_t ce, uint8_t csn, uint16_t start_frequency, 
                uint16_t end_frequency, 
                uint8_t increment) : _dev(ce, csn) {
                    _possible_address_index = 0;
                    _channels_index = (uint16_t *)calloc(10, 2);

                    /** Arduino does not support exception handling.
                     *  When exception occures on Arduino,
                     *  all frequency values are set to default.
                     */
                    #if defined(ARDUINO) && ARDUINO >= 100
                        _radio.increment = increment;
                        _radio.start_frequency = start_frequency;
                        _radio.end_frequency = end_frequency;

                        if (start_frequency < LAW_MIN_FREQUENCY) {
                            Serial.print("Error: start frequency should be between ");
                            Serial.print(LAW_MIN_FREQUENCY);
                            Serial.print(" and ");
                            Serial.println(LAW_MAX_FREQUENCY);
                            _radio.start_frequency = LAW_MIN_FREQUENCY;
                        }

                        if (end_frequency > LAW_MAX_FREQUENCY) {
                            Serial.print("Error: end frequency should be between ");
                            Serial.print(LAW_MIN_FREQUENCY);
                            Serial.print(" and ");
                            Serial.println(LAW_MAX_FREQUENCY);
                            _radio.end_frequency = LAW_MAX_FREQUENCY;
                        }

                        if (start_frequency > end_frequency) {
                            Serial.println("Error: start frequency should be lower than end frequency");
                            _radio.start_frequency = LAW_MIN_FREQUENCY;
                            _radio.end_frequency = LAW_MAX_FREQUENCY;
                        }

                        if (!(increment == 1 || increment == 2)) {
                            Serial.print("Error: frequency increment should be the same as data rate ");
                            Serial.println("and should be either 1 or 2");
                            _radio.increment = EXWAY_INCREMENT;
                        }
                    #elif
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
                        catch (int perror){
                            switch (perror) {
                                case FrequencyUnderFlow:
                                    _radio.start_frequency = LAW_MIN_FREQUENCY;
                                    std::cout << "ERROR: start frequency should be between " << LAW_MIN_FREQUENCY <<
                                                " and " << LAW_MAX_FREQUENCY << std::endl;
                                    break;
                                case FrequencyOverflow:
                                    _radio.end_frequency = LAW_MAX_FREQUENCY;
                                    std::cout << "ERROR: end frequency should be between " << LAW_MIN_FREQUENCY <<
                                                " and " << LAW_MAX_FREQUENCY << std::endl;
                                    break;
                                case FrequencyMismatch:
                                    _radio.start_frequency = LAW_MIN_FREQUENCY;
                                    _radio.end_frequency = LAW_MAX_FREQUENCY;
                                    std::cout << "Error: start frequency should be lower than end frequency" << std::endl;
                                    break;
                                case FrequencyIncrementWrong:
                                    _radio.increment = EXWAY_INCREMENT;
                                    std::cout << "Error: frequency increment should be the same as data rate" <<
                                                " and should be either 1 or 2" << std::endl;
                                    break;
                                default:
                                    break;
                            }
                        }
                    #endif
                    
}

SNIFFER::~SNIFFER() {

}

uint8_t SNIFFER::init() {
    // the 'illegal' 2-byte address 0x0055
    const unsigned char address[2] = {0x55, 0x00};

    _dev.begin();
    _dev.openReadingPipe(0, address);
    _dev.setPALevel(RF24_PA_MAX);
    _dev.setDataRate(RF24_2MBPS);
    _dev.setAddressWidth(-10);
    _dev.setAutoAck(0);
    _dev.disableCRC();
    _dev.disableDynamicPayloads();

    return 0;
}

void SNIFFER::scanWithMagicID() {
    SNIFFER::Iterator it = begin();
    for (it; it != end(); ++it) {
        _dev.setChannel(*it);
        _dev.startListening();
        int flags[10] = {0};

        #if !defined(NDEBUG) 
            #if defined (ARDUINO) && ARDUINO >= 100
                Serial.print("Scanning channel ");
                Serial.println(*it);
            #elif
                std::cout << "Scanning channel " << *it << std::endl;
            #endif
        #endif

        unsigned long start_time = millis();
        while ((millis() - start_time) < SNIFF_DURATION) {
            if (_dev.available()) {
                bool goodSignal = _dev.testRPD();

                // make sure there is signal present on channel
                if (goodSignal) {
                    unsigned char temp_buffer[5];
                    // only read the first 5 address bytes
                    _dev.read(&temp_buffer, sizeof(temp_buffer));

                    // compare with "magic ID"
                    int ret = memcmp(temp_buffer, _magicID, sizeof(_magicID));

                    // if matches with the magic ID
                    if (ret == 0) {
                        #if !defined(NDEBUG) 
                            #if defined (ARDUINO) && ARDUINO >= 100
                                Serial.println("Found a matching address!");
                            #elif
                                std::cout << "Found a matching address!" << std::endl;
                            #endif
                        #endif

                        bool occurred = false;
                        // check if this address has occured previously
                        for (int i = 0; i < _possible_address_index && !occurred; ++i) {
                            int ret = memcmp(temp_buffer, _possible_address[i], sizeof(temp_buffer));
                            occurred = (ret == 0) ? true : false;
                            if (_channels_index[i] < 6 && flags[i] == 0 && occurred) {
                                _channels_with_occurrence[i][_channels_index[i]] = *it;
                                ++_channels_index[i];
                                flags[i] = -1;
                            }
                        }

                        // store candidate address in buffer
                        if (!occurred && _possible_address_index < 10) {
                             // stores the channels of occurrence for each unique matching address
                            if (_channels_index[_possible_address_index] < 6 && flags[_possible_address_index] == 0) {
                                _channels_with_occurrence[_possible_address_index][_channels_index[_possible_address_index]] = *it;
                                ++_channels_index[_possible_address_index];
                                flags[_possible_address_index] = -1;
                            }
                            memcpy(_possible_address[_possible_address_index], temp_buffer, sizeof(temp_buffer));
                            ++_possible_address_index;
                        }
                    }
                }
                _dev.flush_rx();
            }
        }
        _dev.stopListening();
    }

    int ret = _testIfValidExway();
    if (!ret) {
        _testIfValidExway();
    }
}

int SNIFFER::getDirection() {
    if (_listenWithChannelHopping()) {
        // retrieve gear info from the packet
        unsigned char gear_lower_byte = _payload[GEAR_LOWER_BYTE];
        unsigned char gear_higher_byte = _payload[GEAR_HIGHER_BYTE];
        // combines the lower half of the higher byte and higher half of the lower byte
        unsigned char gear = ((gear_higher_byte & HIGHER_MASK) << 4) | ((gear_lower_byte & LOWER_MASK) >> 4);
        if (gear == forward1 || gear == forward2 || gear == forward3 || gear == forward4) {
            return 1;
        } else if (gear == backward1 || gear == backward2 || gear == backward3 || gear == backward4) {
            return -1;
        }
    }

    return 0;
}

uint32_t SNIFFER::getThrottle() {
    if (_listenWithChannelHopping()) {
        unsigned char throttle_lower_byte = _payload[THROTTLE_LOWER_BYTE];
        unsigned char throttle_middle_byte = _payload[THROTTLE_MIDDLE_BYTE];
        unsigned char throttle_higher_byte = _payload[THROTTLE_HIGHER_BYTE];
        // have to check if throttle and gear overlap occured
        unsigned char gear_lower_byte = _payload[GEAR_LOWER_BYTE];
        unsigned char gear_higher_byte = _payload[GEAR_HIGHER_BYTE];
        // combines the lower half of the higher byte and higher half of the lower byte
        unsigned char gear = ((gear_higher_byte & HIGHER_MASK) << 4) | ((gear_lower_byte & LOWER_MASK) >> 4);
        uint32_t throttle;
        if (gear == forward1 || gear == forward2 || gear == forward3 || gear == forward4
            || gear == backward1 || gear == backward2 || gear == backward3 || gear == backward4) {
                throttle = ((throttle_higher_byte & HIGHER_MASK) << 16) | (throttle_middle_byte << 8) | ((throttle_lower_byte & LOWER_MASK) >> 4);
            } else {
                throttle = (throttle_higher_byte << 16) | (throttle_middle_byte << 8) | ((throttle_lower_byte & LOWER_MASK) >> 4);
            }
        return throttle;
    }

    return 0;
}

GearLevel_t SNIFFER::getGear() {
    if (_listenWithChannelHopping()) {
        unsigned char gear = _getDirectionGear();
        return (GearLevel_t)gear;
    }

    return gear_error;
}

void SNIFFER::printCandidateAddresses() {
    for (int i = 0; i < _possible_address_index; ++i) {
        Serial.println("---------Candidate Addresses (in hex)--------");
        Serial.print(_possible_address[i][0], HEX);
        Serial.print(" ");
        Serial.print(_possible_address[i][1], HEX);
        Serial.print(" ");
        Serial.print(_possible_address[i][2], HEX);
        Serial.print(" ");
        Serial.print(_possible_address[i][3], HEX);
        Serial.print(" ");
        Serial.println(_possible_address[i][4], HEX);
        Serial.println("----------------end---------------------------");
    }
}

unsigned char* SNIFFER::getAddress() {
    return _valid_address;
}

unsigned char SNIFFER::_magicID[] = {0x39, 0x32};

void SNIFFER::setMagicID(unsigned char (&id)[2]) {
    if (sizeof(id) != 2) {
        #if defined (ARDUINO) && ARDUINO >= 100
            Serial.println("Error: invalid magic ID, magic ID reset to default");
        #elif
            std::cout << "Error: invalid magic ID, magic ID reset to default" << std::endl;
        #endif
        _magicID[0] = 0x39;
        _magicID[1] = 0x32;
    } else {
        memcpy(_magicID, id, sizeof(id));
    }
}

SNIFFER::Iterator SNIFFER::begin() {
    return SNIFFER::Iterator();
}

SNIFFER::Iterator SNIFFER::end() {
    return SNIFFER::Iterator(-1);
}

void SNIFFER::printDetails() {
    _dev.printDetails();
}

SNIFFER::Iterator::Iterator() {
    _channel = _radio.start_frequency - HARDWARE_MIN_FREQUENCY;
}

SNIFFER::Iterator::Iterator(int16_t channel) {
    /** Arduino does not support exception handling.
     *  When exception occures on Arduino,
     *  all frequency values are set to default.
     */
    #if defined(ARDUINO) && ARDUINO >= 100
        this -> _channel = channel;

        if (channel < 0 && channel != -1) {
            Serial.println("Error: channel number cannot be smaller than 0");
            this -> _channel = LAW_MAX_FREQUENCY - LAW_MIN_FREQUENCY;
        }

        if (channel > 125) {        // channel number max
            Serial.print("Error: channel number cannnot be higher than ");
            Serial.println(LAW_MAX_FREQUENCY - LAW_MIN_FREQUENCY);
            this -> _channel = LAW_MAX_FREQUENCY - LAW_MIN_FREQUENCY;
        }
    #elif
        try {
            if (channel < 0 && channel != -1)
                throw ChannelNumberUnderflow;
            
            if (channel > 125)      // channel number max
                throw ChannelNumberOverflow;

            this -> _channel = channel;
        }
        catch (int perror) {
            switch(perror) {
                case ChannelNumberUnderflow:
                    this -> _channel = LAW_MAX_FREQUENCY - LAW_MIN_FREQUENCY;
                    std::cout << "Error: channel number cannot be smaller than 0" << std::endl;
                    break;
                case ChannelNumberOverflow:
                    this -> _channel = LAW_MAX_FREQUENCY - LAW_MIN_FREQUENCY;
                    std::cout << "Error: channel number cannot be higher than " << LAW_MAX_FREQUENCY - LAW_MIN_FREQUENCY << std::endl;
                    break;
                default:
                    break;
            }
        }
    #endif
}

SNIFFER::Iterator& SNIFFER::Iterator::operator++() {
    if ((_channel + _radio.increment) > (_radio.end_frequency - LAW_MIN_FREQUENCY))
        _channel = -1;
    else 
        _channel += _radio.increment;
    return *this;
}

SNIFFER::Iterator SNIFFER::Iterator::operator++(int) {
    SNIFFER::Iterator iterator = *this;
    ++*this;
    return iterator;
}

bool SNIFFER::Iterator::operator!=(const SNIFFER::Iterator &iterator) {
    return this -> _channel != iterator._channel;
}

bool SNIFFER::Iterator::operator==(const SNIFFER::Iterator &iterator) {
    return !SNIFFER::Iterator::operator!=(iterator);
}

uint16_t SNIFFER::Iterator::operator*() {
    return _channel;
}


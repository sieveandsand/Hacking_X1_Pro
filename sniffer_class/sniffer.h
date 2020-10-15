#pragma once

#include <stdio.h>
#include <stdint.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
#include "sniffer_config.h"

#ifdef ARDUINO
    #include <Arduino.h>
    #include <avr/pgmspace.h>
#elif
    #include <iostream>
#endif

#define PAYLOAD_SIZE                    32
#define ADDRESS_BUFFER_SIZE             200
#define RF_CH_MAX                       525
#define SNIFF_DURATION                  1000
#define EXWAY_MAX_FREQUENCY             2402
#define EXWAY_MIN_FREQUENCY             2480
#define EXWAY_INCREMENT                 2
#define HARDWARE_MAX_FREQUENCY          2525        // hardware RF frequency upper limit
#define HARDWARE_MIN_FREQUENCY          2400        // hardware frequency lower limit

#ifdef COUNTRY_US
    #define LAW_MAX_FREQUENCY           2483        // max RF frequency by US law
    #define LAW_MIN_FREQUENCY           2400        // min RF frequency by US law
#elif
    #define LAW_MAX_FREQUENCY           HARDWARE_MAX_FREQUENCY   // PLEASE UPDATE PER REGION LAW
    #define LAW_MIN_FREQUENCY           HARDWARE_MIN_FREQUENCY   // PLEASE UPDATE PER REGION LAW
#endif

typedef struct Radio_t {
    uint8_t increment;
    uint16_t start_frequency;
    uint16_t end_frequency;
}   Radio_t;


class SNIFFER {
    private:
    RF24 _dev;

    Radio_t _radio;

    static uint16_t _magicID;

    unsigned char _payload_buffer[PAYLOAD_SIZE];

    public:
    /**
     * Default constructor
     * 
     * SNIFFER class initialized to Exway X1 Pro config.
     * 
     * @param ce    ce pin number
     * @param csn   csn pin number
     */
    SNIFFER(uint8_t ce, uint8_t csn);

    /**
     * Constructor w/ parameters
     * 
     * Configures SNIFFER class to potentially work with other remotes
     * 
     * @param ce                ce pin number
     * @param csn               csn pin number
     * @param start_frequency   lower limit of the RF operation frequency.
     *                          LAW_MIN_FREQUENCY <= start_frequency <= LAW_MAX_FREQUENCY;
     *                          start_frequency < end_frequency
     * @param end_frequency     upper limit of the RF operation frequency.
*                               LAW_MIN_FREQUENCY <= end_frequency <= LAW_MAX_FREQUENCY;
     *                          end_frequency > start_frequency
     * @param increment         RF channel width, depends on data rate.
     *                          have int value either 1 or 2
     */
    SNIFFER(uint8_t ce, uint8_t csn, uint16_t start_frequency, uint16_t end_frequency, uint8_t increment);

    ~SNIFFER();

    /**
     * initializes and configures nRF24L01+
     * 
     * Several important configurations are made here:
     *     1. RX address is set to 2-byte illegal address
     *     2. Power level set to minimum
     *     3. Data rate set to 2Mbps
     *     4. AutoACK off, CRC off, dynamic payload off
     * 
     * @return 0 upon success
     */
    uint8_t init();

    /** 
     * sets the magicID
     * 
     * The first two MSBs of the address is the manufacture ID.
     * i.e. Same manufacture will usually have the same ID.
     * Knowing this ID will greatly reduce search time
     * And leads to more accurate results.
     * 
     * Default value is 3932 for Exway boards.
     * 
     * @param id manufacture ID in integer form.
     *           has to be a 4-digit integer
    */
    static void setMagicID(uint16_t id);

    /**
     * Forward declearation of iterator class
     */
    class Iterator;

    /**
     * points to the first channel to be scanned
     * 
     * @return Iterator that points to the start channel number
     */
    Iterator begin();

    /**
     * points to nullpointer
     * 
     * @return Iterator that points to nullpointer
     */
    Iterator end();

    class Iterator {
        private:
        uint16_t channel;

        public:
        /**
         * constructor is always initialized to the first channel
         */
        Iterator();

        Iterator(uint16_t channel);

        /**
         * pre-increment
         */
        Iterator& operator++();

        /**
         * post-increment
         */
        Iterator operator++(int);

        bool operator!=(const Iterator &iterator);

        bool operator==(const Iterator &iterator);

        uint16_t operator*();
    };
};
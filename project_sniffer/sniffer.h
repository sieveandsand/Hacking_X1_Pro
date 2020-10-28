#pragma once

#include <stdio.h>
#include <stdint.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
#include "sniffer_config.h"

#if defined(ARDUINO) && ARDUINO >= 100
    #include <Arduino.h>
    #include <avr/pgmspace.h>
#elif
    #include <iostream>
#endif

#define PAYLOAD_SIZE                    32
#define ADDRESS_BUFFER_SIZE             200
#define RF_CH_MAX                       525
#define SNIFF_DURATION                  2000        // in milliseconds
#define EXWAY_MAX_FREQUENCY             2480
#define EXWAY_MIN_FREQUENCY             2402
#define EXWAY_INCREMENT                 2
#define HARDWARE_MAX_FREQUENCY          2525        // hardware RF frequency upper limit
#define HARDWARE_MIN_FREQUENCY          2400        // hardware frequency lower limit
#define GEAR_LOWER_BYTE                 5           // position of the gear info (lower byte) in the packet
#define GEAR_HIGHER_BYTE                4           // position of the gear info (higher byte) in the packet
#define LOWER_MASK                      0xf0
#define HIGHER_MASK                     0xf  
#define THROTTLE_LOWER_BYTE             7   
#define THROTTLE_MIDDLE_BYTE            6
#define THROTTLE_HIGHER_BYTE            5           // throttle and gear have overlap

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


typedef enum GearLevel_t {
    forward1 = 0x00,
    forward2 = 0x08,
    forward3 = 0x10,
    forward4 = 0x18,
    backward1 = 0x40,
    backward2 = 0x48,
    backward3 = 0x50,
    backward4 = 0x58,
    gear_error = -1
} GearLevel_t;

const int FrequencyUnderflow        = 1;
const int FrequencyOverflow         = 2;
const int FrequencyMismatch         = 3;
const int FrequencyIncrementWrong   = 4;
const int ChannelNumberUnderflow    = 5;
const int ChannelNumberOverflow     = 6;

class SNIFFER {
    private:
    RF24 _dev;

    static Radio_t _radio;

    static unsigned char _magicID[2];

    unsigned char _possible_address[10][5];

    int _possible_address_index;

    uint16_t _channels_with_occurrence[10][6];      // nrf has 6 data pipes

    uint16_t *_channels_index;

    unsigned char _valid_address[5];

    unsigned char _payload[32];

    unsigned char _prevGear;

    /**
     * tests if the addresses stored in buffer are valid exway addresses
     * 
     */ 
    int _testIfValidExway();

    int _listenWithChannelHopping();

    // return the gear info and check if overflow has occured (if so, then gear cannot be changed, and have to return the previous value)
    unsigned char _getDirectionGear();

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

    /**
     * destructor, needs to be implemented
     */
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
     * (work in progress, not yet implemented)
     * scans all possible channels and looks for target address
     * 
     * This function attempts to find the target address without 
     * the use of manufacture id, aka magic ID. Exway board users
     * should refer to scanWithMagicID(), which is both faster and 
     * more reliable. Users of other brands that use nRF24 transceiver
     * should use this function if they do not know the magic ID already.
     * 
     * Three parameters have to be met for an address to become a potential candidate
     *      1. target has occurrence higher than 5% out of all addresses occured in one channel
     *      2. addresses of highest and second highest occurrence must be different in at least 2 bytes
     *      3. target is not explicitly blocked
     */
    void scan();

    /**
     * scans all possible channels and looks for target address while knowing the manufacture id
     * 
     * Knowing the manufacture id eliminates the need for extra calculations 
     * (to be) implemented in scan(). Manufacture ID is pre-configured to exway. 
     * If using this function for other brands, call setMagicID() to change this configuration. 
     * This function is a blocking function that takes numOfChannels * SNIFFDURATION milliseconds to finish
     * 
     * @return unsigned char pointer to valid address upon success, null pointer otherwise
     */
    void scanWithMagicID();

    /**
     * prints the captured candidate addresses to the terminal
     * Currently implemented only for Arduino
     */
    void printCandidateAddresses();

    /** 
     * call this function to get the address of the remote
     * 
     * This function should only be called after scan() or scanWithMagicID() is called,
     * otherwise the return value will be garbage
     * 
     * @return an unsigned char pointer to a 5-byte address
     *         null pointer if no valid address is available
     */
    unsigned char* getAddress();

    /**
     * reads packets and returns the directiont the board is traveling
     * 
     * scanMagicID() or scan() must be called befored using this function
     * 
     * @return -1 if the board is in reverse, 
     *          1 if the board is forward direction
     *          0 if direction is not available
     */
    int getDirection();

    /**
     * reads packets and returns throttle level
     * 
     * The throttle value here is a 3-byte value read from the hall effect sensor on board.
     * It reflects the acclerations of the vehicle.
     * Max throttle value: should be around 0x0BFFFF depending on the calibration of the hall effect sensor
     * Min throttle Value: 0x01
     * 
     * @return uint32_t value that is between the range of min and max described above
     */
    uint32_t getThrottle();

    /** 
     * reads packets and returns GearLevel_t
     *
     * @return enum GearLevel_t that indicates the current gear
     */
    GearLevel_t getGear();

    /** 
     * sets the magicID
     * 
     * The first two MSBs of the address is the manufacture ID.
     * i.e. Same manufacture will usually have the same ID.
     * Knowing this ID will greatly reduce search time
     * And leads to more accurate results.
     * 
     * Default value is 0x3932 for Exway boards.
     * 
     * @param id manufacture ID in integer form.
     *           has to be a 4-digit integer
    */
    static void setMagicID(unsigned char (&id)[2]);

    /**
     * Forward declearation of iterator class
     */
    class Iterator;

    /**
     * points to the first channel to be scanned
     * 
     * @return Iterator that points to the start channel number
     */
    SNIFFER::Iterator begin();

    /**
     * points to nullpointer
     * 
     * @return Iterator that points to nullpointer
     */
    SNIFFER::Iterator end();

    /**
     * prints nRF24L01+ device configurations
     */
    void printDetails();

    /**
     * An iterator class implemented to iterate through channel numbers
     */
    class Iterator {
        private:
        int16_t _channel;

        public:
        /**
         * constructor is always initialized to the first channel
         */
        Iterator();

        Iterator(int16_t channel);

        /**
         * pre-increment operator override
         * 
         * @return reference to @code SNIFFER::Iterator @endcode
         */
        SNIFFER::Iterator& operator++();

        /**
         * post-increment operator override
         * 
         * @return @code SNIFFER::Iterator object @endcode
         */
        SNIFFER::Iterator operator++(int);

        bool operator!=(const SNIFFER::Iterator &iterator);

        bool operator==(const SNIFFER::Iterator &iterator);

        uint16_t operator*();
    };
};
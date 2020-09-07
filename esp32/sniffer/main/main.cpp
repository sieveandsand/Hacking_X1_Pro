// TODO:
// add buttons and eink display


#include "main.h"

RF24* setupRadio() {
    // the 'illegal' 2-byte address 0x0055
    const unsigned char address[2] = {0x55, 0x00};

    RF24* radio = new RF24(12, 5, 18, 19, 23);
    radio.begin()
    radio.openReadingPipe(0, address);
    radio.setPALevel(RF24_PA_MIN);
    radio.setDataRate(RF24_2MBPS);          // date rate set to 2MBPS as per Exway spec
    radio.setAddressWidth(-10);             // arbitrary negative number that sets TX/RX address field to '00'
    radio.setAutoAck(0);                    // turns auto-ack off, CRC cannot be disabled if auto-ack is enabled
    radio.disableCRC();
    radio.disableDynamicPayloads();

    return radio;
}




// make the cpp main function have C linkage
extern "C" {
    void app_main();
}

void app_main() {
    // setup
    RF24* radio = setupRadio();

    while(true) {
        if (s == idle) {                    // idle state
            // check button
        }
    }
}
// esp32 sniffer code for nRF24
#include <stdio.h>
#include <stdint.h>
#include <avr/pgmspace.h>
#include <nRF24L01.h>
#include <printf.h>
#include <RF24.h>
#include <RF24_config.h>

#define PAYLOAD_SIZE              32                // 1 to 32 byte dynamic payload length
#define ADDRESS_BUFFER_SIZE       200               // size of the buffer to store the potential addresses
#define RF_CH_MAX                 525               // channel frequency is calculated by 2400 + RF_CH
#define SNIFF_DURATION            1000             // the amount of time in millisecond to sniff each frequency
#define SCAN                      0x31
#define BYTESPERINT               16                // number of bytes in an int

// defining states
typedef enum {
  idle, scan, set
} State;

// function prototypes
void checkSerial();
bool receiveWithEndMarker();

// using libraries from 
RF24 radio(7, 8); // CE, CSN

// the 'illegal' 2-byte address 0x0055
const byte address[2] = {0x55, 0x00};

unsigned long startTime;
State s = idle;
char number[BYTESPERINT + 1];
uint8_t channel;

void setup() {
  Serial.begin(115200);
  printf_begin();
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_2MBPS);        // data rate set to 2MBPS as per Exway spec
  radio.setAddressWidth(-10);           // arbitrary negative number that sets TX/RX address field to '00'
  radio.setAutoAck(0);                  // turns auto-ack off, CRC cannot be disabled if auto-ack is enabled
  radio.disableCRC();
  radio.disableDynamicPayloads();
}

void loop() {
  if (s == idle) {
    //checkSerial();
    bool newNumber = receiveWithEndMarker();
    if (newNumber) {
      channel = strtol(number, NULL, 10);
      s = set;
    }
  } else if (s == set) {
    // set RF communication channel
    radio.setChannel(channel);
    
    // start listening
    radio.startListening();
    s = scan;
  } else if (s == scan) {
    startTime = millis();
    
    while((millis() - startTime) < SNIFF_DURATION) {
      if(radio.available()) {    
        unsigned char payload[PAYLOAD_SIZE];
        bool goodSignal = radio.testRPD();

        // read iff there is a signal is greater than -64 dBm
        if (goodSignal) {
          radio.read(&payload, sizeof(payload));
          
          for (int i = 0; i < sizeof(payload);  i++) {
            if (i < 5) {
              printf("%02X", payload[i]);
            } else {
              printf(" %02X", payload[i]);
            }
          }
          printf("\n");          
        }

        radio.flush_rx();
      }
    }

    s = idle;
  }
}

// checks input from serial
void checkSerial() {
  int i = 0;
  int frequency;
  

  //while (Serial.available() > 0) {
    frequency = (int) Serial.parseInt();
  //}

  printf("%d\n", frequency);
//  if (i == 3) {
//    s = SCAN;
//  }
}

// checks serial port and converts bytes to integer
// until an end marker is reached
// in: none
// return: true if a new number is received
bool receiveWithEndMarker() {
  bool newNumber = false;
  char endMarker = '\n';
  char received;
  static int i = 0;

  if (Serial.available() > 0) {
    received = Serial.read();

    if (received > 0) {   // to filter out acknoledgement
      if (received != endMarker) {
        number[i++] = received;
        if (i > BYTESPERINT) {
          i = BYTESPERINT;
        }
      } else {
        number[i] = '\0';
        i = 0;
        newNumber = true;
      }
    }
  }

  return newNumber;
}

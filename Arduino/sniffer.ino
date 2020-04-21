// sniffer code for nRF24

#include <stdio.h>
#include <avr/pgmspace.h>
#include <nRF24L01.h>
#include <printf.h>
#include <RF24.h>
#include <RF24_config.h>

#define PAYLOAD_SIZE 32                // 1 to 32 byte dynamic payload length
#define ADDRESS_BUFFER_SIZE 200        // size of the buffer to store the potential addresses
#define RF_CH_MAX 525                  // channel frequency is calculated by 2400 + RF_CH
#define SNIFF_DURATION 30000           // the amount of time in millisecond to sniff each frequency

// function prototypes
int fillAddressBuffer(unsigned char payload[PAYLOAD_SIZE]);
int highestOccurrence();
void printBuffer();

// global variables

// knock-off arduino
RF24 radio(7, 8); // CE, CSN

// the 'illegal' 2-byte address 0x0055
const byte address[2] = {0x55, 0x00};

unsigned long startTime;

void setup() {
  Serial.begin(115200);
  printf_begin();
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);      // set power amplifier level
  radio.setDataRate(RF24_1MBPS);      // set data rate, data rate should match target's
  // arbitrary negative number that sets TX/RX address field to '00'
  radio.setAddressWidth(-10);
  // turns auto-ack off
  // CRC cannot be disabled if auto-ack is enabled
  radio.setAutoAck(0);
  radio.disableCRC();
  radio.disableDynamicPayloads();
  radio.startListening();
  startTime = millis();
}

void loop() {
  while((millis() - startTime) < SNIFF_DURATION) {
    if(radio.available()) {    
      unsigned char payload[PAYLOAD_SIZE];
      radio.read(&payload, sizeof(payload));
      for (int i = 0; i < sizeof(payload);  i++) {
        printf("%02X ", payload[i]);
      }
      printf("\n");
    }
  }
}

//int fillAddressBuffer(unsigned char payload[PAYLOAD_SIZE]) {
//  // checks the first 5 bytes of the payload
//  // stores potential addresses in a buffer
//  
//  if(addressBufferIndex >= ADDRESS_BUFFER_SIZE) {
//    Serial.println("addressBuffer Overflow");
//    return -1;
//  } else {
//    for(int i = 0; i < 5; i++) {
//      addressBuffer[addressBufferIndex][i] = payload[i];
//    }
//  }
//  addressBufferIndex++;
//
//  return 0;
//}
//
//int highestOccurrence() {
//  int same = 1;
//  unsigned char occurrence[ADDRESS_BUFFER_SIZE];
//
//  // find occurrence for each element in addressBuffer
//  for(int i = 0; i < ADDRESS_BUFFER_SIZE; i++) {
//    occurrence[i] = 1;
//    for(int j = 0; j < i; j++) {
//      for(int k = 0; k < 5; k++) {
//        if(addressBuffer[j][k] != addressBuffer[i][k]) {
//          same = 0;
//        }
//      }
//      if(same) {
//        occurrence[i]++;
//      }
//      same = 1;
//    }
//  }
//
//  // find the highest occurrence
//  int high = 0;
//  int high_index = 0;
//  for(int i = 0; i < ADDRESS_BUFFER_SIZE; i++) {
//    if(occurrence[i] > high) {
//      high = occurrence[i];
//      high_index = i;
//    }
//  }
//  Serial.println(high);
//  return high_index;
//}
//
//// prints the content inside addressBuffer
//void printBuffer() {
//  printf("Below is content of the buffer:\n");
//  for(int i = 0; i < ADDRESS_BUFFER_SIZE; i++) {
//    for(int j = 0; j < 5; j++) {
//      printf("%02X ", addressBuffer[i][j]);
//    }
//    printf("\n");;
//  }
//  printf("End of the buffer.\n");
//}

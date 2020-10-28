#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
char msg[6];
RF24 radio(12, 14, 26, 25, 27);
const uint64_t pipe = 0xE8E8F0F0E1LL;
void setup(void){
 Serial.begin(115200);
 radio.begin();
 radio.setChannel(2);
 radio.setPayloadSize(7);
 //radio.setPALevel(RF24_PA_HIGH);
 radio.setDataRate(RF24_250KBPS);
 radio.openReadingPipe(1,pipe);
 radio.startListening();
}

void loop(void){
 if (radio.available()){  
     radio.read(msg, 6);      
     Serial.println(msg);

     delay(10);
 }
 else{
  //Serial.println("No radio available");
 }
}

#include <sniffer.h>
#include <printf.h>

#define GEAR1 3
#define GEAR2 4
#define GEAR3 5
#define GEAR4 6

SNIFFER* sniffer;

void setup() {
  pinMode(GEAR1, OUTPUT);
  pinMode(GEAR2, OUTPUT);
  pinMode(GEAR3, OUTPUT);
  pinMode(GEAR4, OUTPUT);
  Serial.begin(9600);
  printf_begin();
  sniffer = new SNIFFER(7, 8, 2402, 2480, 2);
  sniffer->init();
  sniffer->scanWithMagicID();
}

void loop() {
  // put your main code here, to run repeatedly:
  GearLevel_t gear = sniffer -> getGear();
  Serial.println(gear, HEX);
  if (gear == forward1 || gear == backward1) {
    digitalWrite(GEAR1, HIGH);
    digitalWrite(GEAR2, LOW);
    digitalWrite(GEAR3, LOW);
    digitalWrite(GEAR4, LOW);
  } else if (gear == forward2 || gear == backward2) {
    digitalWrite(GEAR2, HIGH);
    digitalWrite(GEAR1, LOW);
    digitalWrite(GEAR3, LOW);
    digitalWrite(GEAR4, LOW);
  } else if (gear == forward3 || gear == backward3) {
    digitalWrite(GEAR3, HIGH);
    digitalWrite(GEAR1, LOW);
    digitalWrite(GEAR2, LOW);
    digitalWrite(GEAR4, LOW);
  } else if (gear == forward4 || gear == backward4) {
    digitalWrite(GEAR4, HIGH);
    digitalWrite(GEAR1, LOW);
    digitalWrite(GEAR2, LOW);
    digitalWrite(GEAR3, LOW);
  }
}

#include "RF24.h"
#include "esp_bt.h"
#include "esp_wifi.h"
#include <SPI.h>
#include <ezButton.h>

SPIClass *sp = nullptr;
SPIClass *hp = nullptr;

RF24 radio(
    16, 15,
    16000000); // HSPI CAN SET SPI SPEED TO 16000000 BY DEFAULT ITS 10000000
RF24 radio1(
    22, 21,
    16000000); // VSPI CAN SET SPI SPEED TO 16000000 BY DEFAULT ITS 10000000

// HSPI=SCK = 14, MISO = 12, MOSI = 13, CS = 15 , CE = 16
// VSPI=SCK = 18, MISO =19, MOSI = 23 ,CS =21 ,CE = 22

unsigned int flag = 0;  // HSPI// Flag variable to keep track of direction
unsigned int flagv = 0; // VSPI// Flag variable to keep track of direction
int ch = 45;            // Variable to store value of ch
int ch1 = 45;           // Variable to store value of ch

ezButton toggleSwitch(33);

void two() {
  // SWEEP BURST MODE
  // We rapidly scan through the required channels in a tight loop
  // before yielding control to check the button again,
  // massively increasing the SPI hop rate.
  for (int burst = 0; burst < 20; burst++) {
    if (flagv == 0) {
      ch1 += 4;
    } else {
      ch1 -= 4;
    }

    if (flag == 0) {
      ch += 2;
    } else {
      ch -= 2;
    }

    if ((ch1 > 79) && (flagv == 0)) {
      flagv = 1;
    } else if ((ch1 < 2) && (flagv == 1)) {
      flagv = 0;
    }

    if ((ch > 79) && (flag == 0)) {
      flag = 1;
    } else if ((ch < 2) && (flag == 1)) {
      flag = 0;
    }

    radio.setChannel(ch);
    radio1.setChannel(ch1);
  }
}

void one() {
  // RANDOM BURST MODE
  // Removed delayMicroseconds to maximize aggression.
  // Performs a fast tight loop before yielding back to check the button.
  for (int burst = 0; burst < 30; burst++) {
    radio1.setChannel(random(80));
    radio.setChannel(random(80));
  }
}

void setup() {

  Serial.begin(115200);
  esp_bt_controller_deinit();
  esp_wifi_stop();
  esp_wifi_deinit();
  esp_wifi_disconnect();
  toggleSwitch.setDebounceTime(50);

  initHP();
  initSP();
}

void initSP() {
  sp = new SPIClass(VSPI);
  sp->begin();
  if (radio1.begin(sp)) {
    Serial.println("SP Started !!!");
    radio1.setAutoAck(false);
    radio1.stopListening();
    radio1.setRetries(0, 0);
    radio1.setPALevel(RF24_PA_MAX, true); // True enables LNA for max range
    radio1.setDataRate(RF24_2MBPS);
    radio1.setAddressWidth(3); // Tighter bandwidth occupation
    radio1.setPayloadSize(32); // Maximize packet airtime for disruption
    radio1.setCRCLength(RF24_CRC_DISABLED);
    radio1.printPrettyDetails();
    radio1.startConstCarrier(RF24_PA_MAX, ch1);
  } else {
    Serial.println("SP couldn't start !!!");
  }
}
void initHP() {
  hp = new SPIClass(HSPI);
  hp->begin();
  if (radio.begin(hp)) {
    Serial.println("HP Started !!!");
    radio.setAutoAck(false);
    radio.stopListening();
    radio.setRetries(0, 0);
    radio.setPALevel(RF24_PA_MAX, true); // True enables LNA for max range
    radio.setDataRate(RF24_2MBPS);
    radio.setAddressWidth(3); // Tighter bandwidth occupation
    radio.setPayloadSize(32); // Maximize packet airtime for disruption
    radio.setCRCLength(RF24_CRC_DISABLED);
    radio.printPrettyDetails();
    radio.startConstCarrier(RF24_PA_MAX, ch);
  } else {
    Serial.println("HP couldn't start !!!");
  }
}

void loop() {

  // Increased aggressiveness: Check button state less often
  // and perform rapid burst hopping inside the functions instead.
  toggleSwitch.loop();
  int state = toggleSwitch.getState();

  if (state == HIGH) {
    two();
  } else {
    one();
  }
}
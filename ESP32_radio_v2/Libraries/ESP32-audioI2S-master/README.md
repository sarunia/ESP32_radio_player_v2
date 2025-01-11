# Internet Radio & Audio Player v2

## Overview

This project is an **Internet Radio and Audio Player** built using the ESP32 platform. It allows you to listen to various internet radio stations and play audio files directly from an SD card. The device features an OLED display for user interaction, rotary encoders for navigation, and volume control, and supports multiple audio formats such as **MP3**, **FLAC**, and **AAC**. It also provides real-time weather information and has a sleep timer.

## Features

- **Internet Radio**: Stream radio stations from predefined URLs.
- **Audio Player**: Play audio files from an SD card (supports MP3, FLAC, AAC).
- **OLED Display**: Displays current station, track, and weather information.
- **Rotary Encoders**: For navigation and volume control.
- **Wi-Fi Configuration**: Easily set up Wi-Fi using [WiFiManager](https://github.com/tzapu/WiFiManager).
- **Real-Time Clock**: Synchronize time with an NTP server.
- **Weather Updates**: Display current weather conditions on the OLED display.
- **File Browser**: Browse and play files from SD card directories.
- **Multiple Radio Station Banks**: Store up to 100 stations per bank with support for up to 16 banks.
  
## Hardware Requirements

- **ESP32-S3** development board
- **PCM5102A DAC Module**
- **256x64 OLED Display with SSD1322 driver** (SPI-based)
- **Rotary Encoders** with push buttons (x2)
- **SD Card Reader** (SPI-based)
- **Wi-Fi connection** for radio streaming and weather updates

## Pin Configuration

### OLED Display (SPI)
- **MISO**: Not used
- **SCK**: Pin 38
- **MOSI**: Pin 39
- **DC**: Pin 40
- **RESET**: Pin 41
- **CS**: Pin 42

### SD Card Reader (SPI)
- **SCK**: Pin 45
- **MISO**: Pin 21
- **MOSI**: Pin 48
- **CS**: Pin 47

### PCM5102A DAC (I2S)
- **DIN**: Pin 13
- **BCLK**: Pin 12
- **LCK**: Pin 14

### Rotary Encoders
- **Right Encoder**: CLK: Pin 6, DT: Pin 5, SW: Pin 4
- **Left Encoder**: CLK: Pin 11, DT: Pin 10, SW: Pin 1

## Software Dependencies

The project uses various libraries to enable functionality:

- **[Arduino](https://www.arduino.cc/en/software)**: Standard library for ESP32 development.
- **[Audio](https://github.com/schreibfaul1/ESP32-audioI2S)**: For handling audio playback and streaming.
- **[U8g2lib](https://github.com/olikraus/u8g2)**: For controlling the OLED display.
- **[ezButton](https://github.com/ArduinoGetStarted/ezButton)**: For handling rotary encoder buttons.
- **[WiFiManager](https://github.com/tzapu/WiFiManager)**: For Wi-Fi setup.
- **[ArduinoJson](https://arduinojson.org/)**: For handling JSON data (e.g., weather API responses).
- **[Time](https://www.arduino.cc/reference/en/libraries/time/)**: For NTP-based time synchronization.

## Usage

1. Flash the code to your ESP32 device.
2. Use **WiFiManager** to set up Wi-Fi connectivity.
3. Stream internet radio from predefined stations or play audio files from an SD card.
4. Navigate between menus using the rotary encoders and display.

### Radio Station URLs
The radio stations are defined in banks, with up to 100 stations per bank. You can modify the station list by changing the URLs in the code:

- Bank 1: [Radio Bank 1](https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_01)
- Bank 2: [Radio Bank 2](https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_02)
- ... (up to 16 banks)

## License

This project is open-source and licensed under the [MIT License](https://opensource.org/licenses/MIT). Feel free to contribute and improve the code.

## Acknowledgments

- Special thanks to the authors of the libraries used in this project.
- The project is inspired by the open-source IoT community.


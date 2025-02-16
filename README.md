# ESP32-S3 Internet Radio & Audio Player v2

## Overview

This project is an **Internet Radio and Audio Player** built using the ESP32 platform. It allows you to listen to various internet radio stations and play audio files directly from an SD card. The device features an OLED display for user interaction, rotary encoders for navigation and volume control, and supports multiple audio formats such as **MP3**, **FLAC**, and **AAC**. It also provides real-time weather information and can be controlled via an infrared (IR) remote using the **NEC protocol** at 38 kHz.

## Features

- **Internet Radio**: Stream radio stations from predefined URLs (supports MP3, FLAC, AAC, OGG Vorbis and other).
- **Audio Player**: Play audio files from an SD card (supports MP3, FLAC, WAV and other).
- **OLED Display**: Displays current station, track, and weather information.
- **Rotary Encoders**: For navigation and volume control.
- **Wi-Fi Configuration**: Easily set up Wi-Fi using [WiFiManager](https://github.com/tzapu/WiFiManager).
- **Real-Time Clock**: Synchronize time with an NTP server.
- **Weather Updates**: Display current weather conditions on the OLED display.
- **File Browser**: Browse and play files from SD card directories.
- **Multiple Radio Station Banks**: Store up to 100 stations per bank with support for up to 16 banks.
- **Infrared Remote Control**: Control the player using an IR remote control, compatible with the **NEC protocol** (38 kHz). Functions include navigation (up/down/left/right), play/pause, and menu access.

## Hardware Requirements

- **ESP32-S3** development board
- **PCM5102A DAC Module**
- **256x64 OLED Display with SSD1322 driver** (SPI-based)
- **Rotary Encoders** with push buttons (x2)
- **SD Card Reader** (SPI-based)
- **IR Receiver** for 38 kHz (for remote control using the NEC protocol)
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

### IR Receiver (NEC Protocol, 38 kHz)
- **IR Receiver Signal Pin**: Pin 15

Below is the list of button commands used for controlling various functions of the audio device using an IR remote (NEC protocol). Each button has a corresponding hexadecimal address representing its function.

- `rcCmdVolumeUp (0x0028)` - **Volume Up**  
  *Increases the audio volume.*
  
- `rcCmdVolumeDown (0x0024)` - **Volume Down**  
  *Decreases the audio volume.*
  
- `rcCmdArrowRight (0x0026)` - **Right Arrow**  
  *Moves to the next station or file and starts playback immediately.*
  
- `rcCmdArrowLeft (0x0027)` - **Left Arrow**  
  *Moves to the previous station or file and starts playback immediately.*
  
- `rcCmdArrowUp (0x0030)` - **Up Arrow**  
  *Scrolls up through the list of stations or files.*

- `rcCmdArrowDown (0x0022)` - **Down Arrow**  
  *Scrolls down through the list of stations or files.*
  
- `rcCmdOk (0x0025)` - **OK**  
  *Confirms the selection of a station, folder, or file from the list.*
  
- `rcCmdMode (0x0020)` - **Mode**  
  *Switches between internet radio and file player modes.*
  
- `rcCmdHome (0x0023)` - **Home**  
  *Opens the system menu (functionality under development).*
  
- `rcCmdMute (0x0029)` - **Mute**  
  *Mutes the audio output (functionality under development).*
  
- `rcCmdKey0 (0x0012)` - **Key 0**  
  *Presses the "0" button on the remote.*

- `rcCmdKey1 (0x0015)` - **Key 1**  
  *Presses the "1" button on the remote.*

- `rcCmdKey2 (0x0014)` - **Key 2**  
  *Presses the "2" button on the remote.*

- `rcCmdKey3 (0x0008)` - **Key 3**  
  *Presses the "3" button on the remote.*

- `rcCmdKey4 (0x0011)` - **Key 4**  
  *Presses the "4" button on the remote.*

- `rcCmdKey5 (0x0010)` - **Key 5**  
  *Presses the "5" button on the remote.*

- `rcCmdKey6 (0x0009)` - **Key 6**  
  *Presses the "6" button on the remote.*

- `rcCmdKey7 (0x0007)` - **Key 7**  
  *Presses the "7" button on the remote.*

- `rcCmdKey8 (0x0006)` - **Key 8**  
  *Presses the "8" button on the remote.*

- `rcCmdKey9 (0x0005)` - **Key 9**  
  *Presses the "9" button on the remote.*

- `rcCmdBankUp (0x0018)` - **FAV+ (Bank Up)**  
  *Scrolls down through the list of banks or folders.*

- `rcCmdBankDown (0x0019)` - **FAV- (Bank Down)**  
  *Scrolls up through the list of banks or folders.*

## Software Dependencies

The project uses various libraries to enable functionality:

- **[Arduino](https://www.arduino.cc/en/software)**: Standard library for ESP32 development.
- **[Audio](https://github.com/schreibfaul1/ESP32-audioI2S)**: For handling audio playback and streaming.
- **[U8g2lib](https://github.com/olikraus/u8g2)**: For controlling the OLED display.
- **[ezButton](https://github.com/ArduinoGetStarted/ezButton)**: For handling rotary encoder buttons.
- **[WiFiManager](https://github.com/tzapu/WiFiManager)**: For Wi-Fi setup.
- **[ArduinoJson](https://arduinojson.org/)**: For handling JSON data (e.g., weather API responses).
- **[IRremote](https://github.com/z3t0/Arduino-IRremote)**: For decoding infrared remote signals using the NEC protocol.
- **[Time](https://www.arduino.cc/reference/en/libraries/time/)**: For NTP-based time synchronization.

## Usage

1. Flash the code to your ESP32 device.
2. Use **WiFiManager** to set up Wi-Fi connectivity.
3. Stream internet radio from predefined stations or play audio files from an SD card.
4. Navigate between menus using the rotary encoders, OLED display, and IR remote control.

### IR Remote Control Functions
The player can be controlled using an IR remote compatible with the **NEC protocol** (38 kHz). The following functions are mapped:

- **Navigation**: Control the player using up, down, left, right arrows on the remote.
- **Play/Pause**: Use the center or play button to toggle play/pause.
- **Menu Access**: Use the dedicated menu or home button to enter settings.

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

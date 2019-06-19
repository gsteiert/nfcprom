# nfcprom
NFC EEPROM Example for boards based on the ST25DV series of dual port NFC/I2C EEPROM, like the [Qwiic NFCPROM](https://hackaday.io/project/160582-qwiic-nfcprom), [Blinking Business-card Badge](https://hackaday.io/project/160590-blinking-business-card-badge-b3), or [Teardown2019 Badge](https://upverter.com/design/gsteiert/teardown2019/).  This example demonstrates how to manage a headless embedded WiFi web server with a phone through NFC. 

## Features
 - Set WiFi SSID and Password 
 - Provide IP address in URI NDEP
 - Log event messages in EEPROM 

## Requirements
 - ST25DV based board with I2C connector (Qwiic NFCPROM)
 - Phone with ST25 Android App
 - Arduino enabled ESP32 board (Feather Huzzah32) 
 - Adapter/cable to connect boards (QwiiKit)

## Instructions
1. Load SSID into ST25DV memory starting at address 0x00C0 using ST25 Android app
2. Load Password into ST25DV memory starting at address 0x00E0 using ST25 Android app
3. Load nfcprom.ino into ESP32 board, reset and wait for the board to start up.
4. Tap NFCPROM board to retrieve NDEF URI with address recieved through DHCP (phone must be on same WiFi network)
5. Open page served by ESP32, see LED turn/off
6. Read ST25DV memory starting at 0x0100 to see logged events.

## ST25DV Memory Map
 - 0x0000 NDEF URI
 - 0x00C0 SSID
 - 0x00E0 Password
 - 0x0100 Event message log

## Limitations and next steps
 - SSID and Password are just stored in memory as ASCII.  This is not easy to load with the ST25 app.  An NDEF or text file would be easier to load with the app.
 - Message are fixed 16 byte length
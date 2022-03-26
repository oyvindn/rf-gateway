# rf-gateway


## Hardware

* [Adafruit HUZZAH32 - ESP32 Feather](https://learn.adafruit.com/adafruit-huzzah32-esp32-feather)
* [HopeRF RFM69CW 13DBM 433Mhz RF Transceiver Module](https://www.hoperf.com/modules/rf_transceiver/RFM69C.html)


## Arduino sketch dependencies

* [Arduino core for the ESP32](https://github.com/espressif/arduino-esp32)
* [Arduino Client for MQTT](https://pubsubclient.knolleary.net/index.html)
* [RadioHead Packet Radio library for embedded microprocessors](http://www.airspayce.com/mikem/arduino/RadioHead/)


## Compile, upload and monitor ESP32 using the Arduino CLI

### Add secrets
Copy `arduino_secrets.h.example` to `arduino_secrets.h` and change the example secret values.

### Download and install Arduino IDE and/or CLI
[Arduino CLI](https://arduino.github.io/arduino-cli/) is an all-in-one solution that provides Boards/Library Managers, sketch builder, board detection, uploader, and many other tools needed to use any Arduino compatible board and platform from command line or machine interfaces.

### Initialiazies a Arduino CLI new configuration:
`arduino-cli config init`

### Add URL for Arduino core for the ESP32, ESP32-S2 and ESP32-C3
Edit `~/Library/Arduino15/arduino-cli.yaml` and add:

```
board_manager:
  additional_urls:
  - https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

### Update the core list
`arduino-cli core update-index`

### Install Arduino core for the ESP32, ESP32-S2 and ESP32-C3
`arduino-cli core install esp32:esp32`

### Compile sketch
`arduino-cli compile -b esp32:esp32:featheresp32 rf-gateway`

### Upload sketch to ESP32:
`arduino-cli upload -p /dev/cu.usbserial-0190DEAD -b esp32:esp32:featheresp32 rf-gateway`

### Open the serial monitor
`arduino-cli monitor -p /dev/cu.usbserial-0190DEAD -c baudrate=115200`
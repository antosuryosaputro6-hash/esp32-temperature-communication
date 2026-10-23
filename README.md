# esp32-temperature-communication
ESP32-based temperature data communication between melting and pouring areas using keypad input, ESP-NOW, and P5 LED display.
# ESP32 Temperature Communication

**Author:** Anto Suryo Saputro

## Project Description

This project develops a temperature data communication system between the melting and pouring areas in Plant 3 using ESP32, keypad input, ESP-NOW communication, and a P5 LED display.

## Hardware

- ESP32 transmitter
- ESP32-S3 receiver
- 4×4 keypad
- I2C LCD
- P5 HUB75 LED matrix display

## Source Code

- `PROGRAM_PENGIRIM.ino`: Reads manually entered temperature values from the keypad and sends the data through ESP-NOW.
- `PROGRAM_PENERIMA.ino`: Receives temperature data through ESP-NOW and displays the value on the P5 LED matrix.

## License

This project is distributed under the MIT License. See the `LICENSE` file for details.

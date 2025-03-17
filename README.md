# Electricity prices display demo using ESP32 and SSD1306

Fetching daily electricity prices from https://api.porssisahko.net/v1/latest-prices.json and displaying them on the display. Two buttons to cycle back and forward in time.

## Demo

https://github.com/user-attachments/assets/a26da37c-d063-4d56-8475-7a3c76a8d517

## Motivation

* To learn esp-idf, esp32 programming, freeRTOS tasks.
* Monitor the electricity spot price so I can save money on my electricity bill

## Current state

Working as intended but the project could probably be structured in a better way. This was my first esp project after I got a led blinking.

### Project structure

* components/ : all components added with 'ESP-IDF: Add Arduino as ESP32 as ESP-IDF Component' command
* managed_components/ : all components added via the ESP-IDF Component registry
* main/ : main project files
* main/main.c : entrypoint

## Requirements

* esp-idf v5.3.0

## Components used

1. ESP32 microcontroller
2. 2 buttons
3. Small plastic case
4. SSD1306 OLED screen

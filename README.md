# TTGO T-Display Alarm Clock

## Overview

This project implements a simple alarm clock for the TTGO T-Display board using PlatformIO and the ESP-IDF framework. The alarm clock displays the current time and allows setting alarms via MQTT messaging. When an alarm goes off, the screen flashes between red and green, and a GPIO output is activated until a button is pressed.

## Features

- **Time Display**: Shows the current time in hours, minutes, seconds, and am/pm format.
- **SNTP Time Sync**: Time is synchronized with an SNTP server similar to the `time_demo()` function in the demo code.
- **Alarm Setting**: Alarms can be set via MQTT message received from the `/a159236/alarm` topic in the format `YYYY-MM-DD HH:MM`.
- **Alarm Notification**: Displays the set alarm time on the screen when an alarm is set.
- **Alarm Trigger**: When an alarm goes off, the screen alternates between red and green every 300ms, and a GPIO output is activated until a button press.

## Setup

1. **Development Environment**: Set up PlatformIO with ESP-IDF following instructions provided in class.
   
2. **WiFi Connection**: Connect to the "MasseyWifi" access point with no password using the provided demo WiFi code.
   
3. **MQTT Configuration**: Use `mqtt.webhop.org` as the MQTT server. Subscribe to the `/a159236/alarm` topic to receive alarm time updates.
   
4. **Graphics Library**: Utilize the same graphics library as used in Assignment 2 for display and interface design.

## Instructions

1. **Setting Time**: Time is automatically synchronized with the SNTP server.
   
2. **Setting Alarms**: Send MQTT messages in the format `YYYY-MM-DD HH:MM` to `/a159236/alarm` to set alarms.
   
3. **Alarm Notification**: When an alarm is set, the screen displays the alarm time. When the alarm triggers, the screen flashes and GPIO output activates until a button is pressed.

## Testing

- **MQTT Explorer**: Use [MQTT Explorer](http://mqtt-explorer.com/) to test the functionality of the alarm clock, including setting alarms and observing alarm triggers.

# An ESP32-powered epaper display for daily events
**Work on this project is ongoing**
This project uses and ESP32 to talk to google calendar and an Epaper display to show a concise review of upcoming events and tasks. At its core, the ESP32 communicates with a Google Scripts deployment to collect event and task data, then parses this data and prints the information to the display. The project is designed to be cheap and accessible, using few hardware parts, and capable of used by any IDE that can communicate with an ESP32.

## To do list:
The goal of this project is for the ESP32 to:
- [x] Get events from google calendar
- [x] Get tasks from google tasks
- [x] Be battery powered
- [x] Display these events and other pertinent information on an Epaper display
- [x] Refresh once per day
- [ ] Refresh when user presses refresh button

Additional goals include:
- [ ] Battery algorithm
    - [ ] Design a better algorithm for getting and displaying voltage from battery
    - [ ] Utilize deep sleep when running on battery power but always on when plugged in
- [ ] Design a custom housing for the display
- [ ] Design a custom PCB to fit in the housing better
- [ ] Make the design scalable for other size displays

## Current capabilities:
**9/17/2026**: The project can get and display all pertinent information to the epaper display (some GUI clean-up and bug fixes still in progress). It also displays the current date and battery percentage. Next steps: have the processor go into deep sleep between refreshes to conserve battery power, and only refresh once per day or when the user presses a refresh button.

![Image of display with calendar events and to do tasks](doc/9_17_2026_status.jpeg)

## Hardware Requirements:
- Any ESP32 w/ Wifi capability
- Waveshare 5.83" black and white epaper display (https://www.waveshare.com/5.83inch-e-paper-hat.htm)
- 3.7V lithium-ion battery

## Credit:
This project takes inspiration from, uses or builds upon the following open-source repositories:
* [esp32-task-reminder-display](https://github.com/ISC-HEI/esp32-task-reminder-display) - Developed by [@ISC-HEI](https://github.com/ISC-HEI) and licensed under the [Apache License](https://github.com/ISC-HEI/esp32-task-reminder-display/blob/main/LICENSE).
* [Fridge-Calendar](https://github.com/0015/Fridge-Calendar) - Developed by [@0015](https://github.com/0015) and licensed under the [MIT License](https://github.com/0015/Fridge-Calendar/blob/main/LICENSE).
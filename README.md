[![Stand With Ukraine](https://raw.githubusercontent.com/vshymanskyy/StandWithUkraine/main/banner2-direct.svg)](https://vshymanskyy.github.io/StandWithUkraine/)

# Arduino-PPM-Generator
[![Build Status](https://img.shields.io/badge/PayPal-donate-green)](https://www.paypal.com/donate/?hosted_button_id=8NYXYJXL5J2Q6)

Sketch for Arduino Nano allows you to generate a PPM signal.
It uses hardware sixteen bit timer.
The pulse duration is adjusted with an accuracy of 0.0625 microseconds.

![gui screenshot](https://raw.githubusercontent.com/kolod/Arduino-PPM-Generator/master/gui.png)

## Modbus registers
| #   | Mode | Size (bytes) | Description                              |
|-----|------|--------------|------------------------------------------|
| 0   |  R   | 2            | Clock ticks per 1 microsecond            |
| 1   |  R   | 2            | Maximum channels count                   |
| 2   |  RW  | 2            | State 0 - Off / 1 - On / 2 - On inverted |
| 3   |  RW  | 2            | Channels count                           |
| 4   |  RW  | 2            | Pause length in the clock ticks          |
| 5,6 |  RW  | 4            | Synchro pulse length in the clock ticks  |
| 7   |  RW  | 2            | Channel #1 duration in the clock ticks   |
| ... |  ... | ...          | ...                                      |
| n+7 |  RW  | 2            | Channel #n duration in the clock ticks   |

![ppm](https://raw.githubusercontent.com/kolod/Arduino-PPM-Generator/master/ppm-dark.png#gh-dark-mode-only)
![ppm](https://raw.githubusercontent.com/kolod/Arduino-PPM-Generator/master/ppm-light.png#gh-light-mode-only)

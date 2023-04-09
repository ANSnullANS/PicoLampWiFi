# ANSnullANS' PicoLamp WiFi-Edition

**Warning**
This Project is Work-in-Progress and not yet ready for stable/production/non-developer use!

## Description

PicoLamp is an RGB-LED Lamp based on the Raspberry Pi PicoW capable of running in 3 different modes:

- Rainbow: Cycles through all Colors of the Rainbow with a smooth fade
- Static: Fixed Color
- Daylight: (warm) Daylight simulation

The modes are either selected via hardware-switch or via WiFi-Connection and Web-Page hosted directly on the PicoW.

This is mainly the WiFi-Extension of my other Project "PicoLamp", but contains way to many changes to run it in a branch/fork, hence the fully separated repo.

## BOM

### Electronics

Required Materials can be adjusted to your needs (number of LEDs, Switches,...) but for the code to run as-is, use the following:

| Amount | Part | Notes |
| --- | --- | --- |
| 1 | Raspberry Pi PicoW | |
| 1 | Bread- or Protoboard|2.54mm pitch |
| 1 | Logic Level Shifter|To convert the 3.3V from Pico to 5V for the NeoPixels |
| 1 | 47nF Capacitor | |
| 2 | 10K Ohm Resistor | |
| 1 | 2-way Switch with middle-off function | Or regular 2-way to run without Signature-Color |
| 1 | WS281* LED stripe | I'm using WS2813 by default due to the integrated backup-line |
| - | Jump-Wire | |
| - | Solder Equipment | |

Optional:
| Amount | Part | Notes |
| --- | --- | --- |
| 1 | USB-B PCB Connector | For direct Power-In to LEDs, when using large stripes |
| 1 | On/Off Switch | For power-control directly inside the Lamp |

### Enclosure

I recycled some Metal-Bottle-Cover with nice cutouts for the lampshade and 3d-printed the base enclosure to fit to the bottom.

The STL for the electronics-enclosure is added here - for the lampshade, you gotta be creative ;)

## Configuration

All configuration regarding hardware is done inside "config.h" file. Following variables are the most vital:

| Variable  | Description |
| --- | --- |
| LED_PIN | Number of the GPIO-Pin where the LED's data-pin is connected to. |
| NUM_PIXELS | Total amount of Pixels. |
| DAYLIGHT_PIN | GPIO-Pin where the Switch for Daylight-Enable is connected. |
| RAINBOW_PIN | GPIO-Pin where the Switch for Rainbow-Enable is connected. |
| RAINBOW_UPDATE_INTERVAL | Interval in milliseconds for fading between 2 colors of the rainbow. |

The Rainbow-Colors are not meant to be configurable but if you have to, check "Rainbow.h" for following parameters:

| Variable | Description |
| --- | --- |
| RAINBOWMAX | Amount of Colors in RainbowColors. |
| RainbowColors | Array of RGB-Values for the Rainbow-Animation. |

## Assembly

For the actual pinout, see the variables in 'Configuration' above. If I find some time, I'll add proper schematics later.

## Credits

Thanks to all my friends and supporters, as well as the authors of the following 3rd party libraries used by PicoLampWiFi:

pi-pico-adafruit-neopixels by martinkooij (https://github.com/martinkooij/pi-pico-adafruit-neopixels)
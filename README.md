# esp-homekit-demo
Forked demo of [Apple HomeKit accessory server
library](https://github.com/maximkulkin/esp-homekit) by Maxim Kulkin.

# speaker_selector
My first attempt at modifying Maxim's code (specifically the Sonoff S20 submodule) to use an ESP8266 NodeMCU development board as an A/B Speaker Selector.  I added a relay, a button, and an LED for each of A Speakers and B Speakers, which is intended to be wired to an optocoupler relay board for passing audio signals when relays are engaged.  

## Usage

See [build instructions](https://github.com/maximkulkin/esp-homekit-demo/wiki/Build-instructions).

My first learning experience with the ESP8266 was through [this video](https://www.youtube.com/watch?v=QBj8OLig8Kg).

## The quick start guide:

• go to terminal

• cd to folder with VagrantFile

• vagrant up (command)

• open VirtualBox

• add USB host controller in VirtualBox USB settings

• cd to folder with VagrantFile

• vagrant halt (command)

• vagrant up (command)

• vagrant ssh (command)

• usb-devices (command to see if USB on ESP8266 is recognized)

• cd to esp-homekit-demo

• git pull https://github.com/jngothia/esp-homekit-demo.git (command)

• make -C examples/speaker_selector test

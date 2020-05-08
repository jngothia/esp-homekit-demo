# esp-homekit-demo
Forked demo of [Apple HomeKit accessory server
library](https://github.com/maximkulkin/esp-homekit) by Maxim Kulkin.

# speaker_selector
My first attempt at modifying Maxim's code (specifically the Sonoff S20 submodule) to use and ESP8266 as a A/B Speaker Selector.  I added a relay, a button, and an LED for each of A Speakers and B Speakers.

## Usage

See [build instructions](https://github.com/maximkulkin/esp-homekit-demo/wiki/Build-instructions)

My first learning experience with the ESP8266 was through [this video](https://www.youtube.com/watch?v=QBj8OLig8Kg).

The quick start guide:

• go to terminal
• cd to folder with VagrantFile
• vagrant up (command)

• open VirtualBox
• add USB host controller in VirtualBox USB settings

• cd to folder with VagrantFile
• vagrant halt (command)
• vagrant up (command)
• vagrant ssh (command)
• cd to esp-homekit-demo
• git pull https://github.com/jngothia/esp-homekit-demo.git (command)
• make -C examples/speaker_selector test

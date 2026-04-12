I wanted reliable & simple standalone exmaples for [CardputerADV](https://docs.m5stack.com/en/core/Cardputer-Adv).

These are all verified working:

- [battery](./src/battery) - test battery state
- [button](./src/button) - test boot-button
- [display](./src/display) - test screen
- [ir](./src/ir) - test infared TX
- [keyboard](./src/keyboard) - test keyboard
- [motion](./src/motion) - test motion-sensors
- [sd](./src/sd) - test microSD
- [sound](./src/sound) - test microphone/speaker

These use [Lora Cap 1262](https://docs.m5stack.com/en/cap/Cap_LoRa-1262):

- [lora](./src/lora) - test LoRA
- [gps](./src/gps) - test GPS

You can get them in [releases](https://github.com/konsumer/cardputer-examples/releases/), if you want to use them with [m5 launcher](https://bmorcelli.github.io/Launcher/). Just put them on your SD card, and load it up.


## building

You will need to install platformio:

```sh
pip install platformio
```
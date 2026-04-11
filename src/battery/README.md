# Battery

This shows battery-level.

> [!NOTE]
> Due to hardware limitations, Cardputer and Cardputer-Adv cannot read battery charging status or battery current information. Please note that when connecting the Cardputer or Cardputer-Adv to a computer or power source, the switch on the upper side must be turned on to enable charging. Otherwise, the battery will be disconnected and the device will run on external power (showing 100%.)

```sh
pio run --target upload -e battery
```
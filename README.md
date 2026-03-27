I wanted minimal examples that work for CardputerADV (with lora cap) since I kept having issues with the [provided examples](https://docs.m5stack.com/en/arduino/m5cardputer/program).


I wanted a quick & simple example, just to make sure my device works.


You will need to install [platformio](https://platformio.org/install). I don't use VSCode, so I just did `pip install platfformio`

I like to wrap my examples in package.json, just so I don't have to remember how to call pio, but you can also just call the commands in there directly.

```sh
# lora radio receiver in US
npm run lora

# small gps demo that actually gets location
npm run gps

# if you need to look at serial-logging, I use some on gps
npm run serial
```

- GPS requierd me to go outside. It got a fix right away, and once it gets a fix, it gets it faster next time.
- Lora is pre-configured for rnode (reticulum) default settings in US. that is just what I had around to test, so it was easy to send messages to it.
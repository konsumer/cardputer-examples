# lora

This is an expansion of the UI idea in [keyboard](../keyboard/). It receives raw LoRA messages, and when you enter a message, it will send it.

```sh
pio run --target upload --target monitor -e lora
```

I didn't have 2 cardputers, so I just used [rnode](https://unsigned.io/rnode/) on a heltec v3 like this:

```sh
python src/lora/test.py
```

rnode is sort of like a LoRA modem, but you can test with 2 of the same device, too, if you've got them.
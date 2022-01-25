# pico_sequencer
Raspberry Pi Pico based simple eurorack sequencer.

As 2022-01-25 Just started to work on this. 

The hardware consists of:

- pimoroni **pico rgb keypad** board
- Raspberry **pi pico** board
- My own protoboard, which currently is just a 74HC595 chip driving 6 LEDs and eurorack mono plugs.

For development and debugging, I'm using Visual Studio Code and another Raspberry Pi Pico with the **Picoprobe** firmware. I am using a Mac for development, but this stuff should work on any computer capable of running the Pico SDK envinronment.

The code template is the **pimoroni** template for the **pico rgb board**. I removed some extra libraries to make the project nice and compact.

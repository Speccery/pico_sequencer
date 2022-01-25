
// seqdemo.cpp
// EP 2021-03-14 Erik edited from RGB keypad demo

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"

#include "pico_rgb_keypad.hpp"

using namespace pimoroni;

PicoRGBKeypad pico_keypad;

// Configure GPIO
const uint DATA_PIN=6;
const uint SHIFT_CLOCK_PIN=8;
const uint LATCH_CLOCK_PIN=7;

void config_my_gpio() {
  gpio_init(DATA_PIN);
  gpio_init(SHIFT_CLOCK_PIN);
  gpio_init(LATCH_CLOCK_PIN);
  gpio_set_dir(DATA_PIN, GPIO_OUT);
  gpio_set_dir(SHIFT_CLOCK_PIN, GPIO_OUT);
  gpio_set_dir(LATCH_CLOCK_PIN, GPIO_OUT);
  gpio_put(SHIFT_CLOCK_PIN, 0);
  gpio_put(LATCH_CLOCK_PIN, 0);
}

// Shift out 8 bits to 74HC595
void send_hc595(uint8_t v) {
  for(int i=0; i<8; i++) {
    gpio_put(DATA_PIN, (v >> 7) & 1);
    asm volatile("nop \n nop \n nop");
    asm volatile("nop \n nop \n nop");
    gpio_put(SHIFT_CLOCK_PIN, 1);
    asm volatile("nop \n nop \n nop");
    gpio_put(SHIFT_CLOCK_PIN, 0);
    asm volatile("nop \n nop \n nop");
    v <<= 1;
  }
  gpio_put(LATCH_CLOCK_PIN, 1);
  asm volatile("nop \n nop \n nop");
  gpio_put(LATCH_CLOCK_PIN, 0);
}


int main() {

  pico_keypad.init();
  pico_keypad.set_brightness(1.0f);
  config_my_gpio();

  send_hc595(0xFF);
  sleep_ms(100);
  send_hc595(0);

  uint16_t lit = 0;
  uint16_t last_button_states = 0;
  uint8_t colour_index = 0;

  uint8_t cursor = 0;   // Erik - my cursor, run through all 16 buttons
  uint8_t cursor_flash = 0;

  while(true) {
    // read button states from i2c expander
    uint16_t button_states = pico_keypad.get_button_states();

    if(last_button_states != button_states && button_states) {
      last_button_states = button_states;
      if(button_states) {
        if(lit == 0xffff) {
          // all buttons are already lit, reset the test
          lit = 0;
          colour_index++;
          if(colour_index >= 6) {
            colour_index = 0;
          }
        }else{
          uint8_t button = 0;
          for(uint8_t find = 0; find < pico_keypad.NUM_PADS; find++) {
            // check if this button is pressed and no other buttons are pressed
            if(button_states & 0x01) {
              if(!(button_states & (~0x01))) {
                lit |= 1 << button;
              }
              break;
            }
            button_states >>= 1;
            button++;
          }
        }
      }
    }
    send_hc595( lit  & 0xFF);

    last_button_states = button_states;

    for(uint8_t i = 0; i < PicoRGBKeypad::NUM_PADS; i++) {
      if((lit >> i) & 0x01) {
        switch(colour_index)
        {
          case 0: pico_keypad.illuminate(i, 0x00, 0x20, 0x00);  break;
          case 1: pico_keypad.illuminate(i, 0x20, 0x20, 0x00);  break;
          case 2: pico_keypad.illuminate(i, 0x20, 0x00, 0x00);  break;
          case 3: pico_keypad.illuminate(i, 0x20, 0x00, 0x20);  break;
          case 4: pico_keypad.illuminate(i, 0x00, 0x00, 0x20);  break;
          case 5: pico_keypad.illuminate(i, 0x00, 0x20, 0x20);  break;
        }
      } else {
        pico_keypad.illuminate(i, 0x05, 0x05, 0x05);
      }
    }

    // Show my cursor: Flash the cursor key in two different colors depending on cursor_flash.
    // uint8_t k = cursor_flash ? 255 : 0;
    pico_keypad.illuminate(cursor, 0, cursor_flash ? 255 : 100, 0);

    pico_keypad.update();

    sleep_ms(100);
    if(cursor_flash) {
      if(++cursor == PicoRGBKeypad::NUM_PADS) {
        cursor = 0;
      }
    }
    cursor_flash ^= 1;
  }

  return 0;
}

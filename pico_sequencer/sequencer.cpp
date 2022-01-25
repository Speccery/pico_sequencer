
// sequencer.cpp
// Started EP 2022-01-22 for real. Based on the Pimoroni "pico RGB keypad" board.

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include "pico/stdlib.h"

#include "pico_rgb_keypad.hpp"

using namespace pimoroni;

// For starters, let's make the sequences 16 steps long.
// Let's do 8 sequencer lanes, even if my proto board only has 6 outputs.

const int SEQUENCES=8;
uint16_t sequences[SEQUENCES];
uint16_t sample_pos = 0;

int active_displayed_seq = 2; // Show sequence 2

PicoRGBKeypad pico_keypad;

// Configure GPIO to drive the 74HC595 shift register.
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


/**
 * @brief Return true when an eurorack clock has arrived.
 *  or during development when 100ms has passed :)
 * 
 * @return true 
 * @return false 
 */
bool is_eurorack_clock() {
  static uint64_t last_time=0;
  absolute_time_t t = get_absolute_time();
  uint64_t now = to_us_since_boot(t);
  if (now - last_time >= 100000 ) {
    // It's been over 100ms, so this is our fake clock pulse. 
    last_time = now;
    return true;
  }
  return false;
}

/**
 * @brief returns true if 100ms has passed
 * 
 * @return true 
 * @return false 
 */
bool is_keyscan_time() {
  static uint64_t last_time=0;
  absolute_time_t t = get_absolute_time();
  uint64_t now = to_us_since_boot(t);
  if (now - last_time >= 100000 ) {
    // It's been over 100ms, so time to check keys. 
    last_time = now;
    return true;
  }
  return false;  
}

/**
 * @brief Update the pad leds to show the current sequence.
 *  Also show the cursor.
 * 
 */
void show_active_sequence() {
  uint16_t lit = sequences[active_displayed_seq];
  for(uint8_t i = 0; i < PicoRGBKeypad::NUM_PADS; i++) {
    if((lit >> i) & 0x01) 
      pico_keypad.illuminate(i, 0x20, 0x20, 0x00); 
    else
      pico_keypad.illuminate(i, 5, 5, 5);
  }

  // Show current position by overriding the step.
  pico_keypad.illuminate(sample_pos, 0, 100, 0);  
  pico_keypad.update();
}

int main() {

  pico_keypad.init();
  pico_keypad.set_brightness(1.0f);
  config_my_gpio();

  send_hc595(0xFF);
  sleep_ms(100);
  send_hc595(0);

  memset(sequences, 0, sizeof(sequences));

  uint16_t last_button_states = 0;

  while(true) {
    // If eurorack clock pulse, forward the clock and process the sequences.
    if(is_eurorack_clock()) {
      uint8_t out=0;
      for(int i=0; i<SEQUENCES; i++) {
        if(sequences[i] & (1 << sample_pos)) {
          // this step is active.
          out |= 1 << i;
        }
      }
      if(++sample_pos >= 16)
        sample_pos = 0;
      send_hc595(out);

      // Update the display.
      show_active_sequence();
    } 

    if(!is_keyscan_time())
      continue;

    // read button states from i2c expander
    uint16_t button_states = pico_keypad.get_button_states();

    if(last_button_states != button_states && button_states) {
      //uint8_t button = 0;
      for(uint8_t u = 0; u < pico_keypad.NUM_PADS; u++) {
        // Check if a button is pressed. If so, toggle the step state.
        if((button_states & (1 << u)) && !(last_button_states & (1 << u))) {
          sequences[active_displayed_seq] ^= 1 << u;
        }
      }
      last_button_states = button_states;
    }
  }

  return 0;
}

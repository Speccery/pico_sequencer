add_executable(
  sequencer
  sequencer.cpp
)

# Pull in pico libraries that we need
target_link_libraries(sequencer pico_stdlib pico_rgb_keypad)

# create map/bin/hex file etc.
pico_add_extra_outputs(sequencer)

include(${CMAKE_CURRENT_LIST_DIR}/../../drivers/st7789/st7789.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../pico_graphics/pico_graphics.cmake)

add_library(pico_display INTERFACE)

target_sources(pico_display INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/pico_display.cpp
)

target_include_directories(pico_display INTERFACE ${CMAKE_CURRENT_LIST_DIR})

# Pull in pico libraries that we need
target_link_libraries(pico_display INTERFACE pico_stdlib hardware_spi hardware_pwm hardware_dma st7789 pico_graphics)
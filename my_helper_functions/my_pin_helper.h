#include <nrf.h>

// toggle "pin_nr"
void my_gpio_pin_toggle(uint32_t pin_nr)
{
  uint32_t pins_state = NRF_GPIO->OUT;                  // read out current state of GPIO pins
  NRF_GPIO->OUTSET = (~pins_state & 1UL << pin_nr);     // toggle "set" bit on "pin_nr"
  NRF_GPIO->OUTCLR = ( pins_state & 1UL << pin_nr);     // toggle "clear" bit on "pin_nr"
}

void my_gpio_pin_on(uint32_t pin_nr)
{
  NRF_GPIO->OUTSET = (1UL << pin_nr);
}

void my_gpio_pin_off(uint32_t pin_nr)
{
  NRF_GPIO->OUTCLR = (1UL << pin_nr);
}

// configure "pin_nr" as output pin with standard settings
void my_gpio_pin_output(uint32_t pin_nr)
{
  NRF_GPIO->PIN_CNF[pin_nr] = (GPIO_PIN_CNF_DIR_Output     << GPIO_PIN_CNF_DIR_Pos)   |       // direction: output
                              (GPIO_PIN_CNF_DRIVE_S0S1     << GPIO_PIN_CNF_DRIVE_Pos) |       // standard drive strength
                              (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos) |       // connect input buffer
                              (GPIO_PIN_CNF_PULL_Disabled  << GPIO_PIN_CNF_PULL_Pos)  |       // no pull
                              (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) ;       // sense disabled
} 

// configure "pin_nr" as input pin with standard settings
void my_gpio_pin_input(uint32_t pin_nr)
{
  NRF_GPIO->PIN_CNF[pin_nr] = (GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos)   |       // direction: input
                              (GPIO_PIN_CNF_DRIVE_S0S1     << GPIO_PIN_CNF_DRIVE_Pos) |       // standard drive strength
                              (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos) |       // connect input buffer
                              (GPIO_PIN_CNF_PULL_Disabled  << GPIO_PIN_CNF_PULL_Pos)  |       // no pull
                              (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) ;       // sense disabled
}

// configure "pin_nr" as input with pullup, rest is standard
void my_gpio_pin_input_pullup(uint32_t pin_nr)
{
  NRF_GPIO->PIN_CNF[pin_nr] = (GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos)   |       // direction: input
                              (GPIO_PIN_CNF_DRIVE_S0S1     << GPIO_PIN_CNF_DRIVE_Pos) |       // standard drive strength
                              (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos) |       // connect input buffer
                              (GPIO_PIN_CNF_PULL_Pullup    << GPIO_PIN_CNF_PULL_Pos)  |       // Pull up on pin
                              (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) ;       // sense disabled
}

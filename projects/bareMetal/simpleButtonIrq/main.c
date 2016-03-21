#include <nrf.h>
#include "my_pin_helper.h"

#define MY_LED_1			          (22UL)      // general status LED
#define MY_LED_2   	  	        (23UL)      // Button-push LED, connected to MY_BUTTON_1 via PPI
#define MY_LED_3   	  	        (24UL)      // Button-push LED, connected to MY_BUTTON via GPIOTE Irq Handler

#define MY_BUTTON_1		          (13UL)      // Button, controls MY_LED_2 and MY_LED_3 in two different ways (PPI and Irq)

#define GPIOTE_CHANNEL_BUTTON   (0UL)       // GPIOTE channel for MY_BUTTON_1
#define GPIOTE_CHANNEL_LED      (1UL)       // GPIOTE channel for MY_LED_2

#define PPI_CHANNEL_BUTTON_LED  (0UL)       // PPI channel to connect GPIOTE_CHANNEL_BUTTON with GPIOTE_CHANNEL_LED


// overload GPIOTE IRQ handler
void GPIOTE_IRQHandler(void)
{	
  // Event related to button "MY_BUTTON_1" push
	if (NRF_GPIOTE->EVENTS_IN[GPIOTE_CHANNEL_BUTTON] == 1)
  {
		NRF_GPIOTE->EVENTS_IN[GPIOTE_CHANNEL_BUTTON] = 0;         // clear event register (otherwise IRQ keeps on firing)   
    my_gpio_pin_toggle(MY_LED_3);                             // toggle MY_LED_3
	}
}

static void config_leds(void)
{
	// -- configure LEDs as outputs
  my_gpio_pin_output(MY_LED_1);
  my_gpio_pin_output(MY_LED_2);
  my_gpio_pin_output(MY_LED_3); 
  
  // Startup status for LEDs
	my_gpio_pin_on(MY_LED_1);
  my_gpio_pin_off(MY_LED_2);
  my_gpio_pin_off(MY_LED_3);
}

static void config_buttons(void)
{
  // -- configure button as input
  my_gpio_pin_input_pullup(MY_BUTTON_1);
}

static void config_gpiote(void)
{
	// Configure GPIOTE_CHANNEL_NUMBER to generate an event when MY_BUTTON_1 is pushed
	NRF_GPIOTE->CONFIG[GPIOTE_CHANNEL_BUTTON] = ( GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos )          | 
																							( MY_BUTTON_1 << GPIOTE_CONFIG_PSEL_Pos )                       |
																							( GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos ) ;
	
	NRF_GPIOTE->CONFIG[GPIOTE_CHANNEL_LED] =    ( GPIOTE_CONFIG_MODE_Task << GPIOTE_CONFIG_MODE_Pos )           | 
																							( MY_LED_2 << GPIOTE_CONFIG_PSEL_Pos )                          |
																							( GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos ) | 
                                              ( GPIOTE_CONFIG_OUTINIT_Low << GPIOTE_CONFIG_OUTINIT_Pos)       ;
	
	// enable IRQ generation on GPIOTE_CHANNEL_BUTTON event
	NRF_GPIOTE->INTENSET = 1UL << GPIOTE_CHANNEL_BUTTON;
	
	// enable GPIOTE IRQs
	NVIC_EnableIRQ(GPIOTE_IRQn);
}

static void config_ppi(void)
{
  // Configure PPI channel PPI_CHANNEL_BUTTON_LED to toggle MY_LED_2 when MY_BUTTON_1 is pushed
  NRF_PPI->CH[PPI_CHANNEL_BUTTON_LED].EEP = (uint32_t)(&NRF_GPIOTE->EVENTS_IN[GPIOTE_CHANNEL_BUTTON]);
  NRF_PPI->CH[PPI_CHANNEL_BUTTON_LED].TEP = (uint32_t)(&NRF_GPIOTE->TASKS_OUT[GPIOTE_CHANNEL_LED]);
	
	// Enable PPI channel PPI_CHANNEL_BUTTON_LED.
  NRF_PPI->CHEN = (1UL << PPI_CHANNEL_BUTTON_LED);
}

int main(void)
{
  // -- configure everything
  config_leds();      
  config_buttons();  
  config_gpiote();
	config_ppi();
  	
	// we're done, the rest is run via PPI and IRQs
	while(1)
	{
		__WFI();
	}
}


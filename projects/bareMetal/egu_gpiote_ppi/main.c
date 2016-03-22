#include <nrf.h>
#include "my_pin_helper.h"

#define MY_LED_1			          (22UL)      // general status LED
#define MY_LED_2   	  	        (23UL)      // EGU0-led, triggered via PPI (EGU0->GPIOTE)

#define GPIOTE_CHANNEL_LED      (0UL)       // GPIOTE channel for MY_LED_2

#define PPI_CHANNEL_EGU_LED     (0UL)       // PPI channel to connect EGU0 with MY_LED_2

static void config_leds(void)
{
	// -- configure LEDs as outputs
  my_gpio_pin_output(MY_LED_1);
  my_gpio_pin_output(MY_LED_2); 
  
  // Startup status for LEDs
	my_gpio_pin_on(MY_LED_1);
  my_gpio_pin_off(MY_LED_2);
}

//// configure EGU0 to generate event
//static void config_egu0(void)
//{
//  // enable IRQ generation for event0 triggered
//  NRF_EGU0->INTENSET = EGU_INTEN_TRIGGERED0_Enabled << EGU_INTEN_TRIGGERED0_Pos;
//  // enable EGU0 IRQs
//	NVIC_EnableIRQ(SWI0_EGU0_IRQn);  
//}

static void config_gpiote(void)
{
	// GPIOTE channel to toggle MY_LED_2
	NRF_GPIOTE->CONFIG[GPIOTE_CHANNEL_LED] =    ( GPIOTE_CONFIG_MODE_Task << GPIOTE_CONFIG_MODE_Pos )           | 
																							( MY_LED_2 << GPIOTE_CONFIG_PSEL_Pos )                          |
																							( GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos ) | 
                                              ( GPIOTE_CONFIG_OUTINIT_Low << GPIOTE_CONFIG_OUTINIT_Pos)       ;
}

static void config_ppi(void)
{
  // Configure PPI channel PPI_CHANNEL_EGU_LED to toggle MY_LED_2 when EGU0 generates event0
  NRF_PPI->CH[PPI_CHANNEL_EGU_LED].EEP = (uint32_t)(&NRF_EGU0->EVENTS_TRIGGERED[0]);
  NRF_PPI->CH[PPI_CHANNEL_EGU_LED].TEP = (uint32_t)(&NRF_GPIOTE->TASKS_OUT[GPIOTE_CHANNEL_LED]);
	
	// Enable PPI channel PPI_CHANNEL_BUTTON_LED.
  NRF_PPI->CHEN = (1UL << GPIOTE_CHANNEL_LED);
}

int main(void)
{
  // -- configure everything
  config_leds();      
  config_gpiote();
	config_ppi();
  
  // trigger Task0 in EGU0 to trigger the IRQ handler of EGU0
  NRF_EGU0->TASKS_TRIGGER[0] = 1;
  	
	// we're done, the rest is run via PPI and IRQs
	while(1)
	{
    uint32_t volatile tmo;
    
    tmo = 10000000;
    while (tmo--);
    // trigger Task0 in EGU0 to trigger the IRQ handler of EGU0
    NRF_EGU0->TASKS_TRIGGER[0] = 1;
	}
}


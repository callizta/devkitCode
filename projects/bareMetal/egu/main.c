#include <nrf.h>
#include "my_pin_helper.h"

#define MY_LED_1			          (22UL)      // general status LED
#define MY_LED_2   	  	        (23UL)      // EGU0-led, triggered by IRQ handler of EGU0

// overload EGU0 IRQ handler
void SWI0_EGU0_IRQHandler(void)
{	
  // Event related to button "MY_BUTTON_1" push
	if (NRF_EGU0->EVENTS_TRIGGERED[0] == 1)
  {
		NRF_EGU0->EVENTS_TRIGGERED[0] = 0;         // clear event register (otherwise IRQ keeps on firing)   
    my_gpio_pin_toggle(MY_LED_2);              // toggle MY_LED_2
	}
}

static void config_leds(void)
{
	// -- configure LEDs as outputs
  my_gpio_pin_output(MY_LED_1);
  my_gpio_pin_output(MY_LED_2); 
  
  // Startup status for LEDs
	my_gpio_pin_on(MY_LED_1);
  my_gpio_pin_off(MY_LED_2);
}

// configure EGU0 to generate event
static void config_egu0(void)
{
  // enable IRQ generation for event0 triggered
  NRF_EGU0->INTENSET = EGU_INTEN_TRIGGERED0_Enabled << EGU_INTEN_TRIGGERED0_Pos;
  // enable EGU0 IRQs
	NVIC_EnableIRQ(SWI0_EGU0_IRQn);  
}


int main(void)
{
  // -- configure everything
  config_leds();      
  config_egu0();
  
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


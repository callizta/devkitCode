#include <nrf.h>


#define MY_LED1		(22UL)
#define MY_LED2   (23UL)

int main(void)
{

	// Configure GPIO pin as output with standard drive strength.
  NRF_GPIO->PIN_CNF[MY_LED1] = (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos) |
                               (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
                               (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
                               (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
                               (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
  
	
	while(1)
	{
		//__WFI();
	}
}
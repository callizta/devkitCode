#include <nrf.h>


#define MY_LED_1				(22UL)
#define MY_LED_2   	  	(23UL)

#define MY_BUTTON_1		    (13UL)

#define GPIOTE_CHANNEL_NUMBER   (0UL)

static uint16_t MY_LED_2_ON = 0;

void GPIOTE_IRQHandlerMine(void)
{	
	if (NRF_GPIOTE->EVENTS_IN[GPIOTE_CHANNEL_NUMBER] == 1)
	{
		NRF_GPIOTE->EVENTS_IN[GPIOTE_CHANNEL_NUMBER] = 0;
		if(MY_LED_2_ON == 1)	
		{
			NRF_GPIO->OUTSET = (0UL << MY_LED_2);
			MY_LED_2_ON = 0;
		}
		else  
		{
			NRF_GPIO->OUTSET = (1UL << MY_LED_2);
			MY_LED_2_ON = 1;
		}			
	}
}

int main(void)
{

	// Configure MY_LED_1 pin as output with standard drive strength.
  NRF_GPIO->PIN_CNF[MY_LED_1] = (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos) |
                                (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
                                (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
                                (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
                                (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
  
	// Configure MY_LED_2 pin as output with standard drive strength.
  NRF_GPIO->PIN_CNF[MY_LED_2] = (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos) |
                                (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
                                (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
                                (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
                                (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);	
	
	// Configure MY_BUTTON_1 as input	
	NRF_GPIO->PIN_CNF[MY_BUTTON_1] = (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos) |
                                   (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
                                   (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
                                   (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
                                   (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
	
	
	// Configure GPIOTE_CHANNEL_NUMBER to generate an event when MY_BUTTON_1 is pushed
	NRF_GPIOTE->CONFIG[GPIOTE_CHANNEL_NUMBER] = ( GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos ) | 
																							( MY_BUTTON_1 << GPIOTE_CONFIG_PSEL_Pos ) |
																							( GPIOTE_CONFIG_POLARITY_HiToLo << GPIOTE_CONFIG_POLARITY_Pos ) ;
	
		
	NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_IN0_Set << GPIOTE_INTENSET_IN0_Pos;
	
	NVIC_EnableIRQ(GPIOTE_IRQn);
		
	// switch on LED_1
	NRF_GPIO->OUTSET = (1UL << MY_LED_1);
	
	// switch off LED_2
	NRF_GPIO->OUTSET = (0UL << MY_LED_2);
	
	
	while(1)
	{
		__WFI();
	}
}

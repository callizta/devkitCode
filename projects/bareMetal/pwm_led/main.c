#include <nrf.h>
#include "my_pin_helper.h"
#include <math.h>

#define STATUS_LED			          (22UL)      // general status LED
#define PWM_LED1     	  	        (23UL)      // PWM-LED1
#define PWM_LED2     	  	        (24UL)      // PWM-LED1

// -- PWM defines
#define PWM0_PRESCALER          PWM_PRESCALER_PRESCALER_DIV_128     // 125kHz
#define	PWM0_COUNTERTOP 		    1600                               // counter top
#define PM0_LOOPS               10                                  // Number of loops
#define LED_WAIT                10

#define SINE_GENERATOR_SR       100
#define SINE_GENERATOR_FREQ     1
#define LIN_RISE_BUFFER_SIZE    10

#define LED_MIN                 1000
#define LED_MAX                 1600

                            
uint16_t pwm_led_linRise[LIN_RISE_BUFFER_SIZE];
uint16_t pwm_led_linFall[LIN_RISE_BUFFER_SIZE];
                            
static void config_leds(void)
{
	// -- configure LEDs as outputs
  my_gpio_pin_output(STATUS_LED);
  my_gpio_pin_output(PWM_LED1); 
  my_gpio_pin_output(PWM_LED2);
  
  // Startup status for LEDs
	my_gpio_pin_on(STATUS_LED);
  my_gpio_pin_off(PWM_LED1);
  my_gpio_pin_off(PWM_LED2);
}

void makeLinRiseAndFall(void)
{
	int n;
	for (n = 0; n < LIN_RISE_BUFFER_SIZE; n++)
	{
		pwm_led_linFall[n] = PWM0_COUNTERTOP - (PWM0_COUNTERTOP/LIN_RISE_BUFFER_SIZE) * n;
    pwm_led_linRise[n] = (PWM0_COUNTERTOP/LIN_RISE_BUFFER_SIZE) * n;
	}
}


static void config_pwm()
{
    
  // generate PWM values
  makeLinRiseAndFall();
  
  // PWM0  
	NRF_PWM0->PSEL.OUT[0] = (PWM_LED1 << PWM_PSEL_OUT_PIN_Pos)
                        | (PWM_PSEL_OUT_CONNECT_Connected << PWM_PSEL_OUT_CONNECT_Pos);
  
	// Mode: Up counter - edge aligned PWM duty-cycle
	NRF_PWM0->MODE = (PWM_MODE_UPDOWN_Up << PWM_MODE_UPDOWN_Pos);
	
	// Prescaler: 1, i.e. 16MHz
	NRF_PWM0->PRESCALER = (PWM0_PRESCALER << PWM_PRESCALER_PRESCALER_Pos);
	
	// Decoder configuration
  NRF_PWM0->DECODER = (PWM_DECODER_LOAD_Common << PWM_DECODER_LOAD_Pos)
                    | (PWM_DECODER_MODE_RefreshCount << PWM_DECODER_MODE_Pos);

	// PWM sampling rate
	NRF_PWM0->COUNTERTOP = (PWM0_COUNTERTOP << PWM_COUNTERTOP_COUNTERTOP_Pos);
	 
  // "Sequence" to play (which is only one sample for now
  NRF_PWM0->SEQ[0].CNT = ((sizeof(pwm_led_linFall) / sizeof(uint16_t)) << PWM_SEQ_CNT_CNT_Pos);       // length of pwm_buffer
  NRF_PWM0->SEQ[0].PTR = (uint32_t)&pwm_led_linFall[0];		                                            // pointer to pwm_buf in the memory
  NRF_PWM0->SEQ[0].ENDDELAY = 0;	                                                                    // waiting time between the sequences, not relevant here since we only play one sequence
  NRF_PWM0->SEQ[0].REFRESH = LED_WAIT;                                                                // Amount of additional PWM periods between samples (i.e. prolonging each sample)  

  NRF_PWM0->SEQ[1].CNT = ((sizeof(pwm_led_linRise) / sizeof(uint16_t)) << PWM_SEQ_CNT_CNT_Pos);       // length of pwm_buffer
  NRF_PWM0->SEQ[1].PTR = (uint32_t)&pwm_led_linRise[0];		                                            // pointer to pwm_buf in the memory
  NRF_PWM0->SEQ[1].ENDDELAY = 0;	                                                                    // waiting time between the sequences, not relevant here since we only play one sequence
  NRF_PWM0->SEQ[1].REFRESH = LED_WAIT;                                                                // Amount of additional PWM periods between samples (i.e. prolonging each sample) 

//   NRF_PWM1->SHORTS = (PWM_SHORTS_LOOPSDONE_SEQSTART0_Enabled << PWM_SHORTS_LOOPSDONE_SEQSTART0_Pos);

  NRF_PWM0->LOOP = (4 << PWM_LOOP_CNT_Pos);

	// Enable
	NRF_PWM0->ENABLE = (PWM_ENABLE_ENABLE_Enabled << PWM_ENABLE_ENABLE_Pos);
}


int main(void)
{
  // -- configure everything
  makeLinRiseAndFall();
  config_leds(); 
  config_pwm();
  
  // start PWM
  NRF_PWM0->TASKS_SEQSTART[0] = 1;
  	
	// we're done, the rest is run via PPI and IRQs
	while(1)
	{
		__WFI();
	}
}


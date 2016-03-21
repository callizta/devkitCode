#include <nrf.h>
#include "my_pin_helper.h"

#define STATUS_LED              (22UL)                              // general status LED
#define PWM_LED                 (23UL)                              // PWM-LED1
#define PWM_IRQ_LED             (26UL)                              // PWM-IRQ-LED1

// -- PWM defines
#define PWM0_PRESCALER          PWM_PRESCALER_PRESCALER_DIV_64      // 250kHz
#define PWM0_COUNTERTOP         1600                                // counter top
#define PM0_LOOPS               1                                   // Number of loops
#define LED_WAIT                0                                   // Number of samples to insert after each sample - Prolong the current sample
#define PWM0_ENDDELAY           0

#define LIN_RISE_BUFFER_SIZE    256                                 // Number of samples - Resolution of the fade  
                            
uint16_t pwm_led_linRise[LIN_RISE_BUFFER_SIZE];                     // PWM buffer for fade-in values
uint16_t pwm_led_linFall[LIN_RISE_BUFFER_SIZE];                     // PWM buffer for fade-out values

uint16_t currPwmMax = 0;                                            // current PWM max value, used to re-calculate the fade values in the IRQ handler

// update currPwmMax, and wrap around
void updateCurrPwmMax(void)
{
  currPwmMax = currPwmMax + 400;
  if (currPwmMax >= PWM0_COUNTERTOP) currPwmMax = 0;
}

// calculate fade-in values, max-val determines the maximum brightness (0 = maximum, COUNTERTOP = minimum)
void calcFadeIn (uint16_t max_val)
{
  uint16_t n;
  uint16_t max_amplitude = PWM0_COUNTERTOP - max_val;
  for (n = 0; n < LIN_RISE_BUFFER_SIZE; n++)
  {
    // linear fade in
    pwm_led_linRise[n] = PWM0_COUNTERTOP - (uint16_t)(((float)max_amplitude/(LIN_RISE_BUFFER_SIZE-1)) * n);
  }
}

// calculate fade-out values, max-val determines the maximum brightness (0 = maximum, COUNTERTOP = minimum)
void calcFadeOut (uint16_t max_val)
{
  uint16_t n;
  uint16_t max_amplitude = PWM0_COUNTERTOP - max_val;
  for (n = 0; n < LIN_RISE_BUFFER_SIZE; n++)
  {
    // linear fade out
    pwm_led_linFall[n] = max_val + (uint16_t) (((float)max_amplitude/(LIN_RISE_BUFFER_SIZE-1))*n);
  }
}

// overload PWM0 IRQ handler
void PWM0_IRQHandler(void)
{	
  // Event related to seq0 done
	if (NRF_PWM0->EVENTS_SEQEND[0] == 1)
  {
		NRF_PWM0->EVENTS_SEQEND[0] = 0;        // clear event register (otherwise IRQ keeps on firing)
    calcFadeIn (currPwmMax);
    my_gpio_pin_toggle(PWM_IRQ_LED);       // toggle PWM_IRQ_LED
	}
  // Event related to seq1 done
  if (NRF_PWM0->EVENTS_SEQEND[1] == 1)
  {
		NRF_PWM0->EVENTS_SEQEND[1] = 0;        // clear event register (otherwise IRQ keeps on firing)   
    calcFadeOut (currPwmMax);
    my_gpio_pin_toggle(PWM_IRQ_LED);       // toggle PWM_IRQ_LED
	}
  // Event related to loop done
	if (NRF_PWM0->EVENTS_LOOPSDONE == 1)
  {
		NRF_PWM0->EVENTS_LOOPSDONE = 0;        // clear event register (otherwise IRQ keeps on firing)
    updateCurrPwmMax();                    // calculate new pwm maximum
	}  
}

// -- configure LEDs
static void config_leds(void)
{
  // configure LEDs as outputs
  my_gpio_pin_output(STATUS_LED);
  my_gpio_pin_output(PWM_LED); 
  my_gpio_pin_output(PWM_IRQ_LED);
  
  // startup status for LEDs
  my_gpio_pin_on(STATUS_LED);
  my_gpio_pin_off(PWM_LED);
  my_gpio_pin_off(PWM_IRQ_LED);
}




// -- generate linear fade in and fade out values
// the typecasting from uint16_t to float and back to uint16_t is required, otherwise we won't end up at the maximum value due to rounding errors
void makeLinRiseAndFall(uint16_t max_val)
{
  calcFadeIn(max_val);
  calcFadeOut(max_val);
}

// -- configure PWM0 to play the fade-in/face-out sequence with the defined settings
static void config_pwm()
{    
  // PWM0 output: PWM_LED
  NRF_PWM0->PSEL.OUT[0] = (PWM_LED << PWM_PSEL_OUT_PIN_Pos)
                        | (PWM_PSEL_OUT_CONNECT_Connected << PWM_PSEL_OUT_CONNECT_Pos);
  
  // Mode: Up counter - edge aligned PWM duty-cycle
  NRF_PWM0->MODE = (PWM_MODE_UPDOWN_Up << PWM_MODE_UPDOWN_Pos);
  
  // Prescaler
  NRF_PWM0->PRESCALER = (PWM0_PRESCALER << PWM_PRESCALER_PRESCALER_Pos);
  
  // Decoder configuration
  NRF_PWM0->DECODER = (PWM_DECODER_LOAD_Common << PWM_DECODER_LOAD_Pos)                               // load the same values to all decoder channels
                    | (PWM_DECODER_MODE_RefreshCount << PWM_DECODER_MODE_Pos);                        // SEQ[n].REFRESH is used to determine loading internal compare registers

  // PWM sampling rate
  NRF_PWM0->COUNTERTOP = (PWM0_COUNTERTOP << PWM_COUNTERTOP_COUNTERTOP_Pos);                          
   
  // sequence 0 -> fade in
  NRF_PWM0->SEQ[0].CNT = ((sizeof(pwm_led_linRise) / sizeof(uint16_t)) << PWM_SEQ_CNT_CNT_Pos);       // length of pwm_buffer
  NRF_PWM0->SEQ[0].PTR = (uint32_t)&pwm_led_linRise[0];                                               // pointer to pwm_buf in the memory
  NRF_PWM0->SEQ[0].ENDDELAY = 0;                                                                      // waiting time between the sequences, not relevant here since we only play one sequence
  NRF_PWM0->SEQ[0].REFRESH = LED_WAIT;                                                                // Amount of additional PWM periods between samples (i.e. prolonging each sample)  

  // sequence 1 -> fade out
  NRF_PWM0->SEQ[1].CNT = ((sizeof(pwm_led_linFall) / sizeof(uint16_t)) << PWM_SEQ_CNT_CNT_Pos);       // length of pwm_buffer
  NRF_PWM0->SEQ[1].PTR = (uint32_t)&pwm_led_linFall[0];                                               // pointer to pwm_buf in the memory
  NRF_PWM0->SEQ[1].ENDDELAY = PWM0_ENDDELAY;                                                          // waiting time between the sequences, not relevant here since we only play one sequence
  NRF_PWM0->SEQ[1].REFRESH = LED_WAIT;                                                                // Amount of additional PWM periods between samples (i.e. prolonging each sample) 

  // start seq0 again when the loop is done, i.e. go on forever
  NRF_PWM0->SHORTS = PWM_SHORTS_LOOPSDONE_SEQSTART0_Enabled << PWM_SHORTS_LOOPSDONE_SEQSTART0_Pos;

  // number of loops
  NRF_PWM0->LOOP = (PM0_LOOPS << PWM_LOOP_CNT_Pos);

  // Enable PWM0
  NRF_PWM0->ENABLE = (PWM_ENABLE_ENABLE_Enabled << PWM_ENABLE_ENABLE_Pos);
  
  // configure Event 
  NRF_PWM0->INTENSET = PWM_INTEN_SEQEND0_Enabled << PWM_INTEN_SEQEND0_Pos | 
                       PWM_INTEN_SEQEND1_Enabled << PWM_INTEN_SEQEND1_Pos ;
  
  // Enable Irq generation for PWM0
  NVIC_EnableIRQ(PWM0_IRQn);
}


int main(void)
{
  // configure everything  
  config_leds(); 
  config_pwm();
  
  // calculate initial fade-in/fade-out values
  makeLinRiseAndFall(currPwmMax);
  
  // start PWM
  NRF_PWM0->TASKS_SEQSTART[0] = 1;
  updateCurrPwmMax();
    
  // we're done, the rest is run via PWM0 and IRQs
  while(1)
  {
    __WFI();
  }
}


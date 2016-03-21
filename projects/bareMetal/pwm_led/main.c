#include <nrf.h>
#include "my_pin_helper.h"

#define STATUS_LED             (22UL)                               // general status LED
#define PWM_LED1                (23UL)                               // PWM-LED1

// -- PWM defines
#define PWM0_PRESCALER          PWM_PRESCALER_PRESCALER_DIV_64      // 250kHz
#define  PWM0_COUNTERTOP         1600                                // counter top
#define PM0_LOOPS               2                                   // Number of loops
#define LED_WAIT                0                                    // Number of samples to insert after each sample - Prolong the current sample

#define LIN_RISE_BUFFER_SIZE    256                                  // Number of samples - Resolution of the fade  
                            
uint16_t pwm_led_linRise[LIN_RISE_BUFFER_SIZE];                     // PWM buffer for fade-in values
uint16_t pwm_led_linFall[LIN_RISE_BUFFER_SIZE];                     // PWM buffer for fade-out values

// -- configure LEDs
static void config_leds(void)
{
  // configure LEDs as outputs
  my_gpio_pin_output(STATUS_LED);
  my_gpio_pin_output(PWM_LED1); 
  
  // startup status for LEDs
  my_gpio_pin_on(STATUS_LED);
  my_gpio_pin_off(PWM_LED1);
}

// -- generate linear fade in and fade out values
// the typecasting from uint16_t to float and back to uint16_t is required, otherwise we won't end up at the maximum value due to rounding errors
void makeLinRiseAndFall(void)
{
  uint16_t n;
  for (n = 0; n < LIN_RISE_BUFFER_SIZE; n++)
  {
    // linear fade in
    pwm_led_linRise[n] = PWM0_COUNTERTOP - (uint16_t)(((float)PWM0_COUNTERTOP/(LIN_RISE_BUFFER_SIZE-1)) * n);
    // linear fade out
    pwm_led_linFall[n] = (uint16_t)  (((float)PWM0_COUNTERTOP/(LIN_RISE_BUFFER_SIZE-1))*n);
  }
}

// -- configure PWM0 to play the fade-in/face-out sequence with the defined settings
static void config_pwm()
{    
  // PWM0 output: PWM_LED1
  NRF_PWM0->PSEL.OUT[0] = (PWM_LED1 << PWM_PSEL_OUT_PIN_Pos)
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
  NRF_PWM0->SEQ[0].PTR = (uint32_t)&pwm_led_linRise[0];                                                // pointer to pwm_buf in the memory
  NRF_PWM0->SEQ[0].ENDDELAY = 0;                                                                      // waiting time between the sequences, not relevant here since we only play one sequence
  NRF_PWM0->SEQ[0].REFRESH = LED_WAIT;                                                                // Amount of additional PWM periods between samples (i.e. prolonging each sample)  

  // sequence 1 -> fade out
  NRF_PWM0->SEQ[1].CNT = ((sizeof(pwm_led_linFall) / sizeof(uint16_t)) << PWM_SEQ_CNT_CNT_Pos);       // length of pwm_buffer
  NRF_PWM0->SEQ[1].PTR = (uint32_t)&pwm_led_linFall[0];                                                // pointer to pwm_buf in the memory
  NRF_PWM0->SEQ[1].ENDDELAY = 0;                                                                      // waiting time between the sequences, not relevant here since we only play one sequence
  NRF_PWM0->SEQ[1].REFRESH = LED_WAIT;                                                                // Amount of additional PWM periods between samples (i.e. prolonging each sample) 

  // no shortcuts
  NRF_PWM1->SHORTS = 0;

  // number of loops
  NRF_PWM0->LOOP = (PM0_LOOPS << PWM_LOOP_CNT_Pos);

  // Enable PWM0
  NRF_PWM0->ENABLE = (PWM_ENABLE_ENABLE_Enabled << PWM_ENABLE_ENABLE_Pos);
}


int main(void)
{
  // configure everything  
  config_leds(); 
  config_pwm();
  
  // calculate fade-in/fade-out values
  makeLinRiseAndFall();
  
  // start PWM
  NRF_PWM0->TASKS_SEQSTART[0] = 1;
    
  // we're done, the rest is run via PPI and IRQs
  while(1)
  {
    __WFI();
  }
}


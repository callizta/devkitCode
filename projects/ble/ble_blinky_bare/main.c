/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** 
 * @brief Blinky Sample Application main file.
 *
 * This file contains the source code for a sample server application using the LED Button service.
 */

#include <stdint.h>
#include <string.h>
#include "math.h"
#include "bleConnection.h" 

#include "nrf_gpiote.h"
#include "nrf_gpio.h"
#include "boards.h"
//#include "nrf_drv_ppi.h"
//#include "nrf_drv_gpiote.h"
//#include "app_error.h"

#define LED_MINE_A                      (22UL)
#define LED_MINE_B                      (23UL)
#define LED_MINE_C                      (24UL)
#define LED_MINE_D                      (25UL)
																					
#define PWM_PIN0 												(11UL)
#define PWM_PIN1 												(12UL)

#define LEDBUTTON_BUTTON1               BSP_BUTTON_1

#define GPIOTE_CHANNEL_NUMBER           (0)
																					
// -- PWM defines
// Sampling reate
// 512 : 31.25kHz
// 256 : 62.5 kHz
#define	PWM0_COUNTERTOP 		512
#define PM0_LOOPS           1

// -- sine wave generate defines
#define SINE_GENERATOR_SR   62500
#define SINE_GENERATOR_FREQ 880
#define SINE_BUFFER_SIZE    SINE_GENERATOR_SR/SINE_GENERATOR_FREQ
//#define SINE_BUFFER_SIZE    SINE_GENERATOR_SR/4

// PWM stuff
int16_t buf[] = {(1 << 15) | 1500 , (1 << 15) | 15000}; // Inverse polarity (bit 15), 1500us duty cycle
int16_t buf1[] = {(1 << 15) | 15000};
int16_t buf2[] = {1250, 1250};

uint16_t buffer[SINE_BUFFER_SIZE];
uint16_t buffer0[SINE_BUFFER_SIZE];
uint16_t buffer1[SINE_BUFFER_SIZE];

uint16_t frequency = 880;
bool bufferIndex = false;

bool soundPlaying = false;

void makeSine(bool bufferIndex)
{
	uint16_t sampleRate = SINE_GENERATOR_SR;
	float    amplitude = 0x00F0;
	float    offset    = 0xFA;
	//uint16_t frequency = SINE_GENERATOR_FREQ;
	for (int n = 0; n < SINE_BUFFER_SIZE; n++)
	{
		buffer[n] = (uint16_t) (offset + (amplitude * sin((2 * 3.14 * n * frequency) / sampleRate)));
		if (!bufferIndex ) buffer0[n] = (uint16_t) (offset + (amplitude * sin((2 * 3.14 * n * frequency) / sampleRate)));
		else               buffer1[n] = (uint16_t) (offset + (amplitude * sin((2 * 3.14 * n * frequency) / sampleRate)));
	}
}

void PWM0_IRQHandler(void)
{	
	
	if (NRF_PWM0->EVENTS_SEQEND[0] == 1)
	{
		NRF_PWM0->EVENTS_SEQEND[0] = 0;
		nrf_gpio_pin_toggle(LED_MINE_A);
		//makeSine(false);		
		//frequency++;
	}
	if (NRF_PWM0->EVENTS_SEQEND[1] == 1)
	{
		NRF_PWM0->EVENTS_SEQEND[1] = 0;
		nrf_gpio_pin_toggle(LED_MINE_A);
		//makeSine(true);		
		//frequency++;
	}
}

void GPIOTE_IRQHandlerMine(void)
{	
	if (NRF_GPIOTE->EVENTS_IN[GPIOTE_CHANNEL_NUMBER] == 1)
	{
		NRF_GPIOTE->EVENTS_IN[GPIOTE_CHANNEL_NUMBER] = 0;
		nrf_gpio_pin_toggle(LED_MINE_B);
	}
}

// PWM general init
static void prepare_for_pwm(void)
{
	// Start accurate HFCLK (XOSC)
//  NRF_CLOCK->TASKS_HFCLKSTART = 1;
//  while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) ;
//  NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
	
	// Configure PWM_PIN as output, and set it to 0
	nrf_gpio_cfg_output(PWM_PIN1);
	
	//makeSine(false);
	//frequency++;
	makeSine(true);
	//frequency++;
}

// PWM0 config
static void cfg_pwm0(void)
{
	// Enable Interrupt for the PWM0
	NVIC_EnableIRQ(PWM0_IRQn);      
	// Enable Interrupt generation for SEQEND0 event 
	NRF_PWM0->INTENSET = PWM_INTENSET_SEQEND0_Set << PWM_INTENSET_SEQEND0_Pos | 
											 PWM_INTENSET_SEQEND1_Set << PWM_INTENSET_SEQEND1_Pos ;
	
	// PWM0  
	NRF_PWM0->PSEL.OUT[0] =   PWM_PIN1;

	// Mode: Up counter - edge aligned PWM duty-cycle
	NRF_PWM0->MODE = (PWM_MODE_UPDOWN_Up << PWM_MODE_UPDOWN_Pos);
	
	// Prescaler: 1, i.e. 16MHz
	NRF_PWM0->PRESCALER = (PWM_PRESCALER_PRESCALER_DIV_1 << PWM_PRESCALER_PRESCALER_Pos);
	
	// Decoder configuration
  NRF_PWM0->DECODER = (PWM_DECODER_LOAD_Common << PWM_DECODER_LOAD_Pos) | (PWM_DECODER_MODE_RefreshCount << PWM_DECODER_MODE_Pos);

	// PWM sampling rate
	NRF_PWM0->COUNTERTOP = (PWM0_COUNTERTOP << PWM_COUNTERTOP_COUNTERTOP_Pos);
	
	// Shortcut: Start back sequence 0 again after the loop is done, i.e. keep going forever
	NRF_PWM0->SHORTS = PWM_SHORTS_LOOPSDONE_SEQSTART0_Enabled << PWM_SHORTS_LOOPSDONE_SEQSTART0_Pos;
	
	// Enable
	NRF_PWM0->ENABLE = (PWM_ENABLE_ENABLE_Enabled << PWM_ENABLE_ENABLE_Pos);
	
}

static void cfg_pwm1(void)
{
// PWM1
	//NRF_PWM1->PRESCALER   = (PWM_PRESCALER_PRESCALER_DIV_2 << PWM_PRESCALER_PRESCALER_Pos);
  NRF_PWM1->PRESCALER   = PWM_PRESCALER_PRESCALER_DIV_2; // 1 us
  NRF_PWM1->PSEL.OUT[0] = PWM_PIN1;
  NRF_PWM1->MODE        = (PWM_MODE_UPDOWN_Up << PWM_MODE_UPDOWN_Pos);
  NRF_PWM1->DECODER     = (PWM_DECODER_LOAD_Common       << PWM_DECODER_LOAD_Pos) | 
                          (PWM_DECODER_MODE_RefreshCount << PWM_DECODER_MODE_Pos);
  NRF_PWM1->LOOP        = (PWM_LOOP_CNT_Disabled << PWM_LOOP_CNT_Pos);  
  NRF_PWM1->COUNTERTOP  = 8000; // 400HZ
	
	NRF_PWM1->SEQ[0].CNT = ((sizeof(buffer) / sizeof(uint16_t)) << PWM_SEQ_CNT_CNT_Pos); 
  NRF_PWM1->SEQ[0].ENDDELAY = 0;
	NRF_PWM1->SEQ[0].PTR = (uint32_t)&buffer[0];		// anders' code
  NRF_PWM1->SEQ[0].REFRESH = 0;

	NRF_PWM1->SHORTS = 0;
  
	NRF_PWM1->ENABLE = 1;
}

// PWM config
static void cfg_pwm(void)
{	
	cfg_pwm0();
	cfg_pwm1();
}

// my config
static void myCfg_LedsInit(void)
{	
	// make all LEDS outputs
	nrf_gpio_cfg_output(LED_MINE_A);
	nrf_gpio_cfg_output(LED_MINE_B);
	nrf_gpio_cfg_output(LED_MINE_C);
	nrf_gpio_cfg_output(LED_MINE_D);
	// switch off all LEDs
	nrf_gpio_pins_clear( LED_MINE_A | LED_MINE_B | LED_MINE_C | LED_MINE_D );	
}




static void playPwm(uint8_t note)
{

	switch (note){
		case 5: 
			//NRF_PWM0->TASKS_SEQSTART[0] = 1;	
			//makeSine(false);
			NRF_PWM0->TASKS_STOP = 1;	
			buf2[0] = 1700;
			NRF_PWM1->TASKS_SEQSTART[0] = 1;	
			break;
		case 6:
			NRF_PWM1->TASKS_STOP = 1;	
			break;
		case 7:
			NRF_PWM0->TASKS_STOP = 1;	
			buf2[0] = 1250;
			NRF_PWM1->TASKS_SEQSTART[0] = 1;	
			break;
		case 8:
			NRF_PWM1->TASKS_STOP = 1;	
			break;
		default:
			break;
	}
}


static void playA1(void)
{
	if(!soundPlaying)
	{
		makeSine(true);

		NRF_PWM0->SEQ[0].PTR = (uint32_t)&buffer[0];
		NRF_PWM0->SEQ[0].CNT = ((sizeof(buffer) / sizeof(uint16_t)) << PWM_SEQ_CNT_CNT_Pos);
		NRF_PWM0->SEQ[0].REFRESH = 0;
		NRF_PWM0->SEQ[0].ENDDELAY = 0;

		NRF_PWM0->SEQ[1].PTR = (uint32_t)&buffer[0];
		NRF_PWM0->SEQ[1].CNT = ((sizeof(buffer) / sizeof(uint16_t)) << PWM_SEQ_CNT_CNT_Pos);
		NRF_PWM0->SEQ[1].REFRESH = 0;
		NRF_PWM0->SEQ[1].ENDDELAY = 0;
		
		//NRF_PWM0->LOOP = (PWM_LOOP_CNT_Disabled << PWM_LOOP_CNT_Pos);
		NRF_PWM0->LOOP = (PM0_LOOPS << PWM_LOOP_CNT_Pos);
		NRF_PWM0->TASKS_SEQSTART[0] = 1;
		soundPlaying = true;
	}
	else
	{
		NRF_PWM0->TASKS_STOP = 1;
		soundPlaying = false;
	}
}


/**@brief Function for handling write events to the LED characteristic.
 *
 * @param[in] p_lbs     Instance of LED Button Service to which the write applies.
 * @param[in] led_state Written/desired state of the LED.
 */
static void led_write_handler(ble_lbs_t * p_lbs, uint8_t led_state)
{
	printf("\r\nReceived: %d\r\n",led_state);
	
		switch (led_state){
			case 1:
				nrf_gpio_pin_toggle(LED_MINE_A);
				break;
			case 2:
				nrf_gpio_pin_toggle(LED_MINE_B);
				break;
			case 3:
				nrf_gpio_pin_toggle(LED_MINE_C);
				break;
			case 4:
				nrf_gpio_pin_toggle(LED_MINE_D);
				break;
			case 5:
				playPwm(led_state);
				break;
			case 6:
				playPwm(led_state);
				break;
			case 7:
				playPwm(led_state);
				break;
			case 8:
				playPwm(led_state);
				break;
//			case 9:
//				playPiano();
//				break;
			case 10:
				playA1();
				break;

			default:    
				break;
				//LEDS_OFF(LED_MINE_A | LED_MINE_B | LED_MINE_C | LED_MINE_D);
    }
}

static void button_setup()
{
	// define LEDBUTTON_BUTTON1 as input
	//NRF_GPIO->DIRSET = (0UL << LEDBUTTON_BUTTON1);
	//nrf_gpio_cfg_input(LEDBUTTON_BUTTON1, NRF_GPIO_PIN_NOPULL);
	
	nrf_gpio_cfg_sense_input(LEDBUTTON_BUTTON1, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_LOW);
	
	nrf_gpiote_event_configure (GPIOTE_CHANNEL_NUMBER,LEDBUTTON_BUTTON1, NRF_GPIOTE_POLARITY_HITOLO );
	
	// enable IRQ generation for event
	NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_IN0_Set << GPIOTE_INTENSET_IN0_Pos;
	
	// enable GPIOTE IRQ
  //NVIC_EnableIRQ(GPIOTE_IRQn);
	
//	    // This pin is used for waking up from System OFF and is active low, enabling sense capabilities.
//    nrf_gpio_cfg_sense_input(PIN_GPIO_WAKEUP, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_LOW);
//	// Configure GPIO_OUTPUT_PIN_NUMBER as an output.
//    // nrf_gpio_cfg_input(PIN_GPIO_WAKEUP,NRF_GPIO_PIN_PULLUP);
//	nrf_gpiote_event_config (GPIOTE_CHANNEL_NUMBER,PIN_GPIO_WAKEUP, NRF_GPIOTE_POLARITY_HITOLO );
//	
//	
//	
//    uint32_t input_evt_addr;
//    uint32_t gpiote_task_addr;
//    nrf_ppi_channel_t ppi_channel;
//    
//    // Configure GPIOTE OUT task
//    nrf_drv_gpiote_out_config_t output_config =
//    {
//        .init_state     = NRF_GPIOTE_INITIAL_VALUE_HIGH,
//        .task_pin       = true,                                                                       \
//        .action         = NRF_GPIOTE_POLARITY_TOGGLE
//    };
//    APP_ERROR_CHECK(nrf_drv_gpiote_out_init(LED_MINE_B, &output_config));
//	
//    // Configure GPIOTE IN event
//    nrf_drv_gpiote_in_config_t input_config = 
//    {    
//        .is_watcher     = false,
//        .hi_accuracy    = true,
//        .pull           = NRF_GPIO_PIN_PULLUP,
//        .sense          = NRF_GPIOTE_POLARITY_TOGGLE       
//    };
//    APP_ERROR_CHECK(nrf_drv_gpiote_in_init(LEDBUTTON_BUTTON1, &input_config, NULL));
//    
//    // Get the instance allocated for both OUT task and IN event
//    gpiote_task_addr = nrf_drv_gpiote_out_task_addr_get(LED_MINE_B);
//    input_evt_addr = nrf_drv_gpiote_in_event_addr_get(LEDBUTTON_BUTTON1);

//    // Get a PPI channel from the PPI pool
//    APP_ERROR_CHECK(nrf_drv_ppi_channel_alloc(&ppi_channel));        
//    // Tie the GPIOTE IN event and OUT task through allocated PPI channel
//    APP_ERROR_CHECK(nrf_drv_ppi_channel_assign(ppi_channel, input_evt_addr, gpiote_task_addr));
//    // Enable the allocated PPI channel
//    APP_ERROR_CHECK(nrf_drv_ppi_channel_enable(ppi_channel));    
//    
//    // Enable OUT task and IN event
//    nrf_drv_gpiote_out_task_enable(LED_MINE_B);
//    nrf_drv_gpiote_in_event_enable(LEDBUTTON_BUTTON1, false);    
}

/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    uint32_t       err_code;
    ble_lbs_init_t init;

    init.led_write_handler = led_write_handler;

    err_code = ble_lbs_init(&m_lbs, &init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for application main entry.
 */
int main(void)
{
    // Initialize.
		init_uart();
    ble_leds_init();	
		button_setup();
		//
    timers_init();
		//buttons_init();
    ble_stack_init();
    gap_params_init();
    services_init();
    advertising_init();
    conn_params_init();
	
		// Start execution.
    advertising_start();
	
	  // execute my config
		myCfg_LedsInit();	
		prepare_for_pwm();
		cfg_pwm();	

    

    // Enter main loop.
    for (;;)
    {
        power_manage();
    }
}


/**
 * @}
 */

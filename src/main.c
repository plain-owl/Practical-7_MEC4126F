//********************************************************************
//*                    MEC4126F C template                           *
//*==================================================================*
//* WRITTEN BY: James Hepworth   	                 	             *
//* DATE CREATED: 2025/04/08                                         *
//*==================================================================*
//* PROGRAMMED IN: Visual Studio Code                                *
//* TARGET:        STM32F051                                         *
//*==================================================================*
//* DESCRIPTION:     Template for MEC4126F C Practicals              *
//********************************************************************

//====================================================================
// INCLUDE FILES
//====================================================================

#define STM32F051
#include "stm32f0xx.h"											   
#include "lcd_stm32f0.h"
#include <stdio.h>
#include <stdint.h>

//====================================================================
// GLOBAL CONSTANTS
//====================================================================
const float K_p = 50.0; // proportional gain
const float I = 1.0; // integrator constant
const float T_s = 0.001; // frequency of interrupt is 1 kHz
const float max_command = 7.0; // maximum voltage allowable
const float min_command = -7.0; // minimum voltage allowable 
//====================================================================
// GLOBAL VARIABLES
//====================================================================
volatile float u_p = 0.0; // previous control action
volatile float e_p = 0.0; // previous error 
//====================================================================
// FUNCTION DECLARATIONS
//====================================================================
void init_ADC(void);
void init_tim3(void);
void init_tim6(void);
void TIM6_IRQHandler(void);
float PI_control(float command, float feedback);
//void ADC_readings(uint16_t *ch5, uint16_t *ch6);
//====================================================================
// MAIN FUNCTION
//====================================================================

int main (void)
{
    init_ADC();
    init_tim3();
    init_tim6();

    while (1);
}							
// End of main

//====================================================================
// FUNCTION DEFINITIONS
//====================================================================
void init_ADC(void){
    RCC->APB2ENR |= RCC_APB2ENR_ADCEN; // enable ADC clock 
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN; // enable Port A clock for PA6 and PA

    GPIOA->MODER &= ~(GPIO_MODER_MODER5|GPIO_MODER_MODER6); // reset pins 5 and 6
    GPIOA->MODER |= (GPIO_MODER_MODER5|GPIO_MODER_MODER6); // set pins 5 and 6 to analogue mode

    ADC1->CHSELR |= (ADC_CHSELR_CHSEL5|ADC_CHSELR_CHSEL6); // select channel 5, connected to PA5 and select channel 6, connected to PA6
    ADC1->CFGR1 &= ~ADC_CFGR1_CONT; // set ADC to single conversion mode
    ADC1->CFGR1 &= ~ADC_CFGR1_SCANDIR; // set scan direction upwards, so channel 5 read first then channel 6
    ADC1->CFGR1 &= ~ADC_CFGR1_RES; // set ADC resolution to 12 bit,  0.088 degrees per step approximately (assuming about 360 degrees full rotation of pot)

    //NVIC_EnableIRQ(ADC1_IRQn); // Enable interrupt handler 
    ADC1->CR |= ADC_CR_ADEN; // Set ADEN=1 in ADC_CR register, starts ADC
    while((ADC1 -> ISR & ADC_ISR_ADRDY) == 0); // Wait for ADC to be ready to start converting

}
// END OF init_ADC 

//void ADC_readings(uint16_t *ch5, uint16_t *ch6){
    // Read channel 5
    //ADC1->CHSELR = ADC_CHSELR_CHSEL5;
    //ADC1->CR |= ADC_CR_ADSTART;
    //while (!(ADC1->ISR & ADC_ISR_EOC));
    //*ch5 = ADC1->DR;  // 
    //ADC1->ISR |= ADC_ISR_EOC;  // clear flag after reading channel 5, prepares for next reading 
    
    // Read channel 6
    //ADC1->CHSELR = ADC_CHSELR_CHSEL6;
    //ADC1->CR |= ADC_CR_ADSTART;
    //while (!(ADC1->ISR & ADC_ISR_EOC));
    //*ch6 = ADC1->DR;  // 
    //ADC1->ISR |= ADC_ISR_EOC; // clear flag after reading channel 6
//}

void init_tim3(void){
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN; // enable port B clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; // enable timer 3 clock

    GPIOB->MODER &= ~(GPIO_MODER_MODER4); // reset pin 4 
    GPIOB->MODER |=  (GPIO_MODER_MODER4_1);  // AF mode
    GPIOB->AFR[0] |= (1 << (4 * 4)); // Set PB4 to alternate function 1

    TIM3 ->ARR = 399; 
    TIM3-> PSC = 0;
    TIM3->CCR1 = 200; // initial duty cycle of 50%

    TIM3->CCMR1 &= ~ (TIM_CCMR1_CC1S); // set channel 1 to output
    TIM3->CCMR1 |= (TIM_CCMR1_OC1PE); // enable preload 
    TIM3->CCMR1 |= (TIM_CCMR1_OC1M_1|TIM_CCMR1_OC1M_2); // set channel 1 to pwm mode 1
    TIM3->CCER |= TIM_CCER_CC1E; // enable channel 1 signal ouput on PB4
    TIM3->CCER &= ~TIM_CCER_CC1P; // channel configured to active high - default
    TIM3->CR1 |= TIM_CR1_CEN; //start timer

}
// END OF init_tim3 

void init_tim6(void){
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN; // enable clock for basic timer 6
    TIM6->DIER |= TIM_DIER_UIE; // update interrupt enabled 
    TIM6->PSC = 7;
    TIM6->ARR = 999; // gives 1 kHz freqeuncy - timer interrupt period is 1 ms

    //TIM6->CR1 |= TIM_CR1_OPM; ask about where this is useful
    TIM6->CR1 |= TIM_CR1_CEN; // enable counter 
    NVIC_EnableIRQ(TIM6_IRQn); // Enable interrupt handler
    //TIM6->DIER &= ~TIM_DIER_UIE; 
}
// END OF init_tim6

float PI_control(float command, float feedback){
    float error_current = command - feedback; // computes the current error between the command and feedback values
    float u = (K_p * (((2 + I * T_s) * error_current) + ((I * T_s - 2) * e_p)) + (2 * u_p)) / 2; // difference equation

    if (u > max_command){
        u = max_command; // avoid saturation? 
    }

    else if (u < min_command){
        u = min_command; 
    }
    e_p = error_current; 
    u_p = u; 

    return u;
}
// END  OF PI_control


//====================================================================
// INTERRUPT SERVICE ROUTINES
//====================================================================
void TIM6_IRQHandler(void){
    if (TIM6->SR & TIM_SR_UIF){
        TIM6->SR &= ~TIM_SR_UIF;   // clear interrupt flag
        uint16_t channel5_reading, channel6_reading; 
        float voltage_req;
        ADC1->CR |= ADC_CR_ADSTART;
        while (!(ADC1->ISR & ADC_ISR_EOC));   // channel 5 result
        channel5_reading = ADC1->DR;
        while (!(ADC1->ISR & ADC_ISR_EOC));   // channel 6 result
        channel6_reading = ADC1->DR;
        //position_act = (channel5_reading / 4096.0f) * 3.3; // converts the ADC value read from channel 5 (PA5) into the feedback voltage
        //position_des = (channel6_reading / 4096.0f) * 3.3 ; // converts the ADC value read from channel 6 (PA6) into the control voltage 
        voltage_req = PI_control(channel6_reading, channel5_reading); // required control action as a voltage 
        float duty = 0.5f + (voltage_req / (2.0f * max_command));
        if (duty > 1.0f) duty = 1.0f;
        if (duty < 0.0f) duty = 0.0f;
        TIM3->CCR1 = (uint16_t)(duty * TIM3->ARR); // converts the output of PI_control to a number between 0 - 1 (duty) then to a value for ccr1 
        
    }
        
} 

//********************************************************************
// END OF PROGRAM
//********************************************************************
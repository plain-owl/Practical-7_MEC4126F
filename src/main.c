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
//void ADC1_COMP_IRQHandler(void);
void TIM6_IRQHandler(void);
float PI_control(float command, float feedback);
//====================================================================
// MAIN FUNCTION
//====================================================================

int main (void)
{
    init_ADC;
    init_tim3;
    init_tim6;
    ADC1->CR |= ADC_CR_ADSTART; 

    while (1)
    {
    }

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

    ADC1->CHSELR |= ADC_CHSELR_CHSEL5; // select channel 5, connected to PA5
    ADC1->CHSELR |= ADC_CHSELR_CHSEL6; //select channel 6, connected to PA6
    ADC1->CFGR1 |= ADC_CFGR1_CONT; // set ADC to continuous mode
    ADC1->CFGR1 &= ~ADC_CFGR1_RES; // set ADC resolution to 12 bit,  0.088 degrees per step approximately (assuming about 360 degrees full rotation of pot)
    ADC1->CFGR1 |= ADC_CFGR1_WAIT;

    //ADC1->IER |= ADC_IER_EOCIE; 
    //NVIC_EnableIRQ(ADC1_IRQn); // Enable interrupt handler 
    ADC1->CR |= ADC_CR_ADEN; // Set ADEN=1 in ADC_CR register, starts ADC
    while((ADC1 -> ISR & ADC_ISR_ADRDY) == 0); // Wait for ADC to be ready to start converting

}
// END OF init_ADC 

void init_tim3(void){
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN; // enable port B clock

    GPIOB->MODER &= ~(GPIO_MODER_MODER4); // reset pin 4 
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
//====================================================================
// INTERRUPT SERVICE ROUTINES
//====================================================================
void TIM6_IRQHandler(void){
    if (TIM6->SR & TIM_SR_UIF) { // check flag
        TIM6->SR &= ~TIM_SR_UIF; // clear flag
        float position_des = ADC1->DR;
        float position_act = ADC1->DR;
        float voltage_req = PI_control(position_des, position_act); // required control action as a voltage 
        TIM3->CCR1 = (uint16_t)((voltage_req + max_command/(min_command + max_command)) * TIM3->ARR); // converts the output of PI_control to a number between 0 - 1 (duty) then to a value for ccr1 
        
}

//********************************************************************
// END OF PROGRAM
//********************************************************************
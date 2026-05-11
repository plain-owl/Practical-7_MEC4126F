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

//====================================================================
// GLOBAL VARIABLES
//====================================================================

//====================================================================
// FUNCTION DECLARATIONS
//====================================================================
void init_ADC(void);
void init_tim3(void);
//====================================================================
// MAIN FUNCTION
//====================================================================

int main (void)
{
    init_ADC;
    init_tim3;
    
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
    ADC1->CFGR1 |= ADC_CFGR1_RES_1; // set ADC resolution to 8 bit, 0.0659 degrees per step approximately (assuming about 270 degrees full rotation of pot)
    ADC1->CFGR1 |= ADC_CFGR1_WAIT;

    ADC1->IER |= ADC_IER_EOCIE; 
    NVIC_EnableIRQ(ADC1_IRQn); // Enable interrupt handler 
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
//====================================================================
// INTERRUPT SERVICE ROUTINES
//====================================================================


//********************************************************************
// END OF PROGRAM
//********************************************************************
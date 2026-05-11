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
void init_PB4(void);
//====================================================================
// MAIN FUNCTION
//====================================================================

int main (void)
{

    
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
    ADC1->CFGR1 |= ADC_CFGR1_RES_1; // set ADC resolution to 8 bit
    ADC1->CFGR1 |= ADC_CFGR1_WAIT;

    ADC1->IER |= ADC_IER_EOCIE; 
    NVIC_EnableIRQ(ADC1_IRQn); // Enable interrupt handler 
    ADC1->CR |= ADC_CR_ADEN; // Set ADEN=1 in ADC_CR register, starts ADC
    while((ADC1 -> ISR & ADC_ISR_ADRDY) == 0); // Wait for ADC to be ready to start converting

}
// END OF init_ADC 


//====================================================================
// INTERRUPT SERVICE ROUTINES
//====================================================================


//********************************************************************
// END OF PROGRAM
//********************************************************************
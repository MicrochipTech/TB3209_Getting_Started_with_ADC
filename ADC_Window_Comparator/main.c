/*
    \file   main.c

    \brief  ADC window comparator

    (c) 2018 Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip software and any
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party
    license terms applicable to your use of third party software (including open source software) that
    may accompany Microchip software.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS
    FOR A PARTICULAR PURPOSE.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS
    SOFTWARE.
*/

#define WINDOW_CMP_LOW_TH_EXAMPLE    (0x100)

#include <avr/io.h>
#include <stdbool.h>

uint16_t adcVal;

void ADC0_init(void);
uint16_t ADC0_read(void);
void ADC0_start(void);
bool ADC0_conersionDone(void);
bool ADC0_resultBelowTreshold(void);
void ADC0_clearWindowCmpIntFlag(void);
void LED0_init(void);
void LED0_on(void);
void LED0_off(void);

void ADC0_init(void)
{
    /* Disable digital input buffer */
    PORTD.PIN6CTRL &= ~PORT_ISC_gm;
    PORTD.PIN6CTRL |= PORT_ISC_INPUT_DISABLE_gc;
    
    /* Disable pull-up resistor */
    PORTD.PIN6CTRL &= ~PORT_PULLUPEN_bm;

    ADC0.CTRLC = ADC_PRESC_DIV4_gc      /* CLK_PER divided by 4 */
               | ADC_REFSEL_INTREF_gc;  /* Internal reference */
    
    ADC0.CTRLA = ADC_ENABLE_bm          /* ADC Enable: enabled */
               | ADC_RESSEL_10BIT_gc;   /* 10-bit mode */
    
    /* Select ADC channel */
    ADC0.MUXPOS  = ADC_MUXPOS_AIN6_gc;
    
    /* Set conversion window comparator low threshold */
    ADC0.WINLT = WINDOW_CMP_LOW_TH_EXAMPLE;
    
    /* Set conversion window mode */
    ADC0.CTRLE = ADC_WINCM_BELOW_gc;
    
    /* Enable FreeRun mode */
    ADC0.CTRLA |= ADC_FREERUN_bm;
}

uint16_t ADC0_read(void)
{
    /* Clear the interrupt flag by writing 1: */
    ADC0.INTFLAGS = ADC_RESRDY_bm;
    
    return ADC0.RES;
}

void ADC0_start(void)
{
    /* Start conversion */
    ADC0.COMMAND = ADC_STCONV_bm;
}

bool ADC0_conersionDone(void)
{
    return (ADC0.INTFLAGS & ADC_RESRDY_bm);
}

bool ADC0_resultBelowTreshold(void)
{
    return (ADC0.INTFLAGS & ADC_WCMP_bm);
}

void ADC0_clearWindowCmpIntFlag(void)
{
    /* Clear the interrupt flag by writing 1: */
    ADC0.INTFLAGS = ADC_WCMP_bm;
}

void LED0_init(void)
{
    /* Make High (OFF) */
    PORTB.OUT |= PIN5_bm;
    /* Make output */
    PORTB.DIR |= PIN5_bm;
}

void LED0_on(void)
{
    /* Make Low (ON) */
    PORTB.OUT &= ~PIN5_bm;
}

void LED0_off(void)
{
    /* Make High (OFF) */
    PORTB.OUT |= PIN5_bm;
}

int main(void)
{
    ADC0_init();
    LED0_init();
    
    ADC0_start();
        
    while(1)
    {
        if (ADC0_conersionDone())
        {        
            if(ADC0_resultBelowTreshold())
            {
                LED0_on();
                ADC0_clearWindowCmpIntFlag();
            }
            else
            {
                LED0_off();
            }
            
            adcVal = ADC0_read();
        }
    }
}

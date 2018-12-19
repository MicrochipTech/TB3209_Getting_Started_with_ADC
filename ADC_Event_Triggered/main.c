/*
    \file   main.c

    \brief  ADC triggered by RTC Overflow

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

/* RTC Period */
#define RTC_PERIOD            (511)

#include <avr/io.h>
#include <avr/interrupt.h>

uint16_t adcVal;

void ADC0_init(void);
void LED0_init(void);
void LED0_toggle(void);
void RTC_init(void);
void EVSYS_init(void);

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
    
    /* Enable interrupts */
    ADC0.INTCTRL |= ADC_RESRDY_bm;
    
    /* Enable event triggered conversion */
    ADC0.EVCTRL |= ADC_STARTEI_bm;
}

void LED0_init(void)
{
    /* Make High (OFF) */
    PORTB.OUT |= PIN5_bm;
    /* Make output */
    PORTB.DIR |= PIN5_bm;
}

void LED0_toggle(void)
{
    PORTB.IN |= PIN5_bm;
}

ISR(ADC0_RESRDY_vect)
{
    /* Clear flag by writing '1': */
    ADC0.INTFLAGS = ADC_RESRDY_bm;
    adcVal = ADC0.RES;    
    LED0_toggle();
}

void RTC_init(void)
{
    uint8_t temp;
    
    /* Initialize 32.768kHz Oscillator: */
    /* Disable oscillator: */
    temp = CLKCTRL.XOSC32KCTRLA;
    temp &= ~CLKCTRL_ENABLE_bm;
    /* Enable writing to protected register */
    CPU_CCP = CCP_IOREG_gc;
    CLKCTRL.XOSC32KCTRLA = temp;
    
    while(CLKCTRL.MCLKSTATUS & CLKCTRL_XOSC32KS_bm)
    {
        ; /* Wait until XOSC32KS becomes 0 */
    }
    
    /* SEL = 0 (Use External Crystal): */
    temp = CLKCTRL.XOSC32KCTRLA;
    temp &= ~CLKCTRL_SEL_bm;
    /* Enable writing to protected register */
    CPU_CCP = CCP_IOREG_gc;
    CLKCTRL.XOSC32KCTRLA = temp;
    
    /* Enable oscillator: */
    temp = CLKCTRL.XOSC32KCTRLA;
    temp |= CLKCTRL_ENABLE_bm;
    /* Enable writing to protected register */
    CPU_CCP = CCP_IOREG_gc;
    CLKCTRL.XOSC32KCTRLA = temp;
    
    /* Initialize RTC: */
    while (RTC.STATUS > 0)
    {
        ; /* Wait for all register to be synchronized */
    }

    RTC.CTRLA = RTC_PRESCALER_DIV32_gc  /* 32 */
              | RTC_RTCEN_bm            /* Enable: enabled */
              | RTC_RUNSTDBY_bm;        /* Run In Standby: enabled */

    /* Set period */
    RTC.PER = RTC_PERIOD;

    /* 32.768kHz External Crystal Oscillator (XOSC32K) */
    RTC.CLKSEL = RTC_CLKSEL_TOSC32K_gc;

    /* Run in debug: enabled */
    RTC.DBGCTRL |= RTC_DBGRUN_bm;
}

void EVSYS_init(void)
{
    /* Real Time Counter overflow */
    EVSYS.CHANNEL0 = EVSYS_GENERATOR_RTC_OVF_gc;
    /* Connect user to event channel 0 */
    EVSYS.USERADC0 = EVSYS_CHANNEL_CHANNEL0_gc;
}

int main(void)
{
    ADC0_init();
    LED0_init();
    RTC_init();
    EVSYS_init();
    
    /* Enable Global Interrupts */
    sei();
    
    while (1) 
    {
        ;
    }
}

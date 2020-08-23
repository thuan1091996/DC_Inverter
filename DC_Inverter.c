/*******************************************************
This program was created by the
CodeWizardAVR V3.12 Advanced
Automatic Program Generator
© Copyright 1998-2014 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : DC Inverter
Version : 0.1 
Date    : 3/15/2020
Author  : Itachi
Suppoted: Nguyen Duc Quyen
Company : Viet Mold Machine
Comments: DC Inverter with variant frequency using SPWM


Chip type               : ATmega8
Program type            : Application
AVR Core Clock frequency: 8.000000 MHz
Memory model            : Small
External RAM size       : 0
Data Stack size         : 256
*******************************************************/

#include <mega8.h>
#include <stdio.h>

#define F2  10000

/* --------------------Global Variables-----------------*/
unsigned int g_F1=70; 	    //F1=50Hz
unsigned int g_freq_load;

/* ------------------Function Declaration---------------*/

/* -----------Timer 1 Initialization---------------
 * Timer 1 is used to create PWM signal with fixed frequency
 * Detail configuration for Timer1:
 *				- ICR1 contains top value (control frequency)
 *				- OCA1 create positive pole
 *				- OCB1 create negative pole
 *				- Mode 14: Fast PWM go from 0 to value in ICR1
 *				- Clock value: 8000.000 kHz, Prescaler 1
 * Input: ui32freq Frequency of small PWM
 * Output: No
 * Affect global data: g_freq_load (value update to timer to create
 *                      expected frequency (F2)
 * Note: 1+Top (ICR1)=F_clk/F_pwm
 */
void timer1Init(unsigned int freq);

/* -----------Timer 1 Interrupt Service Routine---------------
 * Overflow interrupt timer1  handler
 * Upload new duty cycle value to 
 * Enable/Disable PWM signal depend on the current "numpulse"
 * Input: No
 * Output: No
 */
interrupt [TIM1_OVF] void timer1_ovf_isr(void);

/* -----------Disable Compare Output A---------------
 * Disable output on CMP A (OCA1-PB1) and OC1A pin set as an input
 * Input: No
 * Output: No
 */
void disableOutputA(void);

/* -----------Enable Compare Output A---------------
 * Enable output on CMP A (OCA1-PB1) and set OC1A pin as an output
 * Input: No
 * Output: No
 */
void enableOutputA(void);




void main(void)
{
	timer1Init(10000);	
	#asm("sei") 						// Global enable interrupts
while (1)
      {

      }
}

/* ------------------Function definition---------------*/


interrupt [TIM1_OVF] void timer1_ovf_isr(void)
{
    unsigned int num_pulse;
    static unsigned int num_int_called=0;
//    unsigned int duty_load=g_freq_load*;   //duty cycle control
    //Calculate number of pulses
    num_pulse = F2/g_F1;
    num_int_called++;
    OCR1AH=399>> 8;   OCR1AL=399& 0xff;   //Load new compare value (control Duty A)
    if(num_int_called<num_pulse/2)
    {
        enableOutputA();
    }
    else if ( (num_int_called>=num_pulse/2) && (num_int_called<num_pulse) )
    {
        disableOutputA();
    }
    else if (num_int_called>=num_pulse)
    {
        num_int_called=0;
    }
}

void timer1Init(unsigned int freq)
{
	//PB0,PB1 -> output
	DDRB|=0x06;
	//Disable interrupt timer1 before setting it up
    TIMSK|=(0<<TICIE1)| (0<<OCIE1A) | (0<<OCIE1B) | (0<<TOIE1);
    //Ignore compare match on B (half H only)
    TCCR1A =(1<<COM1A1)|(0<<COM1A0)|(0<<COM1B1)|(0<<COM1B0); 
    //Setup timer -> mode 14
    TCCR1A|=(1<<WGM11)|(0<<WGM10); TCCR1B|=(1<<WGM13) | (1<<WGM12);    
    //Prescaler 1 
    TCCR1B|=(0<<CS12) | (0<<CS11) | (1<<CS10);
	//Set up  PWM frequency
	g_freq_load=(8000000/freq)-1;
    ICR1H=(g_freq_load)>> 8;      ICR1L=(g_freq_load)& 0xff;
	//Enable timmer1 interrupt
    TIMSK|=(0<<TICIE1)| (0<<OCIE1A) | (0<<OCIE1B) | (1<<TOIE1); 
}

void disableOutputA(void)
{
    TCCR1A = TCCR1A&( ~0x80);
    DDRB&=~0x02;
}

void enableOutputA(void)
{
    TCCR1A = TCCR1A|0x80;
    DDRB|=0x02;
}

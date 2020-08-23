 /*************** Project information ******************
Project : SPWM with variety frequency (5-120)
Version : V1.0
Date    : 8/27/2018
Author  : Tran Minh Thuan
Suppoted: Nguyen Duc Quyen
Company : Khuon May Viet
*******************************************************/

/****************Requirement documentations************
Create a Sine signal with changable frequency (5-150Hz)
Use: Timer0 to create delay function
     Tier1  generate SPWM signal
Operating:
     Use Timer1 overflow interrupt, each time1 overflow
change the duty cycle in order to create SPWM signal
     Use Timer1 operating mode 14 - count from 0 to value in ICR1
     ICR1 - Control the frequency of each pulse
     OCA1 - Control the duty cycle of positive pole
     OCB1 - Control the duty cycle of negative pole 
*******************************************************/

/******************************************************
Chip type               : ATmega8
Program type            : Application
AVR Core Clock frequency: 8.000000 MHz
Memory model            : Small
External RAM size       : 0
Data Stack size         : 256
*******************************************************/   
#include <mega8.h>
#include <stdio.h>
#include <alcd.h>                   // Alphanumeric LCD functions
#include <delay.h>
/**************************************/
#define Freq_INC PIND.0
#define Freq_DEC PIND.1
#define Duty_INC PIND.2
#define Duty_DEC PIND.3
#define Duty2_INC PIND.4
#define Duty2_DEC PIND.5
#define SW1 PINB.1
#define SW2 PINB.2
#define SW3 PORTB.3
#define SW4 PORTB.0
#define STR PORTC.2
#define CLK PORTB.3
#define SDI PORTC.4
/**************************************/
#define F_CPU 8000000
#define PB0 PORTB.0
#define PB2 PORTB.2
#define F1  20000

unsigned long Freq=70;
unsigned int  LoadA=0, LoadB=0;
unsigned int  Freq_load=0;
unsigned int  Freq_INC_flag=0,Freq_DEC_flag=0;
unsigned int  Duty_INC_flag=0,Duty_DEC_flag=0;
unsigned int  tick_duty_inc=0,tick_duty_dec=0;
unsigned int  tick_freq_inc=0,tick_freq_dec=0;
unsigned int  Duty=10;
unsigned int  Low_line,High_line;
unsigned int  T_delay=0;
unsigned char MA_7SEG[10]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90};
unsigned char Sine1[70]={0, 9, 18, 27, 35, 43, 51, 59, 66, 72, 78, 83, 88, 92, 95, 97, 99, 100, 100, 99, 97, 95, 92, 88, 83, 78, 72, 66, 59, 51, 43, 35, 27, 18, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,};
unsigned char Sine2[70]={0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 9, 18, 27, 35, 43, 51, 59, 66, 72, 78, 83, 88, 92, 95, 97, 99, 100, 100, 99, 97, 95, 92, 88, 83, 78, 72, 66, 59, 51, 43, 35, 27, 18, 9};
unsigned int numb=0;
unsigned char Chuoi[16]={0};

void ADC_Init(void);
unsigned int ADC_Read(unsigned int channel);
void Timer0_Init();
void Timer1_Init();
void GPIO_Init();
void CheckSW(void);                     //Doc gia tri nut nhan
void Update_Timer1(void);
void Update_value(void);
void Delay_100us(unsigned int Time);
void Display(void);

void main(void){
    GPIO_Init();                        // GPIO Initialization 
    ADC_Init();
    lcd_init(16);
    Timer0_Init();                      // Timer0 Initialization
    Timer1_Init();                      // Timer1 Initialization
    #asm("sei")                         // Global enable interrupts
while (1)
      {
            Update_value();
            Delay_100us(1);
            Display();
      }
}

void Display(void){
        sprintf(Chuoi,"Frequency %03d",Freq);
        lcd_gotoxy(0,0);
        lcd_puts(Chuoi); 
        sprintf(Chuoi,"Duty cycle %03d",Duty);
        lcd_gotoxy(0,1);
        lcd_puts(Chuoi);
}
void Timer0_Init(){
    TIMSK&=~0x01;                         //Disable interrupt while initilization
    TCCR0=(0<<CS02)|(1<<CS01)|(0<<CS00);  //Prescaler - 8
    TCNT0=156;                            //255-156+1=100 (100us)
}   
void Timer1_Init(){
/* Timer 1 Initialization
// Timer 1 is used to create sine wave with changeable frequency
// ICR1 contains top value (control frequency)
// OCA1 create positive pole
// OCB1 create negative pole
// Clock source: System Clock
// Clock value: 8000.000 kHz
// Mode 14: Fast PWM go from 0 to value in ICR1
// Prescaler 1 */
    //The Freq_load will control the frequency of sine wave
    //1+Top=Fclk/Fpwm*100 
//         Freq_load=761;
    Freq_load=8000000/(Freq*70)-1;
    TCCR1A =(1<<COM1A1)|(0<<COM1A0)|(0<<COM1B1)|(0<<COM1B0);        
    TCCR1A|=(1<<WGM11)|(0<<WGM10); TCCR1B|=(1<<WGM13) | (1<<WGM12); //Mode 14
    TCCR1B|=(0<<CS12) | (0<<CS11) | (1<<CS10);                      //Prescaler 1                                     
    ICR1H=(Freq_load)>> 8;      ICR1L=(Freq_load)& 0xff;
    TIMSK|=(0<<TICIE1)| (0<<OCIE1A) | (0<<OCIE1B) | (1<<TOIE1);
}
void GPIO_Init(){
/*GPIO Initialization
 *PB7 Increse frequency  
 *PB6 Decrese frequency
 *PB5 Increse Duty cycle
 *PB4 Decrese Duty cycle
*/
    DDRB= (0<<DDB7) |(0<<DDB6) |(0<<DDB5) | (1<<DDB4) | (1<<DDB3) | (1<<DDB2) | (1<<DDB1) | (1<<DDB0);
    PORTB=(0<<PORTB7) | (0<<PORTB6)| (0<<PORTB5) | (0<<PORTB4) | (0<<PORTB3) | (0<<PORTB2) | (0<<PORTB1) | (0<<PORTB0);
    DDRD= 0x00; 
    PORTD=0xFF;
    DDRC=0x00; 
}
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{/*---------- Timer0 Overflow ISR------------------------------  
 // First Reinitialize timer0 value
 // Increase count (T_delay) value every 100us */    
    TCNT0=156;    //Tinh toan = 0x9D(100), nhung bu sai so nen A0   
    if(Freq_INC_flag==1) tick_freq_inc++;   
    if(Freq_DEC_flag==1) tick_freq_dec++;
    if(Duty_INC_flag==1) tick_duty_inc++;
    if(Duty_DEC_flag==1) tick_duty_dec++;
    T_delay++;
//    TIMSK&=~0x01;        need for button
};
interrupt [TIM1_OVF] void timer1_ovf_isr(void){
    /*  Timer1 Overflow ISR
     *  Each time execute, update new compare value to create sine wave
     *  When reach all array values, new cycle of sine frequency start
     *  The arrays contain duty cycles of small signal of big sine wave 
    */  

    Low_line=(35/2)-(Duty/2*35/100);       //1st half wave ->35
    High_line=(35/2)+(Duty/2*35/100);      //1st half wave ->35
    //Else enable output
    if ((numb<Low_line)||(numb>High_line)||(Sine1[numb] == 0)) TCCR1A = TCCR1A&( ~0x80); //If the duty is 0, then disable output 
    else                  TCCR1A = TCCR1A|0x80;     //Else enable output
    if (Sine2[numb] == 0) TCCR1A = TCCR1A&( ~0x20); //If the duty is 0, then disable output
//    if ((numb<Low_line)||(numb>High_line)||(Sine2[numb] == 0)) TCCR1A = TCCR1A&( ~0x20); //If the duty is 0, then disable output
    else                  TCCR1A = TCCR1A|0x20;     //Else enable output
    //-----------------------Load new value of duty cycle-----------------------------// 
    LoadA=Freq_load/100*Sine1[numb];          //Calculate new compare value (Channel A)
    LoadB=Freq_load/100*Sine2[numb];          //Calculate new compare value (Channel B)
    OCR1AH=LoadA>> 8;   OCR1AL=LoadA& 0xff;   //Load new compare value (control Duty A)
    OCR1BH=LoadB>> 8;   OCR1BL=LoadB& 0xff;   //Load new compare value (control Duty B)
    //-----------------------Control H-Bridge-----------------------------------------//
    if  ((numb<35)&& (SW2==0))                             {SW4=1; SW3=0;}
    else SW4=0;
    if ((numb<=70)&&(numb>=35)&&(SW1==0))                  {SW4=0; SW3=1;}
    else SW3=0;
    //-----------------------Change to next array value-------------------------------//
    numb++;
    if(numb>=70)  numb=0;                    //Reload when count all array elements                                            //Start new cycle
                                   
}
void ADC_Init(void){
    ADMUX = (1<<REFS0); //V_ref is VCC, left adjust
    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);    // prescaler = 128
}
/* Read value from ADC channel
 * Input: Input channel (0-7)
 * Output: 0-1024 corresponding to the value
*/
unsigned int ADC_Read(unsigned int channel)
{
  unsigned int ADCval;
  channel &= 0b00000111;  // AND operation with 7
  ADMUX = (ADMUX & 0xF8)|channel; // clears the bottom 3 bits for choosing channel
  // start single convertion by writing ’1' to ADSC
  ADCSRA |= (1<<ADSC);
  // wait for conversion to complete, ADSC becomes ’0' again
  while(ADCSRA & (1<<ADSC));  
  ADCval = ADCL;
  ADCval = (ADCH << 8) + ADCval;    // ADCH is read so ADC can be updated again
  if(ADCval>=1000) ADCval=1000;
  return (ADCval);
}
void Update_value(void){
  unsigned long temp_duty,temp_freq; 
  temp_duty=ADC_Read(1)/10;
  if(temp_duty<5)         temp_duty=5;
  else if (temp_duty>95) temp_duty=95;
  Duty=temp_duty;
  temp_freq=ADC_Read(0)/10;
  if(temp_freq<1)            temp_freq=1;
  else if (temp_freq>75)     temp_freq=75;
  Freq=temp_freq;    
  Freq_load=8000000/(Freq*70)-1;
  ICR1H=(Freq_load-1)>> 8; ICR1L=(Freq_load-1)& 0xff;  

}
void Delay_100us(unsigned int Time)  //TEST DONE
{
/* Create delay function with 100us corresponding to each value */
    TCNT0=0x9D;         //Tinh toan = 0x9D, nhung bu sai so nen A0
    TIMSK|=0x01;        //Cho phep ngat tran timer0
    T_delay=0;          //Reset gia tri dem
    while(T_delay<Time);//Chua du
    TIMSK&=~0x01;       //Du thoi gian, tat ngat tran timer0
};
void Update_Timer1(void){
/*  Update new value "TOP value" when frequency change */
//    Freq_load=761;
    Freq_load=8000000/(Freq*70)-1;
    ICR1H=(Freq_load-1)>> 8; ICR1L=(Freq_load-1)& 0xff;  
}
void CheckSW(void)
{
    if(!Freq_INC){
        TIMSK|=0x01;
        Freq_INC_flag=1;
        if((tick_freq_inc/2000)!=0) //delay 200ms 
        {
            if(Freq<150)    {Freq+=5;  Update_Timer1();}
            else             Freq=150;
            tick_freq_inc=0;
        }
    }
    else
    Freq_INC_flag=0;
    if(!Freq_DEC){
        TIMSK|=0x01;
        Freq_DEC_flag=1;
        if((tick_freq_dec/2000)!=0) //delay 200ms 
        {
            if(Freq>20)      {Freq-=5;  Update_Timer1();}
            else             Freq=20;
            tick_freq_dec=0;
        }
        
    }
    else
    Freq_DEC_flag=0;
    if(!Duty_INC){
        TIMSK|=0x01;
        Duty_INC_flag=1;
        if((tick_duty_inc/2000)!=0) //delay 200ms 
        {
            if(Duty<100)     Duty+=2;  
            else             Duty=95;
            tick_duty_inc=0;
        }
    }
    else
    Duty_INC_flag=0;
    if(!Duty_DEC){
        TIMSK|=0x01;
        Duty_DEC_flag=1;
        if((tick_duty_dec/2000)!=0) //delay 200ms 
        {
            if(Duty>5)       Duty-=2; 
            else             Duty=5;
            tick_duty_dec=0;
        }
    }
    else
    Duty_DEC_flag=0;  

}
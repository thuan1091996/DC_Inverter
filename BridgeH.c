/*
 * Double_H_Bridges.c
 *
 *  Created on: Jul 8, 2019
 *      Author: Itachi
 */

#include "Userlibs.h"
#include "BridgeH.h"



/* -----------BridgeH_GPIO_Init---------------
 * GPIO Initialization for Bridge-H to control motors's direction (IN1 -> IN4 Pins)
 * Input: No
 * Output: No
 * Note: Must configure at least 8 mA for Bridge H input pins
 */
void BridgeH_GPIO_Init(void)
{
    SysCtlPeripheralEnable(BRIDGEH_GPIO_PERIPH);                                                                //Enable Bridge-H GPIO port
    while(!SysCtlPeripheralReady(BRIDGEH_GPIO_PERIPH));
    GPIOPinTypeGPIOOutput(BRIDGEH_GPIO_PORT,IN1_PIN| IN2_PIN| IN3_PIN| IN4_PIN);
    GPIOPadConfigSet(BRIDGEH_GPIO_PORT, IN1_PIN|IN2_PIN|IN3_PIN|IN4_PIN, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD); //Setup pins output 8mA is a MUST
}

/* -----------BridgeH_PWM_Init---------------
 * PWM Initialization for Bridge-H to control motors's speed (ENA, ENB Pins)
 * Input: No
 * Output: No
 * Affect to global variable Load_PWM (Feedback to Update_Speed() )
 */
void BridgeH_PWM_Init(void)
{
    SysCtlPeripheralEnable(BRIDGEH_PWM_GPIO_PERIPH);                                            //Enable Bridge-H PWM Port
    while(!SysCtlPeripheralReady(BRIDGEH_PWM_GPIO_PERIPH));
    SysCtlPeripheralEnable(BRIDGEH_PWM_PERIPH);                                                 //Enable Bridge-H PWM Module
    while(!SysCtlPeripheralReady(BRIDGEH_PWM_PERIPH));

    GPIOPinConfigure(GPIO_ENACONFIG);                                                           //GPIO Configurations for PWM Pins
    GPIOPinConfigure(GPIO_ENBCONFIG);
    GPIOPinTypePWM(BRIDGEH_PWM_GPIO_PORT, (ENA_PIN|ENB_PIN));

    PWMClockSet(BRIDGEH_PWM_PORT,PWM_SYSCLK_DIV_8);                                             //PWM Configurations for PWM Module with prescaler =8
    PWMGenConfigure(BRIDGEH_PWM_PORT, BRIDGEH_PWM_GEN, PWM_GEN_MODE_DOWN|PWM_GEN_MODE_NO_SYNC);
    PWMGenPeriodSet(BRIDGEH_PWM_PORT, BRIDGEH_PWM_GEN, LOAD_PWM);
    PWMGenEnable(BRIDGEH_PWM_PORT, BRIDGEH_PWM_GEN);
    PWMOutputState(BRIDGEH_PWM_PORT, PWM_OUT_4_BIT, true);
    PWMOutputState(BRIDGEH_PWM_PORT, PWM_OUT_5_BIT, true);
}

/* -----------Update_Speed---------------
 * Update speed by loading new duty cycle of 2 PWM pins
 * Input: - int8_t i8_M0Duty - (-100->100) Duty cycle of motor 1
          - int8_t i8_M1Duty - (-100->100) Duty cycle of motor 2
 * Output: No
 */
void Update_PWM0(int8_t i8_M0Duty)
{
    static int32_t temp0_prev=0;
    int32_t temp_load0=LOAD_PWM/100*i8_M0Duty;
    if (temp_load0!=temp0_prev)
    {
        //Motor 1 direction of rotation control
        if(i8_M0Duty> 0)
        {
            Motor0_Forward();
        }
        else if(i8_M0Duty<0)
        {
            Motor0_Backward();
        }
        else if(i8_M0Duty==0)
        {
            Motor0_Stop();
        }
        
        PWMPulseWidthSet(BRIDGEH_PWM_PORT, PWM_OUT_4,abs(temp_load0) );
        temp0_prev=temp_load0;                              //backup previous value
    }
}

void Update_PWM1(int8_t i8_M1Duty)
{
	static int32_t temp1_prev=0;
	int32_t temp_load1=LOAD_PWM/100*i8_M1Duty;
	//Motor 2 direction of rotation control
	if (temp_load1!=temp1_prev)
	{
		if(i8_M1Duty> 0)
		{
				Motor1_Forward();
		}
		else if(i8_M1Duty<0)
		{
				Motor1_Backward();
		}
		else if(i8_M1Duty==0)
		{
				Motor1_Stop();
		}
		PWMPulseWidthSet(BRIDGEH_PWM_PORT, PWM_OUT_5,abs(temp_load1) );
		temp1_prev=temp_load1; 
	}
}


void Motor0_Forward(void)
{
    uint8_t temp=0;
    temp=DATA_PE_R;
    temp&= MOTOR0;                              //Get the current direction of Motor1
    if(temp!=MOTOR0_FORWARD)                    //If it is not running forward then turn off the motor to protect then running forward
    {
        DATA_PE_R&=~MOTOR0;                     //Stop motor
        SysCtlDelay(SysCtlClockGet()/3000);     //Delay 1ms
        DATA_PE_R|=MOTOR0_FORWARD;              //Run forward
    }
}

void Motor0_Backward(void)
{
    uint8_t temp=0;
    temp=DATA_PE_R;
    temp&= MOTOR0;                              //Get the current direction of Motor1
    if(temp!=MOTOR0_BACKWARD)                   //If it is not running backward then turn off the motor to protect then running backward
    {
        DATA_PE_R&=~MOTOR0;                     //Stop the motor
        SysCtlDelay(SysCtlClockGet()/3000);     //Delay 1ms
        DATA_PE_R|=MOTOR0_BACKWARD;             //Run backward
    }
}

void Motor0_Stop(void)
{
    DATA_PE_R&=~MOTOR0;
}

void Motor1_Forward(void)
{
    uint8_t     temp=0;
    temp=DATA_PE_R;
    temp&= MOTOR1;                              //Get the current direction of Motor2
    if(temp!=MOTOR1_FORWARD)                    //If it is not running forward then turn off the motor to protect then running forward
    {
        DATA_PE_R&=~MOTOR1;                     //Stop the motor
        SysCtlDelay(SysCtlClockGet()/3000);     //Delay 1ms
        DATA_PE_R|=MOTOR1_FORWARD;              //Run Forward
    }
}

void Motor1_Backward(void)
{
    uint8_t     temp=0;
    temp=DATA_PE_R;
    temp&= MOTOR1;                              //Get the current direction of Motor2
    if(temp!=MOTOR1_BACKWARD)                   //If it is not running backward then turn off the motor to protect then running backward
    {
        DATA_PE_R&=~MOTOR1;                     //Stop the motor
        SysCtlDelay(SysCtlClockGet()/3000);     //Delay 1ms
        DATA_PE_R|=MOTOR1_BACKWARD;             //Run backward
    }
}

void Motor1_Stop(void)
{
    DATA_PE_R&=~MOTOR1;
}


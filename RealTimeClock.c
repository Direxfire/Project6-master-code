#include<msp430.h>
/*
TODO:
    1. Create a "Set_Time" function. This will allow the current time to be set, this is used to initalize the RTC
    2. Create a "Set_Timer" function. This will allow the RTC to keep track of the 5 minute interval of which the process runs
    3. Createa a "Read_Time" function, this will allow the master to see the timer go off and then take appropriate actions

Setup: (Using the DS3231 RTC from Analog Devices)
    1. RTC is running on I2C bus, with address 0x068 this is hard coded to the RTC
    2. RTC requires setup of the timer which is register 08h
*/


void Set_Time(void);
void Read_Time(void);
extern First_Set_Time;

void Set_Time(void){
    //Send I2C Message to the RTC to set the time
    //

    //Need to put I2C in transmit mode...
    //Need to set buffer length to 7
    //Should be able to use Send_I2C_Message
    Send_I2C_Message(RTC, Set_Time_ptr, 7);
    First_Set_Time = 1;
}

void Read_Time(void){
    //Recieve the current time from the RTC....
    //Put into recieve mode
    UCB1CTLW0 &= ~UCTR;
    UCB1CTLW0 |= UCRXIE0; //Enable recieve interrupt
    UCB1CTLW0 |= UCTXSTT; //Put into recieve mode
    Data_In = 6;          //Reading in 6 bytes of data
    
}


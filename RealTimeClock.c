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
#define RTC 0x068


void Set_Time(char* RTC_Message);
void Read_Time(void);
extern First_Set_Time;
extern Data_In;
extern char Set_Time_ptr;
extern Read_In_Length;
extern *Get_Time_ptr;
extern char I2C_Message_Global[];
extern int I2C_Message_Counter;
extern int I2C_Message_In_Counter;
extern int Read_Time_True;
extern int Read_Slave_Address;

void Set_Time(char* RTC_Message){
    //Send I2C Message to the RTC to set the time
    //

        //Need to put I2C in transmit mode...
        //Need to set buffer length to 7
        //Should be able to use Send_I2C_Message
        int i;
        for(i = 0; i < 8; i++){
            I2C_Message_Global[i] = RTC_Message[i];
        }
        UCB1TBCNT = 8; //Set the buffer size to the size of the input char* I2C_Message ie how many bytes to send via I2C

        UCB1I2CSA = RTC;  //Set the slave address in the module equal to the input slave address
        UCB1CTLW0 |= UCTR;       //Put into transmit mode
        UCB1CTLW0 |= UCTXSTT;   //Generate the start condition
        First_Set_Time = 1;
}


void Read_Time(void){
    //Recieve the current time from the RTC....

    //Get the current time  from the RTC via I2C
    //Send Address and starting register
    Read_Slave_Address = RTC;
    I2C_Message_Counter = 0;
    I2C_Message_Global[0] = 0;
    UCB1TBCNT = 1;                                          //Number of bytes to write out
    UCB1TXBUF = 0;
    UCB1I2CSA = RTC;
    UCB1CTLW0 |= UCTR;                                      // Tx mode
    UCB1CTLW0 |= UCTXSTT;                                   // generate start condition

    while((UCB1IFG & UCSTPIFG) == 0);                       //wait for stop flag
    int i;
    for(i = 0; i < 20000; i++){

    }
    I2C_Message_In_Counter = 0;
    UCB1TBCNT = 7;                                          //Number of bytes to read in
    UCB1IFG &= ~UCSTPIFG;                                   // clear flag
    UCB1CTLW0 &= ~UCTR;                                     // put into RX mode
    UCB1CTLW0 |= UCTXSTT;                                   // generate START condition

    while((UCB1IFG & UCSTPIFG) == 0);                       //wait for stop flag
        UCB1IFG &= ~UCSTPIFG;                               // clear flag
}

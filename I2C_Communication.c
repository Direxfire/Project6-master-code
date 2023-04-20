#include <msp430.h>
#include <string.h>

/*
 * I2C_Communication.c
 *Setup the I2C communication system and create the function to send I2C messages
 *
 *  Created on: Mar 27, 2023
 *      Author: Drew
 */

//Variables from the main.c file
//Must be defined this way because the code is going to replace values in the I2C_Message_Global string to be written out I2C

extern int I2C_Message_Counter;
extern char I2C_Message_Global[32];

void Setup_I2C_Module(void){
    UCB1CTLW0 |= UCSWRST;       //Put UCB1CTLW0 into software reset
    UCB1CTLW0 |= UCSSEL_3;      //Select mode 3
    UCB1BRW = 10;               //Something useful

    UCB1CTLW0 |= UCMODE_3;      //Mode 3
    UCB1CTLW0 |= UCMST;         //Master
    UCB1CTLW0 |= UCTR;          //Transmit mode

    UCB1CTLW1 |= UCASTP_2;      //Autostop enabled

//----- P4.6 and P4.7 for I2C ---
    P4SEL1 &= ~BIT7;
    P4SEL0 |= BIT7;

    P4SEL1 &= ~BIT6;
    P4SEL0 |= BIT6;
//--------------------------
    PM5CTL0 &= ~LOCKLPM5;   //Take out of low power mode

    UCB1CTLW0 &= ~UCSWRST;  //Take out of Software Reset

    UCB1IE |= UCTXIE0;      //Enable TX interrupt
    UCB1IE |= UCRXIE0;      //Enable RX interrupt

}



void Send_I2C_Message(int Slave_Address, char* I2C_Message, int Size_of_Message){
    I2C_Message_Counter = 0;
    UCB1TBCNT = Size_of_Message; //Set the buffer size to the size of the input char* I2C_Message ie how many bytes to send via I2C

    strcpy(I2C_Message_Global, I2C_Message);


    UCB1I2CSA = Slave_Address;  //Set the slave address in the module equal to the input slave address
    UCB1CTLW0 |= UCTR;       //Put into transmit mode
    UCB1CTLW0 |= UCTXSTT;   //Generate the start condition
    int i;
    for(i = 0; i < 1000; i=i+1){} //Short delay loop for some reason

    I2C_Message_Counter = 0;
}

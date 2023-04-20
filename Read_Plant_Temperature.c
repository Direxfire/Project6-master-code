#include<msp430.h>
/*
 *  This script attempts to read 2 bits from the LM92 Temperature sensor
 *  It returns the temperature in C as a float with a resolution of 1/16 of a degree if it gets valid data
 *  Otherwise it returns a 0 and sets Data_Valid to 0
 *
 */


#define LM92 0x048

// Need some global variables
extern int I2C_Message_Counter;

extern int I2C_Message_In_Counter;
extern char I2C_Message_Global_Receive[32];
extern int Data_Valid;
extern int Read_Slave_Address;


float Read_Plant_Temperature(void)
{

    Read_Slave_Address = LM92;
    int c = 0;
    float current_temp = 0.0;

    Data_Valid = 1;
    // Use the I2C interrupt to get the LM92 temp
    I2C_Message_In_Counter = 0;
    UCB1TBCNT = 2;
    UCB1I2CSA = LM92;
    UCB1CTLW0 &= ~UCTR;
    UCB1CTLW0 |= UCTXSTT;

   while((UCB1IFG & UCSTPIFG) == 0){

            c++;
            if(c > 3000){
                // We will ignore temperature readings when Data_Valid is 0
                Data_Valid = 0;
                break;

            }


        }
        UCB1IFG &= ~UCSTPIFG;           //CLEAR STOP FLAG

    // Counter


    // Timeout to make sure we actually got something back from the sensor
   // while((UCB1IFG & UCSTPIFG) == 0){
        /*
        c++;
        if(c > 30000){
            // We will ignore temperature readings when Data_Valid is 0
            Data_Valid = 0;
            break;
        }
        */

    //}
   // UCB1IFG &= ~UCSTPIFG;           //CLEAR STOP FLAG

    // We got data, so we convert it into a float
    if (Data_Valid == 1)
    {
        // Data format is: |sign bit| 12 data bits| 3 bits we don't care about|
        // So we need to do some shifting
        int upperbits = I2C_Message_Global_Receive[0];
        int lowerbits = I2C_Message_Global_Receive[1];
        int cTemp = ((((upperbits) << 5)&~0x1F) + ((lowerbits>>3)& 0x1F) );

        // This means we have a negative
        if( cTemp >= 4096){
            cTemp =( cTemp^0x00001FFF) + 1;
            current_temp = ((0-1)*(float)cTemp);
            current_temp = current_temp/16;

        }
        // Positive number
        else{

            current_temp = (float)cTemp/16;

        }
        return current_temp;

    }
    else
    {
        return 0;

        //return 0;
    }


}

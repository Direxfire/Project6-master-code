#include <msp430.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define third_second_timer_period 333333

#define LCD 0x038
#define LED 0x058
#define RTC 0x068
#define LM92 0x048
#define Temp_Sensor 0x012
#define LM19_PIN BIT0 // Connected on pin 5.0

#define Test_LED BIT6

#define max_I2C_Length 32
#define max_UART_Length 128

void Setup_I2C_Module(void);
void Send_I2C_Message(int, char *, int);
void Setup_Keypad_Ports(void);
char Decode_Input(int);

// Global Variables needed for I2C communication
int I2C_Message_Counter = 0;             // Index value to count the position in the message being written out I2C
char I2C_Message_Global[max_I2C_Length]; // Create an empty "string" to hold the final message as it is being sent out I2C
int I2C_Message_In_Counter = 0;
int Data_In[7] = {0, 0, 0, 0, 0, 0, 0};
int Read_In_Length = 6;

char I2C_Message_Global_Receive[max_I2C_Length];

// int Data_In;                 //Used to count how long the input should be for the RTC or the LM92
int temp_pos; // Counter for the inputs

// Create Read SLave Address:
int Read_Slave_Address;
// UART Function Prototypes
void Setup_UART(void);
void Send_UART_Message(int Size_UART_Message);

char spin[] = {"|/-\\"};

/*
//UART test variables below
char message[] = "Hello World!\r\n";
int position;
//End UART test variables.
*/

// UART Global Variables
char UART_Message_Global[max_UART_Length];
char *UART_Message_ptr = UART_Message_Global;
unsigned int UART_Position_Counter;
int UART_Message_Length;
char Temp_Raw_ASCII[5];
char *Temp_Raw_ASCII_ptr = Temp_Raw_ASCII;
// Analog to digital function prototypes

void Setup_TimerB0_A2D();
void Setup_A2D();

/*This section contains the test variables used to verify the functionality of the system



char Test_Char[] = "A";
char* Test_Char_ptr = Test_Char;
int Test_Slave_Address = 0x048;

*/

/* this section is the configuration of the Real Time Clock
 */

// Functions for the RTC
void Setup_TimerB_RTC(void);
void Set_Time(char *RTC_Message);
void Read_Time(void);

// Constants for the RTC
#define Starting_Seconds 0x00
#define Starting_Minutes 0x30
#define Starting_Hours 0x16
#define Starting_Day 0x01 // This is 1-7 for the days of the week
#define Starting_Date 0x17
#define Starting_Month 0x04
#define Starting_Year 0x23

// Data management for the RTC
char Set_Time_Arr[] = {0x00, Starting_Seconds, Starting_Minutes, Starting_Hours, Starting_Day, Starting_Date, Starting_Month, Starting_Year}; // This will be removed later this is the start time for the RTC
char *Set_Time_ptr = Set_Time_Arr;
char I2C_Message_Global[max_I2C_Length]; // Create an empty "string" to hold the final message as it is being sent out I2C

int Read_Time_True = 0;

int Get_Time = 0;
/*
RTC Struct and variables
*/

/*
Creating a struct to hold the data read in from the RTC
Below is basically the "data base" that holds the current time values.
*/

struct Time
{
    int seconds;
    int minutes;
    int hours;
    int day;
    int date;
    int month;
    int year;
};

struct Time Current_Time;

#define Seconds Current_Time.seconds
#define Minutes Current_Time.minutes
#define Hours Current_Time.hours
#define Day Current_Time.day
#define Date Current_Time.date
#define Month Current_Time.month
#define Year Current_Time.year

int Current_Time_BCD[7]; // Create an array which contains structs
//^This allows for the indexing of the Current_Time struct with an integer

int First_Set_Time = 0;

void bcd_decimal(void);

// End RTC

/*
This section is the struct system for storing all the temperature values.
*/
void Process_Temperature_Data(int);
float Convert_to_Celsius(float);
struct Temperature
{
    float Celsius_Float;
    char Upper_Bit[2];
    char Lower_Bit;
    char Kelvin[4];
    int Raw_Value;
};

struct Temperature Temperature_Array[9];

char Temperature_Write_Out[8];
char *Temperature_Write_Out_ptr = Temperature_Write_Out;
int Samples = 0;
int New_Temp_Value;
int Sample_Number = 0;
float Rolling_Average;
char Rolling_Average_ASCII[4];
char *Rolling_Average_ASCII_ptr = Rolling_Average_ASCII;
float Real_Analog_Value;
int Sample_Size = 0;
int Rolling_Average_Unlocked = 0;
int Raw_Temp = 0;
int Fresh_Data;
// End A2D variables.
// Keypad/Locked code globals
/*
int Passcode_Inputs[5] = {0, 0, 0, 0, 0}; // Three digit passcode, the last value holds the locked state
int Input_Counter = 0;
int Status = 1;
char Input_Arr[3] = {0, 0, 0};
*/
int New_Input = 0;
int Temp_In[2] = {0, 0};

// Keypad functions prototypes

// Program Flow function prototypes

void Time_Out(void);
int Locked_Status(void);
int Unlocked_Status(void);

// Program flow globals
// This is depreciated and needs revised.
/*
char Locked_Code[1];
char *Locked_Code_ptr = Locked_Code;
char Unlocked_ASCII[1];
char *Unlocked_ASCII_ptr = Unlocked_ASCII;
*/
int Unlocked_Input;
char Mode_Select;
int Key_In = 0;
char Sample_Size_ASCII[1];
char *Sample_Size_ASCII_ptr = Sample_Size_ASCII;
/*

Peltier Controls Section
*/

void Setup_PID_Ports(void);
float Read_Plant_Temperature(void);
void Peltier_PID(float, float);

int Peltier_On = 0;

// Here be stuff for the PID
int Data_Valid;
float Derivative;
float Integral;
float Previous_Error;
float Previous_Temperature;
float MIN_OUTPUT = 1.0;
float Current_Temperature;

char Current_Temperature_ASCII[3];
/*

Final revision for the Project 4 code
The goal of this was to create a completely stable version with the I2C bug fixed
Additionally, all functionality will be put into function to allow for a modular design

Group: Drew Currie, Gary Jiang, Johnathan Norell
Written 03/27/23 Last Revision 03/27/23 by Drew.

*/
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer

    // Using some simple IO to debug
    P6DIR |= BIT6;
    P6OUT &= ~BIT6;
    P1IE |= 0xFF;

    // Need to create a simple I2C transmission protocol

    // Call I2C Setup function
    Setup_I2C_Module();
    Setup_Keypad_Ports();

    // Setup A2D and Timers
    Setup_A2D();
    Setup_TimerB0_A2D();
    Setup_TimerB_RTC();
    Setup_PID_Ports();
    // Setup UART
    Setup_UART();
    // After setup don't forget to enable GPIO....
    PM5CTL0 &= ~LOCKLPM5;
    __enable_interrupt();

    /*
    1. Get resolution from user
    2. Begin collecting temperature data from the LN19 analog temp sensor every .5s
    3. Begin collecting temperature data from the LM92 I2C temp sensor          .5s
    4. Get time from the RTC every                                               1s
        -Note the timing on all of these functions is incredibly tight and the system
            must be written very effeciently.
    5. Create the logic for temperature matching when 'C' is selected.
        -This will not be PID.
        -Collecting data from the peltier and analog sensor, the system will check if the value is
            greater than or less than ambient then make the decision to heat or cool.
    6. Add the I2C communication to the slaves to write out the current information.
    */

    /*
    TO-DO:
        1. Impliment Mosfet control circuitirty and code
        2. Reading RTC values and converting to ASCII
        3. Reading LM92 Temperature data
        4. Feedback controls
        5. Add additional I2C Slave communication as needed.
    */

    // Test function to send the initial RTC setup values.......
    void Set_Time(char *RTC_Message); // Yeah this literally just calls the Send_I2C_Message function now...
    unsigned Delay;

    Set_Time(Set_Time_ptr);
    while (1)
    {
        int i;

        snprintf(UART_Message_Global, 100, "System Starting \e[?25l");
        Send_UART_Message(21);
        for (i = 0; i < 20000; i++)
        {
        }

        snprintf(UART_Message_Global, 100, "|");
        Send_UART_Message(1);
        for (i = 0; i < 20000; i++)
        {
        }
        for (i = 0; i < 20000; i++)
        {
        }
        for (i = 0; i < 20000; i++)
        {
        }
        for (i = 0; i < 20000; i++)
        {
        }
        for (i = 0; i < 20000; i++)
        {
        }
        for (i = 0; i < 20000; i++)
        {
        }
        snprintf(UART_Message_Global, 100, "\b/");
        Send_UART_Message(2);
        for (i = 0; i < 20000; i++)
        {
        }
        for (i = 0; i < 20000; i++)
        {
        }
        for (i = 0; i < 20000; i++)
        {
        }
        for (i = 0; i < 20000; i++)
        {
        }
        for (i = 0; i < 20000; i++)
        {
        }
        for (i = 0; i < 20000; i++)
        {
        }
        snprintf(UART_Message_Global, 100, "\b-");
        Send_UART_Message(2);
        for (i = 0; i < 20000; i++)
        {
        }
        for (i = 0; i < 20000; i++)
        {
        }
        for (i = 0; i < 20000; i++)
        {
        }

        for (i = 0; i < 20000; i++)
        {
        }
        for (i = 0; i < 20000; i++)
        {
        }
        for (i = 0; i < 20000; i++)
        {
        }
        snprintf(UART_Message_Global, 100, "\b\\");
        Send_UART_Message(2);
        for (i = 0; i < 20000; i++)
        {
        }
        for (i = 0; i < 20000; i++)
        {
        }
        for (i = 0; i < 20000; i++)
        {
        }

        for (i = 0; i < 20000; i++)
        {
        }
        for (i = 0; i < 20000; i++)
        {
        }
        for (i = 0; i < 20000; i++)
        {
        }

        snprintf(UART_Message_Global, 100, "\b \n\r");
        Send_UART_Message(4);

        // Turn timer on...
        TB0CTL |= MC__UP; // Put timer for A2D into up mode.
        TB1CTL |= MC__UP;

        // Hold the device here indefineatly.
        // Send I2C Message to the RTC to set the time
        // P6OUT ^= BIT6;
        // Read_Time();

        // Now convert to decimal from BCD
        // bcd_decimal();
        // Now convert from decimal to ASCII

        // snprintf(UART_Message_Global, 100, "The Time is now: [20%d-%d-%dT%d:%d:%d]\n\r", Year, Month, Date, Hours, Minutes, Seconds);
        // Send_UART_Message(38);


        //Send UART message for the inputs
        while(Sample_Size == 0){
            //Wait for user sample input
            snprintf(UART_Message_Global, 100, "Please enter the amount of samples to collect: ");
            Send_UART_Message(47);
            for (i = 0; i < 10000; i++);
            if(New_Input == 1){
               *Sample_Size_ASCII_ptr = Decode_Input(Key_In);
                sscanf(Sample_Size_ASCII_ptr, "%d", &Sample_Size);
                snprintf(UART_Message_Global, 100, "%d Samples chosen\r\n", Sample_Size);
                Send_UART_Message(19);
                for (i = 0; i < 1000; i++);
            }
        }

        for(i = 0; i < 1000; i++);

        while(1){
            if (New_Input == 1)
            {
                // Call char decoder here
                Mode_Select = Decode_Input(Key_In);
                New_Input = 0;
            }
        switch(Mode_Select){
            case 'A':
                    // Heat the device
                P5OUT &= ~BIT2; // Set led dir out Cool
                P6OUT &= ~BIT6;
                P5OUT |= BIT3;      // Set led dir out Heat
                //Not sure if this is how this is supposed to be but this resets the 5 min time out:
                //Peltier_On = 0;
                break;
            case 'B':
                    // Cool the device
                P5OUT &= ~BIT3; // Set led dir out Heat
                P6OUT &= ~BIT6;
                P5OUT |= BIT2;      // Set led dir out Cool
                //Not sure if this is how this is supposed to be but this resets the 5 min time out:
                //Peltier_On = 0;
                break;
            case 'C':
                    // Match ambient
                    P5OUT &= ~BIT2;
                    P5OUT &= ~BIT3;
                    P6OUT |= BIT6;
                    //Not sure if this is how this is supposed to be but this resets the 5 min time out:
                    //Peltier_On = 0;
                    //Moving because the PID loop is happening too often here.....
                    //Peltier_PID(Current_Temperature, 100);
                    break;
            case 'D':
                    // Turn off device
                P5OUT &= ~BIT3;
                P5OUT &= ~BIT2;
                P6OUT &= ~BIT6;
                break;
                //Below is for if we're feeling ambitious
            /*case 'E':
                //Not sure if this is how this is supposed to be but this resets the 5 min time out:
                //Peltier_On = 0;
                snprintf(UART_Message_Global, 100, "Please enter override temperature control: ");
                Send_UART_Message(42);
                for (i = 0; i < 10000; i++);
                if(New_Input == 1){
                   *Sample_Size_ASCII_ptr = Decode_Input(Key_In);
                    sscanf(Sample_Size_ASCII_ptr, "%d", &Sample_Size);
                    snprintf(UART_Message_Global, 100, "%d Samples chosen\r\n", Sample_Size);
                    Send_UART_Message(19);
                    for (i = 0; i < 1000; i++);
                }
                break;*/
            default:
                break;
            }

            if (Fresh_Data == 1)
            {
                Fresh_Data = 0;
                Current_Temperature = Read_Plant_Temperature();
                Process_Temperature_Data(Raw_Temp);

            }
            if (Get_Time == 1)
            {
                if(Mode_Select == 'C'){
                    Peltier_PID(Current_Temperature, Rolling_Average);
                }
                Get_Time = 0;
                Read_Time();
                bcd_decimal();
                if (Rolling_Average_Unlocked == 1)
                {
                    for (i = 0; i < 2000; i++)
                        ;

                    // Note these time stamps follow the ISO 8601 standard format of UTC time then + or - the hours to the current time zone. For mountain time this is 6.
                    snprintf(UART_Message_Global, 100, "The current temperature is: %c%c.%c%cC. ", Rolling_Average_ASCII[0], Rolling_Average_ASCII[1], Rolling_Average_ASCII[2], 176);
                    Send_UART_Message(36);
                    for (i = 0; i < 10000; i++)
                        ;
                    snprintf(UART_Message_Global, 100, "The Peltier Temperature is: %c%c.%c%cC.", Current_Temperature_ASCII[0], Current_Temperature_ASCII[1], Current_Temperature_ASCII[2], 176);
                    Send_UART_Message(35);
                    for (i = 0; i < 10000; i++)
                        ;
                    snprintf(UART_Message_Global, 100, "The current time is:[20%d-%d-%dT%d:%d:%d-06:00]\r", Year, Month, Date, Hours, Minutes, Seconds);
                    Send_UART_Message(47);
                    for (i = 0; i < 10000; i++)
                        ;
                }
                else
                {
                    snprintf(UART_Message_Global, 100, "The current time is:[20%d-%d-%dT%d:%d:%d-06:00]\r", Year, Month, Date, Hours, Minutes, Seconds);
                    Send_UART_Message(72);
                }
            }

            if (Peltier_On >= 300)
            {
                Peltier_On = 0;     // Reset Peltier Device counter
                TB0CTL |= MC__STOP; // Disable timers
                TB1CTL |= MC__STOP;
                Mode_Select = 'D';
                P6OUT &= ~BIT6;
                P5OUT &= ~BIT3;
                P5OUT &= ~BIT2;
                snprintf(UART_Message_Global, 100, "\e[?25h");
                Send_UART_Message(5);
                snprintf(UART_Message_Global, 100, "System Turning Off.\n\r");
                Send_UART_Message(21);
                for (i = 0; i < 10000; i++)
                {
                }
                // snprintf(UART_Message_Global, 100, "System Restarting...\n\r");
                // Send_UART_Message(21);
            }
        }
    }
}

void bcd_decimal(void)
{
    Seconds = ((Current_Time_BCD[0] & 0xF0) >> 4) * 10 + (Current_Time_BCD[0] & 0x0F);
    Minutes = ((Current_Time_BCD[1] & 0xF0) >> 4) * 10 + (Current_Time_BCD[1] & 0x0F);
    Hours = ((Current_Time_BCD[2] & 0xF0) >> 4) * 10 + (Current_Time_BCD[2] & 0x0F);
    Day = ((Current_Time_BCD[3] & 0xF0) >> 4) * 10 + (Current_Time_BCD[3] & 0x0F);
    Date = ((Current_Time_BCD[4] & 0xF0) >> 4) * 10 + (Current_Time_BCD[4] & 0x0F);
    Month = ((Current_Time_BCD[5] & 0xF0) >> 4) * 10 + (Current_Time_BCD[5] & 0x0F);
    Year = ((Current_Time_BCD[6] & 0xF0) >> 4) * 10 + (Current_Time_BCD[6] & 0x0F);
}

/* reverse:  reverse string s in place */
void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s) - 1; i < j; i++, j--)
    {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

/* itoa:  convert n to characters in s */
void itoa(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0) /* record sign */
        n = -n;         /* make n positive */
    i = 0;
    do
    {                          /* generate digits in reverse order */
        s[i++] = n % 10 + '0'; /* get next digit */
    } while ((n /= 10) > 0);   /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

void Process_Temperature_Data(int New_Temp_Value)
{
    // Now we have an array of Temperature structs which we can begin to populate??? And then feed these temperature structs into Gary's struct to hold the rolling average
    char buffer[20];
    Temperature_Array[Sample_Number].Raw_Value = New_Temp_Value;
    New_Temp_Value = 0; // Move the new raw value into the struct then clear the new raw value
    // Now to convert from mV to degrees C
    Real_Analog_Value = Temperature_Array[Sample_Number].Raw_Value * 830e-6;
    Temperature_Array[Sample_Number].Celsius_Float = Convert_to_Celsius(Real_Analog_Value);
    int whole_num = (int)Temperature_Array[Sample_Number].Celsius_Float;
    int frac_num = (int)((Temperature_Array[Sample_Number].Celsius_Float - whole_num) * 100); // multiply by 100 to get 2 decimal places

    // Convert int to char then move into struct
    // Using itoa
    itoa(whole_num, buffer);
    Temperature_Array[Sample_Number].Upper_Bit[0] = buffer[0];
    Temperature_Array[Sample_Number].Upper_Bit[1] = buffer[1];

    itoa(frac_num, buffer);
    Temperature_Array[Sample_Number].Lower_Bit = buffer[0];
    // Move everything into the struct

    // Convert to the kelvin equivalent
    float Kelvin_Value = Temperature_Array[Sample_Number].Celsius_Float + 273.15;

    // whole_num = (int) Kelvin_Value;
    // itoa(whole_num, buffer);
    itoa((int)Kelvin_Value, Temperature_Array[Sample_Number].Kelvin);
    // snprintf(Temperature_Array[Sample_Number].Kelvin, 4, "%d", Kelvin_Value);
    // End of the processing of the value.

    if (Sample_Number == Sample_Size - 1)
    {
        Rolling_Average_Unlocked = 1;
    }
    // snprintf(UART_Message_Global, 100, "Current temperature Celsius temperature is: %c%c.%c. \n\r", Temperature_Array[Sample_Number].Upper_Bit[0], Temperature_Array[Sample_Number].Upper_Bit[1], Temperature_Array[Sample_Number].Lower_Bit);
    // Send_UART_Message(40);
    if (Rolling_Average_Unlocked == 1)
    {
        // TODO add average here.... DONE??
        TB0CTL |= MC__STOP; // Put timer into stop mode, change after sample size collected

        int i;
        Rolling_Average = 0;
        for (i = 0; i <= Sample_Size - 1; i++)
        {
            Rolling_Average = Temperature_Array[i].Celsius_Float + Rolling_Average;
        }

        Rolling_Average = Rolling_Average / Sample_Size;
        int whole_num = (int)Rolling_Average;
        int frac_num = (int)((Rolling_Average - whole_num) * 100); // multiply by 100 to get 2 decimal places
        itoa(whole_num, buffer);
        Rolling_Average_ASCII[0] = buffer[0];
        Rolling_Average_ASCII[1] = buffer[1];
        itoa(frac_num, buffer);
        Rolling_Average_ASCII[2] = buffer[0];

        whole_num = (int)Current_Temperature;
        frac_num = (int)((Current_Temperature - whole_num) * 100); // multiply by 100 to get 2 decimal places
        itoa(whole_num, buffer);
        Current_Temperature_ASCII[0] = buffer[0];
        Current_Temperature_ASCII[1] = buffer[1];
        itoa(frac_num, buffer);
        Current_Temperature_ASCII[2] = buffer[0];

        // TODO create snprintf like below with average.
        // snprintf(Temperature_Write_Out, 100, "%c%c.%c\n%c%c%c\n%c%c.%c\n", Temperature_Array[Sample_Number].Upper_Bit[0], Temperature_Array[Sample_Number].Upper_Bit[1], Temperature_Array[Sample_Number].Lower_Bit, Temperature_Array[Sample_Number].Kelvin[0], Temperature_Array[Sample_Number].Kelvin[1], Temperature_Array[Sample_Number].Kelvin[2], Rolling_Average_ASCII[0], Rolling_Average_ASCII[1], Rolling_Average_ASCII[2]);
        snprintf(Temperature_Write_Out, 100, "%c%c.%c\n%c%c%c\n", Rolling_Average_ASCII[0], Rolling_Average_ASCII[1], Rolling_Average_ASCII[2], Temperature_Array[Sample_Number].Kelvin[0], Temperature_Array[Sample_Number].Kelvin[1], Temperature_Array[Sample_Number].Kelvin[2]);

        /* Removing as this is no longer the time to write out the message :)
        Send_I2C_Message(0x048, Temperature_Write_Out_ptr, 8);
        snprintf(UART_Message_Global, 100, "Current Average Temperature %c%c.%cC and %c%c%cK. \n\r", Rolling_Average_ASCII[0], Rolling_Average_ASCII[1], Rolling_Average_ASCII[2], Temperature_Array[Sample_Number].Kelvin[0], Temperature_Array[Sample_Number].Kelvin[1], Temperature_Array[Sample_Number].Kelvin[2]);
        Send_UART_Message(49);
        */
        // Increment sample number

        // Sample_Number = 0; <-- Don't need??
    }
    // Create string of values if samples has not reached the threshold without the average
    // snprintf(Temperature_Write_Out, 100, "%c%c.%c%c%c%c\n", Temperature_Array[Sample_Number].Upper_Bit[0], Temperature_Array[Sample_Number].Upper_Bit[1], Temperature_Array[Sample_Number].Lower_Bit, Temperature_Array[Sample_Number].Kelvin[0], Temperature_Array[Sample_Number].Kelvin[1], Temperature_Array[Sample_Number].Kelvin[2]);
    // Send_I2C_Message(0x048, Temperature_Write_Out_ptr);
    if (Sample_Number == Sample_Size - 1)
    {
        Sample_Number = 0;
    }
    else
    {
        Sample_Number = Sample_Number + 1;
    }
}

float Convert_to_Celsius(float V0)
{
    float Celsius = 0;
    Celsius = 4.5 + (-1481.6 + sqrt(2.1962e6 + ((1.8639 - V0) / (3.88e-6))));
    return (Celsius);
}

#pragma vector = EUSCI_A1_VECTOR
__interrupt void ISR_EUSCI_A1(void)
{
    if (UART_Position_Counter == UART_Message_Length)
    {
        UCA1IE &= ~UCTXCPTIE;
    }
    else
    {
        UART_Position_Counter++;
        UCA1TXBUF = UART_Message_Global[UART_Position_Counter];
    }
    UCA1IFG &= ~UCTXCPTIFG;
}

/*
Interrupt Service Routines

*/

// I2C Transmit intterupt vector
// Used to send data out the master
// TODO UPDATE CODE TO RECIEVE DATA FROM RTC AND TEMP_SENSOR

#pragma vector = EUSCI_B1_VECTOR
__interrupt void EUSCI_B1_I2C_ISR(void)
{
    switch (UCB1IV)
    {
    case 0x16:
        if (Read_Slave_Address == LM92)
        {
            I2C_Message_Global_Receive[I2C_Message_In_Counter] = UCB1RXBUF; // IT PUTS THE DATA IN THE BASKET
            I2C_Message_In_Counter++;
        }
        if (Read_Slave_Address == RTC)
        {
            Current_Time_BCD[I2C_Message_In_Counter] = UCB1RXBUF; // receive data
            I2C_Message_In_Counter++;
        }
        break;

    case 0x18:
        UCB1TXBUF = I2C_Message_Global[I2C_Message_Counter]; // Send the next byte in the I2C_Message_Global string
        I2C_Message_Counter++;                               // Increase the message position counter
        break;
    default:
        break;
    }
}

// Keypad interrupt vector
#pragma vector = PORT1_VECTOR
__interrupt void ISR_Keypad_Pressed(void)
{
    Temp_In[0] = P1IN;
    // Switch to P1 as output, and P2 as input

    // Set P1.0->P1.3 as Outputs
    P1DIR |= 0xFF;
    P1REN |= 0xFF;
    P1OUT |= 0xFF;
    // Set P2.0->P2.3 as Outputs
    P2DIR &= ~0xFF;
    P2REN |= 0xFF;
    P2OUT &= ~0xFF;

    P2IN &= ~0xFF;
    P1IFG &= ~0xFF; // Clear P1 IFG

    Temp_In[1] = P2IN;
    // Rotate the upper nibble
    Temp_In[0] = Temp_In[0] << 4;

    Key_In = Temp_In[0] | Temp_In[1];

    // Switch to P1 as output, and P2 as input

    // Set P1.0->P1.3 as inputs
    P1DIR &= ~0xFF;
    P1REN |= 0xFF;
    P1OUT &= ~0xFF;
    // Set P2.0->P2.3 as Outputs
    P2DIR |= 0xFF;
    P2REN &= 0xFF;
    P2OUT |= 0xFF;

    // Enable P1.0->P1.3 Interrupts
    P1IN &= ~0xFF;
    P1IFG &= ~0xFF;
    P2IFG &= ~0xFF; // Clear P2 IFG

    P1IE |= 0xFF;
    New_Input = 1;
}

// Analog to Digital interrupt triggered by Timer A1
#pragma vector = TIMER0_B0_VECTOR
__interrupt void Sample_Timer(void)
{

    // TB0CTL |= MC__STOP;

    // Clear timer interrupt flag
    TB0CCTL0 &= ~CCIFG;
    // read LM19 sensor and convert to temperature
    ADCCTL0 |= ADCENC | ADCSC;
    while (ADCCTL1 & ADCBUSY)
        ;
    Raw_Temp = ADCMEM0;
    Fresh_Data = 1;
}

#pragma vector = TIMER1_B0_VECTOR
__interrupt void RTC_Timer(void)
{
    TB0CCTL0 &= ~CCIFG;
    Get_Time = 1;
    Peltier_On++;
}

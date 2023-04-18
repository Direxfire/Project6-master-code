#include <msp430.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>


#define third_second_timer_period 333333

#define LCD 0x048
#define LED 0x058
#define RTC 0x068
#define Temp_Sensor 0x012
#define LM19_PIN BIT0	//Connected on pin 5.0

#define Test_LED BIT6

#define max_I2C_Length 32
#define max_UART_Length 64

void Setup_I2C_Module(void);
void Send_I2C_Message(int, char *, int);
void Setup_Keypad_Ports(void);
char Decode_Input(int);

// Global Variables needed for I2C communication
int I2C_Message_Counter = 0; // Index value to count the position in the message being written out I2C
char I2C_Message_Global[max_I2C_Length]; // Create an empty "string" to hold the final message as it is being sent out I2C
int Data_In; 				//Used to count how long the input should be for the RTC or the LM92
int temp_pos;				//Counter for the inputs

// UART Function Prototypes
void Setup_UART(void);
void Send_UART_Message(int Size_UART_Message);

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

//Functions for the RTC
void Set_Time(void);
void Read_Time(void);


//Constants for the RTC
#define Starting_Seconds 0x00
#define Starting_Minutes 0x00
#define Starting_Hours   0x00
#define Starting_Day     0x01 //This is 1-7 for the days of the week
#define Starting_Data    0x01 
#define Starting_Year    0x17 //This is 23 in hex

//Data management for the RTC
char Set_Time[] = {0x00, Starting_Seconds, Starting_Minutes, Starting_Hours, Starting_Day, Starting_Data, Starting_Year}; //This will be removed later this is the start time for the RTC
char *Set_Time_ptr = Set_Time;

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

struct Current_Time time;

#define Seconds Current_Time.seconds
#define Minutes Current_Time.minutes
#define Hours Current_Time.hours
#define Day Current_Time.day
#define Date Current_Time.date
#define Month Current_Time.month
#define Year Current_Time.year

int Current_Time_BCD[6] ={Seconds, Minutes, Hours, Day, Date, Month, Year};	//Create an array which contains structs
//^This allows for the indexing of the Current_Time struct with an integer

int First_Set_Time = 0;

//End RTC

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

// End A2D variables.
// Keypad/Locked code globals
/*
int Passcode_Inputs[5] = {0, 0, 0, 0, 0}; // Three digit passcode, the last value holds the locked state
int Input_Counter = 0;
int Temp_In[2] = {0, 0};
int Status = 1;
char Input_Arr[3] = {0, 0, 0};
int New_Input = 0;
*/

// Keypad functions prototypes

// Program Flow function prototypes

void Time_Out(void);
int Locked_Status(void);
int Unlocked_Status(void);

// Program flow globals
//This is depreciated and needs revised.
char Locked_Code[1];
char *Locked_Code_ptr = Locked_Code;
char Unlocked_ASCII[1];
char *Unlocked_ASCII_ptr = Unlocked_ASCII;

int Unlocked_Input;



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
	/*
	// Using some simple IO to debug
	P6DIR |= BIT6;
	P6OUT &= ~BIT6;
	*/
	Set_RTC();

	// Need to create a simple I2C transmission protocol

	// Call I2C Setup function
	Setup_I2C_Module();
	Setup_Keypad_Ports();

	// Setup A2D and Timers
	Setup_A2D();
	Setup_TimerB0_A2D();

	// Setup UART
	Setup_UART();
	// After setup don't forget to enable GPIO....
	PM5CTL0 &= ~LOCKLPM5;
	__enable_interrupt();

	/*
	1. Get resolution from user
	2. Begin collecting temperature data from the LN19 analog temp sensor every .5s
	3. Begin collecting temperature data from the LM92 I2C temp sensor          .5s
	4. Get time from the RTC every 												 1s
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

	//Test function to send the intitial RTC setup values.......
	void Set_Time(void); //Yeah this litterally just calls the Send_I2C_Message function now...
	unsigned Delay;
	while(1){
		//Hold the device here indefinately. 
		void Set_Time(void);
		for(Delay = 20000; Delay >= 0; Delay--){
			//Delay the system before resending the Set_Time message
		}
	}


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
		n = -n;			/* make n positive */
	i = 0;
	do
	{						   /* generate digits in reverse order */
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
    //snprintf(UART_Message_Global, 100, "Current temperature Celsius temperature is: %c%c.%c. \n\r", Temperature_Array[Sample_Number].Upper_Bit[0], Temperature_Array[Sample_Number].Upper_Bit[1], Temperature_Array[Sample_Number].Lower_Bit);
    //Send_UART_Message(40);
	if (Rolling_Average_Unlocked == 1)
	{
		// TODO add average here.... DONE??
	    TB0CTL |= MC__STOP;                      //Put timer into stop mode, change after sample size collected

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

		// TODO create snprintf like below with average.
		// snprintf(Temperature_Write_Out, 100, "%c%c.%c\n%c%c%c\n%c%c.%c\n", Temperature_Array[Sample_Number].Upper_Bit[0], Temperature_Array[Sample_Number].Upper_Bit[1], Temperature_Array[Sample_Number].Lower_Bit, Temperature_Array[Sample_Number].Kelvin[0], Temperature_Array[Sample_Number].Kelvin[1], Temperature_Array[Sample_Number].Kelvin[2], Rolling_Average_ASCII[0], Rolling_Average_ASCII[1], Rolling_Average_ASCII[2]);
		snprintf(Temperature_Write_Out, 100, "%c%c.%c\n%c%c%c\n", Rolling_Average_ASCII[0], Rolling_Average_ASCII[1], Rolling_Average_ASCII[2], Temperature_Array[Sample_Number].Kelvin[0], Temperature_Array[Sample_Number].Kelvin[1], Temperature_Array[Sample_Number].Kelvin[2]);
		Send_I2C_Message(0x048, Temperature_Write_Out_ptr, 8);
		snprintf(UART_Message_Global, 100, "Current Average Temperature %c%c.%c %cC and %c%c%c %cK. \n\r", Rolling_Average_ASCII[0], Rolling_Average_ASCII[1], Rolling_Average_ASCII[2], 248, Temperature_Array[Sample_Number].Kelvin[0], Temperature_Array[Sample_Number].Kelvin[1], Temperature_Array[Sample_Number].Kelvin[2], 248);
		Send_UART_Message(49);
		// Increment sample number

		Sample_Number = 0;
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
	Celsius = -1481.6 + sqrt(2.1962e6 + ((1.8639 - V0) / (3.88e-6)));
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
//TODO UPDATE CODE TO RECIEVE DATA FROM RTC AND TEMP_SENSOR

/* OLD I2C Interrupt Service Routine
#pragma vector = EUSCI_B1_VECTOR
__interrupt void EUSCI_B1_I2C_ISR(void)
{
	// Send I2C Message
	UCB1TXBUF = I2C_Message_Global[I2C_Message_Counter]; // Send the next byte in the I2C_Message_Global string
	I2C_Message_Counter++;								 // Increase the message position counter
}

*/

//New Interrupt Service Routine
//Updated to allow for sending the RTC setup function although that could be changed to just use the Send_I2C_Message funciton ¯\_(ツ)_/¯ 
#pragma vector = EUSCI_B1_VECTOR
__interrupt void EUSCI_B1_I2C_ISR(void){

    switch(UCB1IV){				//Read in data
     case 0x16:                 //ID 16: RXIFG0 -->Recieve, used for both temperature and RTC. 
                                //Expecting 7 bytes of data from the RTC
                                //Expecting 1 byte from the LM92 
         for(temp_pos = 0; temp_pos < Data_In; temp_pos++){
        // Data_In[temp_pos] = UCB1RXBUF;   // receive data
        //Need to do some conversions here to move the imported data into the struct. Using an array I think is the best option
            Current_Time_BCD[j] = UCB1RXBUF;    //Read into the BCD array for temporary storage
         int j;
         for (j = 0; j < 500; ++j) {
            //delay
        }
         }
         break;
    //This whole case needs to be updated for the transmission with variable buffer size and etc.
     case 0x18:                 // ID 18: TXIFG0 --> Transmit
	 //This if statement is probably redundant and can be removed and optomized but ¯\_(ツ)_/¯ 
         if(first_Set_Time == 1 && I2C_Message_Counter <= 6){		//Send the setup time to the RTC
                 UCB1TBCNT = 6;  //# of bytes in Set_Time

                 UCB1TXBUF = Set_Time[I2C_Message_Counter];

                 if(I2C_Message_Counter == 5){
                     First_Set_Time = 0;
                 }
                 else{
                     I2C_Message_Counter++;
                 }
		else if (first_Set_Time != 1)				//Write out to the slave from Send_I2C_Message 
		{
				//Count set in Send_I2C_Message function
				//Now just increase I2C_Message_Global
				UCB1TXBUF = I2C_Message_Global[I2C_Message_Counter]; // Send the next byte in the I2C_Message_Global string
				I2C_Message_Counter++;								 // Increase the message position counter

				 
		}
		
         }
         else{
                UCB1TXBUF = 0x03;      //send register address to start
         }
         break;
     default:
         break;
    }
}





// Keypad interrupt vector
#pragma vector = PORT1_VECTOR
__interrupt void ISR_Keypad_Pressed(void)
{
	
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
	while (ADCCTL1 & ADCBUSY);
	Raw_Temp = ADCMEM0;
	Fresh_Data = 1;

}

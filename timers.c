#include<msp430.h>

//Setting up timers to trigger the .333 second analog read in 

//NEEDS UPDATED TO TRIGGER TIMER EVERY .5 SECONDS  

void Setup_TimerB0_A2D(void){
    /*
    // configure TimerA for interrupts
    TA0CTL |= TASSEL_2 | ID_3 | MC_1 | TACLR;
    TA0CCR0 = TIMER_PERIOD - 1;	// .333 seconds
    TA0CCTL0 |= CCIE;	// Enables Timer Interupt
    */

   //Using timer B0 to trigger a .333ms interrupt to poll for the current temp.

   TB0CTL |= TBCLR;                         //Clear timers and dividers
   TB0CTL |= TBSSEL__SMCLK;                 //Source is SMCLOCK (1Mhz)
   TB0CTL |= MC__STOP;                      //Put timer into stop mode, change after sample size collected
   TB0CTL |= ID__8;
   TB0EX0 |= TBIDEX__4;
   TB0CCR0 = 10944;                         //Set CCR0 to the value put into this function

   TB0CCTL0 |= CCIE;                        //Enable CCR0 interrupt
   TB0CCTL0 &= ~CCIFG;                      //Clear interrupt flag

}

void Setup_TimerB_RTC(void){
    /*
    Configure the timer to trigger every one second to get the time from the RTC
    */
   TB1CTL |= TBCLR;
   TB1CTL |= TBSSEL__ACLK;
   TB1CTL |= MC__STOP;
   TB1CCR0 |= 32768;

   TB1CCTL0 |= CCIE;
   TB1CCTL0 &= ~CCIFG;

}




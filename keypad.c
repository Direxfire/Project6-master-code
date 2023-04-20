#include <msp430.h>
/*
This file contains all the functions related to the keypad


Functions:
    Setup_Keypad_Ports
        -> This function configures P1.0->P1.3 and P2.0->P2.3 as inputs for the keypad
        -> This starts with P1 as an input with P2 as an ouput, wating for P1 Interrupt to trigger 
        -> After this happens, it logs the input, swaps P1 and P2 as input/ouput
        -> Then read in again
        -> Then rotate the first value x4 then OR the values into one combined value.
    Decode_Input
        -> This returns an ASCII character based on the input
        -> IE if 1 is pressed this function will return '1' 
    
*/


void Setup_Keypad_Ports(void){
    //Setup P1.0->P1.3 as inputs

    P1DIR &= ~0xFF;      //Clear P1 direction = in
    P1REN |= 0xFF;       //Enable Pull up/down resistor
    P1OUT &= ~0xFF;      //Set Pull Down resistor
    P1IES &= ~0xFF;      //Set interrupt sensativity to L-to-H
    P1IFG &= ~0xFF;      //Clear any P1IFG on P1.0->P1.3
    P1IE |= ~0xFF;
    //Setup P2.0->P2.3 as inputs

    P2DIR |= 0xFF;       //Set P2 direction =  out
    P2OUT |= 0xFF;       //Set P2 out on
    P2REN &= ~0xFF;      //Disable Resistor
    //No interrupts needed.

}

char Decode_Input(int Input) {

    switch (Input) {
        case 0x84:
            return ('0');
            break;

        case 0x18:
            return ('1');
            break;

        case 0x14:
            return ('2');
            break;

        case 0x12:
            return ('3');
            break;

        case 0x28:
            return ('4');
            break;

        case 0x24:
            return ('5');
            break;

        case 0x22:
            return ('6');
            break;

        case 0x48:
            return ('7');
            break;

        case 0x44:
            return ('8');
            break;

        case 0x42:
            return ('9');
            break;

        case 0x11:
            return ('A');
            break;

        case 0x21:
            return ('B');
            break;

        case 0x41:
            return ('C');
            break;

        case 0x81:
            return ('D');
            break;

        case 0x88:
            return ('*');
            break;

        case 0x82:
            return ('#');
            break;
        default:
            return ('\0');
            break;
    }
}

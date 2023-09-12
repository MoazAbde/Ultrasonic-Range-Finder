/* 
 * File:   main.c
 * Author: Moaz Abdelmonem
 * ID: 1660622
 * Created on October 16, 2022, 10:16 AM
 */

#include "defines.h"


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#include <avr/interrupt.h>

#include "hd44780.h"
#include "lcd.h"
#define trigger_pin PB1
#define switch_pin PB2


// initialization of LCD 
static void
ioinit(void)
{
 
  lcd_init();
}

FILE lcd_str = FDEV_SETUP_STREAM(lcd_putchar, NULL, _FDEV_SETUP_WRITE);

int overflow = 0;

ISR(TIMER1_OVF_vect)
{
	overflow++;	/* Increment Timer Overflow count */
}


int main() {
  long count;
  double distance;
  double minDistance = 30.00;
  double maxDistance = 0;
  double sensorMaxRange = 70.00; //threshold max senor distance
  double sensorMinRange = 2.00; //threshold min sensor distance
  int outOfRange = 0;
  
    
  DDRB &= ~(1<<switch_pin); // Switch. PB2 as I/P
  PORTB |= (1<<switch_pin); // Enable pull up
  DDRB |= (1<<trigger_pin); // make the trigger pin an output
  
  ioinit();
  stderr = &lcd_str; //to print o/p on LCD
  
  sei(); //enable global interrupts 
  
  TIMSK1 = (1<<TOIE1); //enabling timer overflow interrupt
  TCCR1B |= (1<<CS10); // no pre-scaler
  TCCR1A = 0; //setting timer to normal mode operations 
  
  while(1){
      
      PORTB |= (1 << trigger_pin); //send the ultra sound signal
      _delay_us(10);
      PORTB &= ~(1 << trigger_pin); //stop sending signal
     
      TCNT1 = 0; //RESET TIMER
      
      TCCR1B |= (1 << ICES1); // CAPTURE TIMER VALUE ON RISING EDGE
      TIFR1 |= (1 << ICF1); // CLEAR INPUT CAPTURE FLAG IN CASE IT WAS SET
      TIFR1 |= (1<<TOV1); //clear timer over flow flag
      
      //spin wait, for the rising edge of the ICP
      while(!(TIFR1 & (1 << ICF1)));
      TCNT1 = 0; //RESET TIMER
      TCCR1B &= ~(1 << ICES1); //CAPTURE ON FALLING EDGE NOW
      TIFR1 |= (1 << ICF1); // CLEAR INPUT CAPTURE FLAG IN CASE IT WAS SET
      TIFR1 |= (1<<TOV1); 
      
      overflow = 0; //clear over flow count
      
      //spin wait for the falling edge of the ICP (ECHO pin)
      while(!(TIFR1 & (1 << ICF1)));
      count = ICR1 + (65535 * overflow);
      
      
      
//      distance = (double)count / 859.8017491;
       distance = (double)count / 890.1624122;
       if ((distance < sensorMinRange) || (distance > sensorMaxRange))
        {
            fprintf(stderr, " Out of Range \x1b\xc0");
        }
        else
        {
            fprintf(stderr, "     %.1f cm \x1b\xc0", distance); // Display distance
        }

        // when the button is not clicked, find the min and max distances if they are within the sensors range
        if (PINB & (1 << switch_pin))
        {

            if ((distance > sensorMinRange) && (distance < sensorMaxRange) && (distance > maxDistance))
            {
                maxDistance = distance;
                outOfRange =0;
            }
            if ((distance > sensorMinRange) && (distance < sensorMaxRange) && (distance < minDistance))
            {
                minDistance = distance;
                outOfRange =0;
            }
        }
        else 
        {
            //When the button is clicked check if the sensors range is satisfied
            // if it's not set out of range to 1
            outOfRange = 0;
            if ((distance > sensorMinRange) && (distance < sensorMaxRange))
            {
                maxDistance = distance;
                minDistance = distance;
            }
            else
            {
                outOfRange = 1;
            }
        }
       // if the button is pressed out of range, set the min and max to NaN
       //if it's not pressed out of range, then detect the current min and max distances measured
        if (outOfRange == 1)
        {
            fprintf(stderr, "NaN     NaN", distance);
        }
        else
        {
            fprintf(stderr, "%.1f cm  %.1f cm ", minDistance, maxDistance);
        }
   
        _delay_ms(100);
        fprintf(stderr, "\x1b\x01"); // clear LCD

    
  }  
  
  
  
  

  
   return 0;
  
 }
  
 

  
  


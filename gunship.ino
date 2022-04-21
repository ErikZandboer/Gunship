/*--------------------------------------------------------------------------------------------*/
/* Gunship led & servos for front gun.                                                        */
/*                                                                                            */
/* Controls various leds and servos on a timed basis                                          */
/*                                                                                            */
/* Versioning:                                                                                */
/* 1.00            Creation.                                                                  */
/*--------------------------------------------------------------------------------------------*/

// Includes
#include <Arduino.h>
#include <SoftwareSerial.h>
#include "RedMP3.h"
#include <Servo.h>

// Define the physical pinout
#define SERVOX        5
#define SERVOY        6
#define MP3_TX        7   // connect to RX of the MP3 player module
#define MP3_RX        8
#define GREENLIGHTS   9 
#define PULSELEDS     10
#define MAINROTOR     2
#define TAILROTOR     3

// Variables for the motors
byte    MainMotorSpeed = 0;  // Speed at which the motor turns. Used in the Motor state machine.
byte    TailMotorSpeed = 0;  // Speed at which the motor turns. Used in the Motor state machine.

// Define the servos
Servo servoX;
Servo servoY;

// Setup serial comms to the MP3 hardware
MP3 mp3(MP3_RX, MP3_TX);

// Get some base counters in for the runtime in seconds.
unsigned int    TickCounter=0;

#define servoXmin 45
#define servoXmax 135
#define servoYmin 45
#define servoYmax 135

#define RUN_FREQ 100

// Timings and things to change
#define TIM_SAMPLESTART       20*RUN_FREQ       // Seconds after which the sample starts playing 
#define TIM_SAMPLELENGTH      40*RUN_FREQ       // Length of the sample 
#define TIM_GREEN_ON          20*RUN_FREQ
#define TIM_GREEN_OFF         40*RUN_FREQ
#define TIM_SERVOS_ON         26*RUN_FREQ
#define TIM_SERVOS_OFF        40*RUN_FREQ
#define TIM_PULSE_ON          20*RUN_FREQ
#define TIM_PULSE_OFF         50*RUN_FREQ
#define TIM_TAILROTOR_ON      22*RUN_FREQ
#define TIM_TAILROTOR_OFF     65*RUN_FREQ
#define TIM_TAILROTOR_KICK     1*RUN_FREQ        // Number of seconds for the kickstart of the main rotor
#define TIM_MAINROTOR_ON      22*RUN_FREQ
#define TIM_MAINROTOR_OFF     65*RUN_FREQ
#define TIM_MAINROTOR_KICK     2*RUN_FREQ        // Number of seconds for the kickstart of the main rotor
#define TIM_REPEAT            90*RUN_FREQ       // Rewind at 90 seconds (max. value is 655535 = 327 seconds = a little over 5 minutes)

// This runs only once when powering on
void setup()
{
   digitalWrite(GREENLIGHTS, LOW);       // GREEN lights start OFF
   digitalWrite(PULSELEDS,   LOW);      // Pulse leds start OFF
   digitalWrite(MAINROTOR,   LOW);     // Main Rotor starts OFF   
   digitalWrite(TAILROTOR,   LOW);    // Tail Rotor starts OFF   
   
   // All led and motor groups are defined as pin OUTPUT
   pinMode (GREENLIGHTS, OUTPUT);
   pinMode (PULSELEDS, OUTPUT);
   pinMode (MAINROTOR, OUTPUT);
   pinMode (TAILROTOR, OUTPUT);
        
   // Get a random seeding by reading analog pin 0 (leave disconnected!)
   randomSeed(analogRead(0));
   
   TickCounter=0;
}

// This loops forever.
void loop()
{
   while ( (millis() % 10) != 0L ) //Just do nothing until millis()/10 has no remainder --> Run at 100Hz
   {
      asm("nop \n"); // Just do NOP to make sure the compiler doesn't optimize the while() away
   }

   // This code executes 100 times a second
   TickCounter++;
   if (TickCounter > TIM_REPEAT)       
   {
       TickCounter = 0;   // Reset the tick counter when we need to restart the animatronics
   }
  
   // ------------------------------ HANDLE THE MP3 AUDIO --------------------------------------
   if (TickCounter == TIM_SAMPLESTART)
   {
      mp3.playWithVolume(1,26);  // Play the first mp3 on the card at volume 26 (max is 30)
   }        
   if (TickCounter == TIM_SAMPLESTART+TIM_SAMPLELENGTH)
   {
       mp3.stopPlay(); // Stop playing after the show is over
   }

   // ------------------------------ HANDLE THE PULSING LEDS --------------------------------------
   if ((TickCounter >= TIM_PULSE_ON) && (TickCounter <= TIM_PULSE_OFF))
   {
      if ((TickCounter % 100) == 0)
      {
        digitalWrite(PULSELEDS, HIGH); // Pulse leds switch ON
      }
      if ((TickCounter % 100) == 1)
      {
        digitalWrite(PULSELEDS, LOW); // Pulse leds switch OFF just 1/100th of a second later
      }
   }

   // ------------------------------ HANDLE THE MAIN ROTOR --------------------------------------
   if (TickCounter == TIM_MAINROTOR_ON)
   {
      analogWrite(MAINROTOR,255); // Kick the motor at full speed to make sure it starts turning
      MainMotorSpeed = 10; // Next iterations of motorspeed will tune down the speed again (end the kickstart)      
   }
   if ((TickCounter > TIM_MAINROTOR_ON + TIM_MAINROTOR_KICK) && (TickCounter < TIM_MAINROTOR_OFF)) // Revving or running
   {
    if (TickCounter % 10 == 0) // ten times per second
    {
      if (MainMotorSpeed < 255)
      {
        MainMotorSpeed++; // Starts at 10, so revvs up 245 --> revving should take 24.5 seconds 
        analogWrite(MAINROTOR,MainMotorSpeed);
      }
    }
   }
   if (TickCounter == TIM_MAINROTOR_OFF)
   {
    MainMotorSpeed = 0;
    analogWrite(MAINROTOR,0);
   }

   // ------------------------------ HANDLE THE TAIL ROTOR --------------------------------------
   if (TickCounter == TIM_TAILROTOR_ON)
   {
      analogWrite(TAILROTOR,255); // Kick the motor at full speed to make sure it starts turning
      TailMotorSpeed = 10; // Next iterations of motorspeed will tune down the speed again (end the kickstart)      
   }
   if ((TickCounter > TIM_TAILROTOR_ON + TIM_TAILROTOR_KICK) && (TickCounter < TIM_TAILROTOR_OFF)) // Revving or running
   {
    if (TickCounter % 10 == 0) // ten times per second
    {
      if (TailMotorSpeed < 255)
      {
        TailMotorSpeed++; // Starts at 10, so revvs up 245 --> revving should take 24.5 seconds 
        analogWrite(MAINROTOR,TailMotorSpeed);
      }
    }
   }
   if (TickCounter == TIM_TAILROTOR_OFF)
   {
    TailMotorSpeed = 0;
    analogWrite(TAILROTOR,0);
   }

   // ------------------------------ HANDLE THE GREEN BURNING LEDS --------------------------------------
   if (TickCounter == TIM_GREEN_ON)
   {
      digitalWrite(GREENLIGHTS, HIGH); // Green leds switch ON
   } 
   if (TickCounter == TIM_GREEN_OFF)
   {
      digitalWrite(GREENLIGHTS, LOW); // Green leds switch OFF
   } 

   // ------------------------------ HANDLE THE SERVOS --------------------------------------
   if (TickCounter == TIM_SERVOS_ON)
   {
      servoX.attach(SERVOX);
      servoY.attach(SERVOY);
   }

   // Within this timeframe the servo's are allowed to move each second. X moves on the second, Y moves on every half-a-second
   if ( (TickCounter >= TIM_SERVOS_ON) && (TickCounter < TIM_SERVOS_OFF) && ((TickCounter %100) ==0) )
   {
      servoX.write((unsigned char)random(servoXmin,servoXmax));
   }
   if ( (TickCounter >= TIM_SERVOS_ON) && (TickCounter < TIM_SERVOS_OFF) && ((TickCounter %100) ==50) )
   {
      servoY.write((unsigned char)random(servoYmin,servoYmax));
   }

   if (TickCounter == TIM_SERVOS_OFF)
   {
      servoX.detach();
      servoY.detach();
   }

   delay(1); // Added this dummy delay() to make sure the code takes more than 1 ms to execute.
}

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
#define GUNSERVO      5
#define MP3_TX        7   // connect to RX of the MP3 player module
#define MP3_RX        8
#define CONSTANTLEDS  9 
#define PULSELEDS     10
#define REDPULSE      11
#define MAINROTOR     2
#define TAILROTOR     3

// Variables for the motors
byte    MainMotorSpeed = 0;  // Speed at which the motor turns. Used in the Motor state machine.
byte    TailMotorSpeed = 0;  // Speed at which the motor turns. Used in the Motor state machine.

// Define the servos
Servo gunServo;

// Setup serial comms to the MP3 hardware
MP3 mp3(MP3_RX, MP3_TX);

// Get some base counters in for the runtime in seconds.
unsigned int    TickCounter=0;

// in/max degrees for gunservo
#define GUNSERVO_MIN 70
#define GUNSERVO_MAX 110

#define RUN_FREQ 100

// Timings and things to change
#define TIM_SAMPLESTART       20*RUN_FREQ       // Seconds after which the sample starts playing 
#define TIM_SAMPLELENGTH      40*RUN_FREQ       // Length of the sample 
#define TIM_CONSTANT_ON       20*RUN_FREQ
#define TIM_CONSTANT_OFF      40*RUN_FREQ
#define TIM_SERVO_ON          26*RUN_FREQ
#define TIM_SERVO_OFF         40*RUN_FREQ
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
   digitalWrite(CONSTANTLEDS, LOW);        // GREEN lights start OFF
   digitalWrite(PULSELEDS,    LOW);       // Pulse leds start OFF
   digitalWrite(REDPULSE,     HIGH);     // Pulse leds start OFF (Active LOW led)
   digitalWrite(MAINROTOR,    LOW);     // Main Rotor starts OFF   
   digitalWrite(TAILROTOR,    LOW);    // Tail Rotor starts OFF   
   
   // All led and motor groups are defined as pin OUTPUT
   pinMode (CONSTANTLEDS, OUTPUT);
   pinMode (PULSELEDS,    OUTPUT);
   pinMode (REDPULSE,     OUTPUT);
   pinMode (MAINROTOR,    OUTPUT);
   pinMode (TAILROTOR,    OUTPUT);
        
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
      if ((TickCounter % 100) == 50)
      {
        digitalWrite(REDPULSE, LOW); // Red Pulse leds switch ON (this led is active LOW) at a different timing
      }
      if ((TickCounter % 100) == 51)
      {
        digitalWrite(REDPULSE, HIGH); // Pulse leds switch OFF just 1/100th of a second later
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

   // ------------------------------ HANDLE THE CONSTANT BURNING LEDS --------------------------------------
   if (TickCounter == TIM_CONSTANT_ON)
   {
      digitalWrite(CONSTANTLEDS, HIGH); // Green leds switch ON
   } 
   if (TickCounter == TIM_CONSTANT_OFF)
   {
      digitalWrite(CONSTANTLEDS, LOW); // Green leds switch OFF
   } 

   // ------------------------------ HANDLE THE SERVO --------------------------------------
   if (TickCounter == TIM_SERVO_ON)
   {
      gunServo.attach(GUNSERVO);
   }

   // Within this timeframe the servo is allowed to move each second.
   if ( (TickCounter >= TIM_SERVO_ON) && (TickCounter < TIM_SERVO_OFF) && ((TickCounter %100) ==0) )
   {
      gunServo.write((unsigned char)random(GUNSERVO_MIN,GUNSERVO_MAX));
   }

   if (TickCounter == TIM_SERVO_OFF)
   {
      gunServo.detach();
   }

   delay(1); // Added this dummy delay() to make sure the code takes more than 1 ms to execute.
}

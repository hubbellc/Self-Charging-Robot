// Written by Caleb Hubbell and Alex Thomposon for ECE 342

//=================================================================
//     Declare Global Variables & Header Files
//=================================================================
// Including Arduino's I2C library, as well as the motorshield, and LCD libraries.
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "HMC5883Llib.h"

//For the RGB sensor digital input
#define S0 4
#define S1 5
#define S2 6
#define S3 7
#define sensorOut 8

// Constants are defined for the physical elements connecting to the Arduino
const int LEFT_LIMIT = 12; // these are pulled up, so other end goes to ground
const int RIGHT_LIMIT = 13;
const int LOW_BAT = 12; //????? Which pin did you put Tonys signal in?

// declare states
int DEFAULT_STATE = 1;
int HIT_RIGHT_STATE = 2;
int HIT_LEFT_STATE = 3;
int HIT_STATE = 4;
int LOW_STATE = 5;
 
 Magnetometer magnetic; //for the digital compass 
 double location_degrees = 0; //tracks the current degree position in relation to the X axis on the compass
 double base_degrees = 0; //predetermined location of the charging base station (Assume 0 degrees North for now)
 
// declare variables
int _currentState;

// adjust to adjust the speed of the robot. Could be tuned to make it drive straight too
int LSpeed = 120;
int RSpeed = 120;

int Back_Delay = 700; // should allow bot to get about 6 inches from wall
int Turn_Delay = 1700; // should allow bot to turn ~90 degrees 

bool Positive_Red = false; //testing for a red color true or false
bool Positive_Green = false; //testing for a green color true or false
bool LtoR = false; // for if we hit a wall while looking for line

// Stores frequency read by the photodiodes
int redFrequency = 0;
int greenFrequency = 0;
int blueFrequency = 0;

// Stores the red green and blue colors
int redColor = 0;
int greenColor = 0;
int blueColor = 0;

int8_t ret;

// Create the motor shield object with the default I2C address
Adafruit_MotorShield motorShield = Adafruit_MotorShield();

// Select which 'port' for each motor.
Adafruit_DCMotor *leftMotor = motorShield.getMotor(3);
Adafruit_DCMotor *rightMotor = motorShield.getMotor(4);

void setup() 
{
 // Setting the outputs
 pinMode(S0, OUTPUT);
 pinMode(S1, OUTPUT);
 pinMode(S2, OUTPUT);
 pinMode(S3, OUTPUT);

 // Setting the sensorOut as an input
 pinMode(sensorOut, INPUT);

 // Setting frequency scaling to 
 digitalWrite(S0,HIGH);
 digitalWrite(S1,LOW);

  // set the amount of gain - Use the most sensitive
  // for reading the earths magnetic field
  // 
  // MSB/Gauss   Field Range
  // 1370     +- 0.88 Ga
  // 1090     +- 1.3 Ga
  // 820      +- 1.9 Ga
  // 660      +- 2.5 Ga
  // 440      +- 4.0 Ga
  // 390      +- 4.7 Ga
  // 330      +- 5.6 Ga
  // 230      +- 8.1 Ga
  magnetic.setGain(HMC5833L_GAIN_1370); //defined by the compass library 
    
  // The motor shield and the serial monitor are initialized
  motorShield.begin();
  Serial.begin(9600);

  // Motors started
  leftMotor->run(RELEASE);
  rightMotor->run(RELEASE);

  // Limit inputs enabled
  pinMode(LEFT_LIMIT, INPUT_PULLUP);
  pinMode(RIGHT_LIMIT, INPUT_PULLUP);
  pinMode(LOW_BAT, INPUT_PULLUP); /////NOTE: this is now active high. Tony will write his pin LOW to signal a change.

  // go to first state
  _currentState = DEFAULT_STATE;
}

// the loop routine runs over and over again forever:
void loop() 
{
  _currentState = LOW_STATE; //// NOTE: for testing, this will default the code to Alex's part.
  
  // check for low battery
  if(digitalRead(LOW_BAT == LOW))
    _currentState = LOW_STATE;

  if(_currentState == DEFAULT_STATE) // Just go forward
  {

    // go forward
    forward();

    // check the limit switches
    if ((digitalRead(RIGHT_LIMIT) == LOW) && (digitalRead(LEFT_LIMIT) == LOW))
    {
      // Center hit
      _currentState = HIT_STATE;
    }
    else if (digitalRead(LEFT_LIMIT) == LOW)
    {
      // Left hit
      _currentState = HIT_LEFT_STATE;
    }
    else if (digitalRead(RIGHT_LIMIT) == LOW)
    {
      // Right hit
      _currentState = HIT_RIGHT_STATE;
    }
  }

  if(_currentState == HIT_RIGHT_STATE) // Backup and turn left
  {
    // backup
    backward();
    delay(Back_Delay);

    // turn left
    left();
    delay(Turn_Delay);

    // go back to main loop
    _currentState = DEFAULT_STATE;
  }

  if(_currentState == HIT_LEFT_STATE) // Backup and turn right
  {
    // backup
    backward();
    delay(Back_Delay);

    // turn right
    right();
    delay(Turn_Delay);

    // go back to main loop
    _currentState = DEFAULT_STATE;
  }

  if(_currentState == HIT_STATE) // Backup and turn around
  {
    // backup
    backward();
    delay(Back_Delay);

    // turn all the way around
    right();
    delay(Turn_Delay * 2);

    // go back to main loop
    _currentState = DEFAULT_STATE;
  }

   if(_currentState == LOW_STATE)
   { 
     return_to_base();
   } 
}



// FUNCTIONS
void forward()
{
  leftMotor->run(FORWARD);
  leftMotor->setSpeed(LSpeed);
  rightMotor->run(FORWARD);
  rightMotor->setSpeed(RSpeed);
}

void backward()
{
  leftMotor->run(BACKWARD);
  leftMotor->setSpeed(LSpeed);
  rightMotor->run(BACKWARD);
  rightMotor->setSpeed(RSpeed);
}

void left()
{
  leftMotor->run(FORWARD);
  leftMotor->setSpeed(30);
  rightMotor->run(FORWARD);
  rightMotor->setSpeed(RSpeed);
}

void right()
{
  leftMotor->run(FORWARD);
  leftMotor->setSpeed(LSpeed);
  rightMotor->run(FORWARD);
  rightMotor->setSpeed(30);
}

 void return_to_base()
 {
   //OPTION #1
   //align 45 degrees to the right in relation to the charging base position
   //move straight forward until a wall is hit then initiate HIT_STATE function to backup and turn around 180 degrees
   //track for a red line while moving forward
          //when red line is found, turn to the left 45 degrees facing the charging station (or align compass to known location)
          //begin by turning on left motor and continuing until red line is no longer sensed
                //then engage right motor until red line is no longer sensed
                //continue until "green" tape is detected and stop
                
    // reads the heading in degrees using the X and Y axis
    ret = magnetic.readHeadingDeg(&location_degrees);

    // start going left
    left(); 

    //MAKE SURE TO FIX THIS ONCE BASE STATION AND COMPASS PLACEMENT IS KNOWN!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // here you should be able to align in two directions
    while((location_degrees != 0) || (location_degrees != 1) || (location_degrees != 359) || (Positive_Red == false)) //add these if accuracy is too harsh
    {
      // Dont delay, will make it bypass the location. Write new left function if you need to. 
      ret = magnetic.readHeadingDeg(&location_degrees); // refresh the value
      
      // check for the line
      getColor();
      if(redColor > greenColor && redColor > blueColor)
        Positive_Red = true;
    }
    // assuming we are guarenteed that it is going R -> L along wall at this point

    // go back straight?
    forward();

    // make sure this logic is right to detect the red stripe
    while(Positive_Red == false)
    {
      // check for the line
      getColor();
      if(redColor > greenColor && redColor > blueColor)
        Positive_Red = true;

      // check for a wall 
      if ((digitalRead(RIGHT_LIMIT) == LOW) || (digitalRead(LEFT_LIMIT) == LOW))
      {
        // hit a wall, oops!
        LtoR = true;
        
        // backup
        backward();
        delay(Back_Delay);
    
        // turn all the way around
        right();
        delay(Turn_Delay * 2);

        // go back straight
        forward();
      }
    }

    // assuming we are guarenteed that red is true

    //if Red is detected, realign with charging station
    //!!!!!!!!!!!!!!MAY NEED TO ADD A DELAY TO ENSURE ALIGNMENT WITH THE RED LINE STAYS TRUE BASED OFF OF COLOR SENSOR PLACEMENT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // reads the heading in degrees using the X and Y axis
    ret = magnetic.readHeadingDeg(&location_degrees);

    // begin turning (based on which direction we are going)
    if(LtoR)
      right();
    else 
      left(); 

    //MAKE SURE TO FIX THIS ONCE BASE STATION AND COMPASS PLACEMENT IS KNOWN!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    while(location_degrees != base_degrees) //while compass is unaligned, rotate until it is aligned
    {
      // Dont delay, will make it bypass the location. Write new left function if you need to. 
      ret = magnetic.readHeadingDeg(&location_degrees); // refresh the value
    }
    // assuming we are guarenteed that it is going toward base at this point. Why do we need to find the line before this? Couldn't we use both compas values to align?

    while(Positive_Green == false)
    {
      // check for the line
      getColor();

      // wait till we loose the line... assuming colors are calibrated correctly
      if(redColor < greenColor && redColor < blueColor)
      {
        // see which way we were going
        if(LtoR)
        {
          left();
          LtoR = false;
        }
        else 
        {
          right(); 
          LtoR = true;
        }
      }
      
      //if green is sensed based on previously calibrated range
      if(greenColor > redColor && greenColor > blueColor)
         Positive_Green = true;
    }

    // assuming we are guarenteed that green line is found. Sit here while charging
    //delay(30); // add delay here if we want to go further into the base
    leftMotor->run(RELEASE); 
    rightMotor->run(RELEASE);
    while(1)
      Serial.println("CHARGING");
   

   //OPTION #2
   //align 45 degrees to the right in relation to the charging base position
   //move straight forward until a wall is hit then initiate HIT_STATE function to backup and turn around 180 degrees
   //track for a red line while moving forward
         //when red line is found, keep going for however many seconds to correct sensor offset from wheels
         //turn to the left 45 degrees facing the charging station 
         //move forward until "green" tape is detected and stop  
 }

void getColor()
{
  // Setting RED (R) filtered photodiodes to be read
  digitalWrite(S2,LOW);
  digitalWrite(S3,LOW);
  
  // Reading the output frequency
  redFrequency = pulseIn(sensorOut, LOW);
  // Remaping the value of the RED (R) frequency from 0 to 255
  // You must replace with your own values. Here's an example: 
  redColor = map(redFrequency, 18, 98, 255,0);
  
  // Setting GREEN (G) filtered photodiodes to be read
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
  
  // Reading the output frequency
  greenFrequency = pulseIn(sensorOut, LOW);
  // Remaping the value of the GREEN (G) frequency from 0 to 255
  // You must replace with your own values. Here's an example: 
  greenColor = map(greenFrequency, 36, 128, 255, 0);
 
  // Setting BLUE (B) filtered photodiodes to be read
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
  
  // Reading the output frequency
  blueFrequency = pulseIn(sensorOut, LOW);
  // Remaping the value of the BLUE (B) frequency from 0 to 255
  // You must replace with your own values. Here's an example: 
  // blueColor = map(blueFrequency, 38, 84, 255, 0);
  blueColor = map(blueFrequency, 16, 97, 255, 0);
}



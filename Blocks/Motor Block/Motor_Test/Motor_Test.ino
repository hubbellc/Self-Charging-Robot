// Written by Caleb Hubbell for ECE 342

//=================================================================
//     Declare Global Variables & Header Files
//=================================================================
// Including Arduino's I2C library, as well as the motorshield, and LCD libraries.
#include <Wire.h>
#include <Adafruit_MotorShield.h>

int LSpeed = 80;
int RSpeed = 80;

// Create the motor shield object with the default I2C address
Adafruit_MotorShield motorShield = Adafruit_MotorShield();

// Select which 'port' for each motor.
Adafruit_DCMotor *leftMotor = motorShield.getMotor(3);
Adafruit_DCMotor *rightMotor = motorShield.getMotor(4);

void setup() 
{
  // The motor shield and the serial monitor are initialized
  motorShield.begin();
  Serial.begin(9600);

  // Motors started
  leftMotor->run(RELEASE);
  rightMotor->run(RELEASE);
}

// the loop routine runs over and over again forever:
void loop() 
{
    // go forward
    leftMotor->run(FORWARD);
    leftMotor->setSpeed(LSpeed);
    rightMotor->run(FORWARD);
    rightMotor->setSpeed(RSpeed);   
}

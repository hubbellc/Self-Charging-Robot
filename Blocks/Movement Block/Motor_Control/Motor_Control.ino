// Written by Caleb Hubbell for ECE 342

//=================================================================
//     Declare Global Variables & Header Files
//=================================================================
// Including Arduino's I2C library, as well as the motorshield, and LCD libraries.
#include <Wire.h>
#include <Adafruit_MotorShield.h>

// Constants are defined for the physical elements connecting to the Arduino
const int LEFT_LIMIT = 12; // these are pulled up, so other end goes to ground
const int RIGHT_LIMIT = 13;
const int BASE_PIN = 0;

// declare states
int DEFAULT_STATE = 1;
int HIT_RIGHT_STATE = 2;
int HIT_LEFT_STATE = 3;
int HIT_STATE = 4;

// declare variables
int _currentState;

// adjust to adjust the speed of the robot. Could be tuned to make it drive straight too
int LSpeed = 120;
int RSpeed = 120;

int Back_Delay = 700; // should allow bot to get about 6 inches from wall
int Turn_Delay = 1700; // should allow bot to turn ~90 degrees

unsigned long refreshTime = 0;
unsigned long stateTime = 0;

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

  // Limit inputs enabled
  pinMode(LEFT_LIMIT, INPUT_PULLUP);
  pinMode(RIGHT_LIMIT, INPUT_PULLUP);
  pinMode(BASE_PIN, INPUT);

  // go to first state
  _currentState = DEFAULT_STATE;
}

// the loop routine runs over and over again forever:
void loop() 
{
  // see if at charger
  if(digitalRead(BASE_PIN) == HIGH)
  {
    leftMotor->run(RELEASE);
    rightMotor->run(RELEASE);
  }

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

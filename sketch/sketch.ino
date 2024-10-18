/****************************************************************************** 
SparkFun Easy Driver Basic Demo
Toni Klopfenstein @ SparkFun Electronics
March 2015
https://github.com/sparkfun/Easy_Driver

Simple demo sketch to demonstrate how 5 digital pins can drive a bipolar stepper motor,
using the Easy Driver (https://www.sparkfun.com/products/12779). Also shows the ability to change
microstep size, and direction of motor movement.

Development environment specifics:
Written in Arduino 1.6.0

This code is beerware; if you see me (or any other SparkFun employee) at the local, and you've found our code helpful, please buy us a round!
Distributed as-is; no warranty is given.

Example based off of demos by Brian Schmalz (designer of the Easy Driver).
http://www.schmalzhaus.com/EasyDriver/Examples/EasyDriverExamples.html
******************************************************************************/
//Declare pin functions on syringe stepper driver
#define PIN_STEP 2
#define PIN_DIR 3
#define MS1 4
#define MS2 5
#define EN  6

//Declare pin functions on stirrer motor driver
#define B1A 8
#define B1B 9

//Declare pin functions on stirrer motor potentiometer
#define POT A0

//Declare pin functions on rotary encoder
#define BUTTON 21
#define ROT_ENC_D3 19
#define ROT_ENC_D2 20

//Declare pin functions on LCD
#define POT_ A0

//Declare pin functions on SD card module
#define CS 10
#define MOSI 11
#define SCK 12
#define MISO 13

//Declare pin functions on end switches
#define POT__ A0

//Declare pin functions on thermistor
#define POT___ A1

//Declare variables for functions
char user_input;
int x;
int y;
int state;

char BUTTON_WAS_PRESSED = 0;
char ROT_ENC_D2_WAS_PRESSED = 0;
char ROT_ENC_D3_WAS_PRESSED = 0;
char ROT_ENC_STATE = 0;

void buttonInterrupt() {
  BUTTON_WAS_PRESSED = 1;
}
void rotEncD2Interrupt() {
  char high = digitalRead(ROT_ENC_D2);
  switch(ROT_ENC_STATE) {
    case 0:
      if (high){
        ROT_ENC_STATE = 1;
      }
      break;
    case 2:
      if (!high){
        ROT_ENC_STATE = 3;
      }
      break;
      
    case 5:
      if (high){
        ROT_ENC_STATE = 6;
      }
      break;
    case 7:
      if (!high){
        ROT_ENC_STATE = 8;
      }
      break;
  }
}
void rotEncD3Interrupt() {
  char high = digitalRead(ROT_ENC_D3);
  switch(ROT_ENC_STATE) {
    
    case 1:
      if(high){
        ROT_ENC_STATE = 2;
      }
      break;
    case 3:
      if(!high){
        ROT_ENC_STATE = 4;
      }
      break;


    case 0:
      if(high){
        ROT_ENC_STATE = 5;
      }
      break;
    case 6:
      if(!high){
        ROT_ENC_STATE = 7;
      }
      break;

  }
}

void setup() {
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_DIR, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(EN, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(ROT_ENC_D2, INPUT_PULLUP);
  pinMode(ROT_ENC_D3, INPUT_PULLUP);
  
  resetEDPins(); //Set step, direction, microstep and enable pins to default states
  Serial.begin(9600); //Open Serial connection for debugging
  Serial.println("Begin motor control");
  Serial.println();

  
  int interrupt_id = digitalPinToInterrupt(BUTTON);
  if (interrupt_id == -1) {
    Serial.println("Error: BUTTON is not an interrupt pin");
    exit(1);
  }
  String s = String("For BUTTON pin is  ");
  s.concat(BUTTON);
  s.concat(" and interrupt ID is ");
  s.concat(interrupt_id);
  Serial.println(s);
  attachInterrupt(interrupt_id, buttonInterrupt, FALLING);
  Serial.println("Interrupt attached");
  
  interrupt_id = digitalPinToInterrupt(ROT_ENC_D2);
  if (interrupt_id == -1) {
    Serial.println("Error: ROT_ENC_D2 is not an interrupt pin");
    exit(1);
  }
  s = String("For ROT_ENC_D2 pin is ");
  s.concat(ROT_ENC_D2);
  s.concat(" and interrupt ID is ");
  s.concat(interrupt_id);
  Serial.println(s);
  attachInterrupt(interrupt_id, rotEncD2Interrupt, CHANGE);
  Serial.println("Interrupt attached");
  
  interrupt_id = digitalPinToInterrupt(ROT_ENC_D3);
  if (interrupt_id == -1) {
    Serial.println("Error: ROT_ENC_D3 is not an interrupt pin");
    exit(1);
  }
  s = String("For ROT_ENC_D3 pin is ");
  s.concat(ROT_ENC_D3);
  s.concat(" and interrupt ID is ");
  s.concat(interrupt_id);
  Serial.println(s);
  attachInterrupt(interrupt_id, rotEncD3Interrupt, CHANGE);
  Serial.println("Interrupt attached");

  //Print function list for user selection
  Serial.println("Enter number for control option:");
  Serial.println("1. Turn at default microstep mode.");
  Serial.println("2. Reverse direction at default microstep mode.");
  Serial.println("3. Turn at 1/8th microstep mode.");
  Serial.println("4. Step forward and reverse directions.");
  Serial.println();
}

//Main loop
void loop() {
  while(Serial.available() || BUTTON_WAS_PRESSED || ROT_ENC_STATE == 8 || ROT_ENC_STATE == 4){
      
      if (BUTTON_WAS_PRESSED == 1) {
        Serial.println("BUTTON interrupt fired");
        BUTTON_WAS_PRESSED = 0;
      }
      else if (ROT_ENC_STATE == 4) {
        Serial.println("ROT_ENC is a Right");
          StepForwardDefault();
        ROT_ENC_STATE = 0;
      }
      else if (ROT_ENC_STATE == 8) {
        Serial.println("ROT_ENC is a Left");
          ReverseStepDefault();
        ROT_ENC_STATE = 0;
      }
      else if (Serial.available()){
        user_input = Serial.read(); //Read user input and trigger appropriate function
        digitalWrite(EN, LOW); //Pull enable pin low to allow motor control
        if (user_input == '1')
        {
          StepForwardDefault();
        }
        else if(user_input == '2')
        {
          ReverseStepDefault();
        }
        else if(user_input == '3')
        {
          SmallStepMode();
        }
        else if(user_input == '4')
        {
          ForwardBackwardStep();
        }
        else if(user_input == 'F' || user_input == 'R' || user_input == 'f' || user_input == 'r')
        {
          ShanesCustomCrapRoutine(user_input);
        }
        else
        {
          Serial.println("Invalid option entered.");
        }
      }
      resetEDPins();
  }
}

//Reset Easy Driver pins to default states
void resetEDPins()
{
  digitalWrite(PIN_STEP, LOW);
  digitalWrite(PIN_DIR, LOW);
  digitalWrite(MS1, LOW);
  digitalWrite(MS2, LOW);
  digitalWrite(EN, HIGH);
  
}

char* MESSAGETOTHEWORLD = "Doing whatever Shanes routine does";

const int BIG_REPS_TO_360_DEGREES = 1000;
const int SMOL_REPS_TO_360_DEGREES = 8000;

// Definitely Not Default
void ShanesCustomCrapRoutine(char direction)
{
  Serial.println(MESSAGETOTHEWORLD);
  String user_input_str = Serial.readStringUntil(',');
  int input_reps = user_input_str.toInt();
  user_input_str = Serial.readStringUntil(',');
  int input_delay1 = user_input_str.toInt();
  user_input_str = Serial.readString();
  int input_delay2 = user_input_str.toInt();
  String s = String("Repetitions:");
  s.concat(input_reps);
  s.concat(" Delay:");
  s.concat(input_delay1);
  s.concat(" DelayBetweenReps:");
  s.concat(input_delay2);
  Serial.println(s);

  if (direction == 'R' || direction == 'r') {
    digitalWrite(PIN_DIR, HIGH); //Pull direction pin high to move "backward"
  } else {
    digitalWrite(PIN_DIR, LOW); //Pull direction pin low to move "forward"
  }

  if (direction == 'f' || direction == 'r') {
    digitalWrite(MS1, HIGH); //Pull MS1, and MS2 high to set logic to 1/8th microstep resolution
    digitalWrite(MS2, HIGH);
  }

  for(x= 0; x<input_reps; x++)  //Loop the forward stepping enough times for motion to be visible
  {
    digitalWrite(PIN_STEP,HIGH); //Trigger one step forward
    delay(input_delay1);
    digitalWrite(PIN_STEP,LOW); //Pull step pin low so it can be triggered again
    delay(input_delay1);
    if(input_delay2) {
      delay(input_delay2);
    }
  }
  Serial.println("Enter new option");
  Serial.println();
}

//Default microstep mode function
void StepForwardDefault()
{
  Serial.println("Moving forward at default step mode.");
  digitalWrite(PIN_DIR, LOW); //Pull direction pin low to move "forward"
  for(x= 0; x<BIG_REPS_TO_360_DEGREES/8; x++)  //Loop the forward stepping enough times for motion to be visible
  {
    digitalWrite(PIN_STEP,HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(PIN_STEP,LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
  Serial.println("Enter new option");
  Serial.println();
}

//Reverse default microstep mode function
void ReverseStepDefault()
{
  Serial.println("Moving in reverse at default step mode.");
  digitalWrite(PIN_DIR, HIGH); //Pull direction pin high to move in "reverse"
  for(x= 0; x<BIG_REPS_TO_360_DEGREES/8; x++)  //Loop the stepping enough times for motion to be visible
  {
    digitalWrite(PIN_STEP,HIGH); //Trigger one step
    delay(1);
    digitalWrite(PIN_STEP,LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
  Serial.println("Enter new option");
  Serial.println();
}

// 1/8th microstep foward mode function
void SmallStepMode()
{
  Serial.println("Stepping at 1/8th microstep mode.");
  digitalWrite(PIN_DIR, HIGH); //Pull direction pin low to move "forward"
  digitalWrite(MS1, HIGH); //Pull MS1, and MS2 high to set logic to 1/8th microstep resolution
  digitalWrite(MS2, HIGH);
  for(x= 0; x<50; x++)  //Loop the forward stepping enough times for motion to be visible
  {
    digitalWrite(PIN_STEP,HIGH); //Trigger one step forward
    delay(4);
    digitalWrite(PIN_STEP,LOW); //Pull step pin low so it can be triggered again
    delay(4);
  }
  Serial.println("Enter new option");
  Serial.println();
}

//Forward/reverse stepping function
void ForwardBackwardStep()
{
  Serial.println("Alternate between stepping forward and reverse.");
  for(x= 1; x<5; x++)  //Loop the forward stepping enough times for motion to be visible
  {
    //Read direction pin state and change it
    state=digitalRead(PIN_DIR);
    if(state == HIGH)
    {
      digitalWrite(PIN_DIR, LOW);
    }
    else if(state ==LOW)
    {
      digitalWrite(PIN_DIR,HIGH);
    }
    
    for(y=0; y<1000; y++)
    {
      digitalWrite(PIN_STEP,HIGH); //Trigger one step
      delay(1);
      digitalWrite(PIN_STEP,LOW); //Pull step pin low so it can be triggered again
      delay(1);
    }
  }
  Serial.println("Enter new option:");
  Serial.println();
}

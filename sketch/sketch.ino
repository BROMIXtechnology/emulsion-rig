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

//Declare pin functions on LCD
#define POT A0

//Declare pin functions on SD card module
#define CS 10
#define MOSI 11
#define SCK 12
#define MISO 13

//Declare pin functions on end switches
#define POT A0

//Declare pin functions on thermistor
#define POT A1

//Declare pin functions on LCD
#define NOKIA_RST A12
#define NOKIA_CE A11
#define NOKIA_DC A13
#define NOKIA_DIN A14
#define NOKIA_CLK A15
#define NOKIA_NOTUSED A10

//Declare pin functions on rotary encoder
#define BUTTON 21
#define ROT_ENC_D3 19
#define ROT_ENC_D2 20

#define BIG_BUTTON 14

#include <NOKIA5110_TEXT.h>


// Create an LCD object
NOKIA5110_TEXT mylcd(NOKIA_RST, NOKIA_CE, NOKIA_DC, NOKIA_DIN, NOKIA_CLK);
#define inverse  false
#define contrast 0xBF // default is 0xBF set in LCDinit, Try 0xB1 - 0xBF if your display is too dark/dim
#define bias 0x12 // LCD bias mode 1:48: Try 0x12, 0x13 or 0x14

/**
 * Or, if you would like to control the backlight on your own, init the lcd without the last argument
 * Nokia_LCD lcd(13, 12, 11, 10, 9);
*/

//Declare variables for functions
char user_input;
int x;
int y;
int state;

int rotary = 20;
int previous_rotary = 20;

#include <EncoderButton.h>
EncoderButton eb1(ROT_ENC_D2, ROT_ENC_D3, BUTTON);
EncoderButton eb2(BIG_BUTTON);
bool pushed_the_button_like_the_sugababes = false;

// Create one or more callback functions 
void onEb1Encoder(EncoderButton& eb) {
  Serial.print("eb1 incremented by: ");
  Serial.println(eb.increment());
  Serial.print("eb1 position is: ");
  Serial.println(eb.position());
  rotary += eb.increment();
}

void onEb1Button(EncoderButton& eb) {
  if (eb.isPressed()) {
    pushed_the_button_like_the_sugababes = true;
  }
}

void setup() {
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_DIR, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(EN, OUTPUT);
  resetEDPins(); //Set step, direction, microstep and enable pins to default states
  Serial.begin(9600); //Open Serial connection for debugging
  Serial.println("Begin motor control");
  Serial.println();
  //Print function list for user selection
  Serial.println("Enter number for control option:");
  Serial.println("1. Turn at default microstep mode.");
  Serial.println("2. Reverse direction at default microstep mode.");
  Serial.println("3. Turn at 1/8th microstep mode.");
  Serial.println("4. Step forward and reverse directions.");
  Serial.println();
  mylcd.LCDInit(inverse, contrast, bias); // init the lCD
  mylcd.LCDClear(); // clear whole screen
  mylcd.LCDFont(LCDFont_Default); // Font 1 default
  Serial.println("Okay now the LCD should be on.");
  mylcd.LCDString("doit doit doit");
  delay(5);
  
  eb1.setEncoderHandler(onEb1Encoder);
  eb1.setClickHandler(onEb1Button);
  eb2.setClickHandler(onEb1Button);
}

//Main loop
void loop() {
  eb1.update();
  if (pushed_the_button_like_the_sugababes){
    GoForwardQuiteABit();
    pushed_the_button_like_the_sugababes = false;
  }
  if (rotary != previous_rotary) {
      
    mylcd.LCDClear(); // clear whole screen
    mylcd.LCDgotoXY(rotary, 2);
    mylcd.LCDString("*");

    previous_rotary = rotary;
    
  }
  while(Serial.available()){
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
  
  mylcd.LCDClear(); // clear whole screen
  mylcd.LCDgotoXY(0, 0);
  mylcd.LCDString(s.c_str());

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

void GoForwardQuiteABit()
{
    mylcd.LCDClear(); // clear whole screen
    mylcd.LCDgotoXY(5, 5);
    mylcd.LCDString("How nice, it's moving");

    digitalWrite(PIN_DIR, LOW); //Pull direction pin low to move "forward"
    
    int input_reps = 30000;
    int input_delay1 = 2;

    for(x= 0; x<input_reps; x++)  //Loop the forward stepping enough times for motion to be visible
    {
      digitalWrite(PIN_STEP,HIGH); //Trigger one step forward
      delay(input_delay1);
      digitalWrite(PIN_STEP,LOW); //Pull step pin low so it can be triggered again
      delay(input_delay1);
    }
    mylcd.LCDClear(); // clear whole screen
    mylcd.LCDgotoXY(5, 5);
    mylcd.LCDString("Phew that's done");
}

//Default microstep mode function
void StepForwardDefault()
{
  Serial.println("Moving forward at default step mode.");
  digitalWrite(PIN_DIR, LOW); //Pull direction pin low to move "forward"
  for(x= 0; x<3000; x++)  //Loop the forward stepping enough times for motion to be visible
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
  for(x= 0; x<1000; x++)  //Loop the stepping enough times for motion to be visible
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
  for(x= 0; x<2000; x++)  //Loop the forward stepping enough times for motion to be visible
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

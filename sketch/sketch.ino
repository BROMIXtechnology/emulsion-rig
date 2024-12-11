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

#define BIG_BUTTON 18

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

int rotary = 0;
int previous_rotary = 1;

#include <EncoderButton.h>
EncoderButton eb1(ROT_ENC_D2, ROT_ENC_D3);
// EncoderButton eb2(BIG_BUTTON);
Bounce2::Button button2 = Bounce2::Button();
Bounce2::Button button3 = Bounce2::Button();
bool pushed_the_button_like_the_sugababes = false;


#define BIG_REPS_TO_360_DEGREES 1000
#define BIG_REPS_TO_1ML BIG_REPS_TO_360_DEGREES
#define SMOL_REPS_TO_360_DEGREES 8000

// flow rate in ml/minute
// minimum bound 0.1ml/minute
// maximum bound 60ml in 30 seconds = 120ml/minute

// FLOW RATE is tenth of ml per minute

#define PIZZA_TABLE_IS_EMPTY 0
#define PIZZA_ORDERED 1
#define PIZZA_SPINNING 2
#define PIZZA_BAKING 3
#define PIZZA_SERVED 4

int making_the_pizza = PIZZA_TABLE_IS_EMPTY;

double last_measured_millis = 1;
double delta_millis = 0;
double millis_since_rep = 0;
double millis_since_start = 0;
double millis_value_that_changes = 15;
bool last_switch_state = false;
double total_rotations = 0;

float START_ML_PER_MINUTE = 0.5;
float END_ML_PER_MINUTE = 3.5;
float ACCELERATION_PER_MINUTE = 0.1;
float current_ml_per_minute = START_ML_PER_MINUTE;

// Create one or more callback functions 
void onEb1Encoder(EncoderButton& eb) {
  // Serial.print("eb1 incremented by: ");
  // Serial.println(eb.increment());
  Serial.print("flow rate: ");
  float flowRate = float(rotary) / 100;
  Serial.print(flowRate);
  Serial.print("ml/minute");
  // float rotationsPerSecond = flowRate / 60;
  // Serial.print("; ");
  // Serial.print(rotationsPerSecond);
  // Serial.print("rot/s");
  // float rotationsPerMillisecond = rotationsPerSecond * 1000;
  // Serial.print("; ");
  // Serial.print(rotationsPerMillisecond);
  // Serial.print("rot/ms");
  // float millisecondsPerRot = 1000 / rotationsPerSecond;
  // Serial.print("; ");
  // Serial.print(millisecondsPerRot);
  // Serial.print("ms/rot");
  // Serial.println("");
  // float millisecondsPerSwitch = millisecondsPerRot / BIG_REPS_TO_360_DEGREES;
  // Serial.print("; ");
  // Serial.print(millisecondsPerSwitch);
  // Serial.print("ms/switch");
  Serial.println("");
  rotary += eb.increment();
}

void setup() {
  button2.attach( BIG_BUTTON, INPUT_PULLUP ); // USE EXTERNAL PULL-UP
  button3.attach( BUTTON, INPUT_PULLUP ); // USE EXTERNAL PULL-UP
  button2.interval(5);
  button3.interval(5);
  button2.setPressedState(LOW); 
  button3.setPressedState(LOW); 
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

  last_measured_millis = millis();
  
  eb1.setEncoderHandler(onEb1Encoder);
  // eb1.setClickHandler(onEb1Button);
  // eb2.setClickHandler(onEb1Button);
}

//Main loop
void loop() {
  double new_millis = millis();
  delta_millis = new_millis - last_measured_millis;
  last_measured_millis = new_millis;
  eb1.update();
  button2.update();
  button3.update();
  if (button2.pressed() || button3.pressed()) {
    switch(making_the_pizza) {
      case PIZZA_TABLE_IS_EMPTY: {
        making_the_pizza = PIZZA_SPINNING;
        StartPizza();
        break;
      }
      case PIZZA_SPINNING: {
        making_the_pizza = PIZZA_TABLE_IS_EMPTY;
        StopPizza();
        break;
      }
    }
  }
  if (making_the_pizza == PIZZA_SPINNING){
    SpinThePie();
  }
  if (rotary != previous_rotary) {
      
    mylcd.LCDClear(); // clear whole screen
    mylcd.LCDgotoXY(rotary, 2);
    mylcd.LCDString("*");

    previous_rotary = rotary;
    
  }
  // while(Serial.available()){
  //     user_input = Serial.read(); //Read user input and trigger appropriate function
  //     digitalWrite(EN, LOW); //Pull enable pin low to allow motor control
  //     if (user_input == '1')
  //     {
  //        StepForwardDefault();
  //     }
  //     else if(user_input == '2')
  //     {
  //       ReverseStepDefault();
  //     }
  //     else if(user_input == '3')
  //     {
  //       SmallStepMode();
  //     }
  //     else if(user_input == '4')
  //     {
  //       ForwardBackwardStep();
  //     }
  //     else if(user_input == 'F' || user_input == 'R' || user_input == 'f' || user_input == 'r')
  //     {
  //       ShanesCustomCrapRoutine(user_input);
  //     }
  //     else
  //     {
  //       Serial.println("Invalid option entered.");
  //     }
  //     resetEDPins();
  // }
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

// // Definitely Not Default
// void ShanesCustomCrapRoutine(char direction)
// {
//   Serial.println(MESSAGETOTHEWORLD);
//   String user_input_str = Serial.readStringUntil(',');
//   int input_reps = user_input_str.toInt();
//   user_input_str = Serial.readStringUntil(',');
//   int input_delay1 = user_input_str.toInt();
//   user_input_str = Serial.readString();
//   int input_delay2 = user_input_str.toInt();
//   String s = String("Repetitions:");
//   s.concat(input_reps);
//   s.concat(" Delay:");
//   s.concat(input_delay1);
//   s.concat(" DelayBetweenReps:");
//   s.concat(input_delay2);
//   Serial.println(s);
  
//   mylcd.LCDClear(); // clear whole screen
//   mylcd.LCDgotoXY(0, 0);
//   mylcd.LCDString(s.c_str());

//   if (direction == 'R' || direction == 'r') {
//     digitalWrite(PIN_DIR, HIGH); //Pull direction pin high to move "backward"
//   } else {
//     digitalWrite(PIN_DIR, LOW); //Pull direction pin low to move "forward"
//   }

//   if (direction == 'f' || direction == 'r') {
//     digitalWrite(MS1, HIGH); //Pull MS1, and MS2 high to set logic to 1/8th microstep resolution
//     digitalWrite(MS2, HIGH);
//   }

//   for(x= 0; x<input_reps; x++)  //Loop the forward stepping enough times for motion to be visible
//   {
//     digitalWrite(PIN_STEP,HIGH); //Trigger one step forward
//     delay(input_delay1);
//     digitalWrite(PIN_STEP,LOW); //Pull step pin low so it can be triggered again
//     delay(input_delay1);
//     if(input_delay2) {
//       delay(input_delay2);
//     }
//   }
//   Serial.println("Enter new option");
//   Serial.println();
// }

void StartPizza()
{
    mylcd.LCDClear(); // clear whole screen
    mylcd.LCDgotoXY(0, 0);
    mylcd.LCDString("How nice, it's moving");
  // digitalWrite(MS1, HIGH); //Pull MS1, and MS2 high to set logic to 1/8th microstep resolution
  // digitalWrite(MS2, HIGH);

    digitalWrite(PIN_DIR, LOW); //Pull direction pin low to move "forward"
    millis_since_rep = 0;
    millis_since_start = 0;
    millis_value_that_changes = rotary;

    current_ml_per_minute = START_ML_PER_MINUTE;
}
void SpinThePie() {
  millis_since_start += delta_millis;
  millis_since_rep += delta_millis;

  float delta_minutes = float(delta_millis) / 1000.0 / 60;

  if (current_ml_per_minute < END_ML_PER_MINUTE) {
    current_ml_per_minute += ACCELERATION_PER_MINUTE * delta_minutes;
  }

  // BIG_REPS_TO_1ML
  float reps_per_minute = current_ml_per_minute * float(BIG_REPS_TO_1ML);
  float millis_per_minute = 60000;
  long millis_between_reps = long(millis_per_minute / reps_per_minute);

  if (millis_since_rep > millis_between_reps) {
      digitalWrite(PIN_STEP,false); //Trigger one step forward
      delayMicroseconds(600);
      digitalWrite(PIN_STEP,true); //Trigger one step forward
      millis_since_rep = 0;
      total_rotations += 1;
      // if (total_rotations % 8 == 0){
      //   Serial.print("rotations: ");
      //   Serial.println(total_rotations);
      // }
      delayMicroseconds(50);
  }
  // if (millis_since_start > 2000) {
  //   millis_value_that_changes = 50;
  // } else if (millis_since_start > 4000)
  // {
  //   millis_value_that_changes = 40;
  // } else if (millis_since_start > 6000)
  // {
  //   millis_value_that_changes = 30;
  // } else if (millis_since_start > 8000)
  // {
  //   millis_value_that_changes = 20;
  // } else if (millis_since_start > 10000)
  // {
  //   millis_value_that_changes = 5;
  // }
  
  if (millis_since_start > 40000) {
    StopPizza();
  }
}

void StopPizza() {
    mylcd.LCDClear(); // clear whole screen
    mylcd.LCDgotoXY(0, 0);
    mylcd.LCDString("Phew that's done");
    making_the_pizza = PIZZA_TABLE_IS_EMPTY;
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

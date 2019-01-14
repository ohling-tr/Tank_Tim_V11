/*************************************************************
  
 ______     __  __     __     __         _____        ______        ______   ______     __   __     __  __    .......(\_/) 
/\  == \   /\ \/\ \   /\ \   /\ \       /\  __-.     /\  __ \      /\__  _\ /\  __ \   /\ "-.\ \   /\ \/ /     ......( '_') 
\ \  __<   \ \ \_\ \  \ \ \  \ \ \____  \ \ \/\ \    \ \  __ \     \/_/\ \/ \ \  __ \  \ \ \-.  \  \ \  _"-.    ..../""""""""""""\======░ ▒▓▓█D    
 \ \_____\  \ \_____\  \ \_\  \ \_____\  \ \____-     \ \_\ \_\       \ \_\  \ \_\ \_\  \ \_\\"\_\  \ \_\ \_\     /"""""""""""""""""""""""\   
  \/_____/   \/_____/   \/_/   \/_____/   \/____/      \/_/\/_/        \/_/   \/_/\/_/   \/_/ \/_/   \/_/\/_/     \_@_@_@_@_@_@_@_/
                                                                                                              

  This sketch was based on existing sketches from the following:

    DFRobot:  https://www.dfrobot.com/blog-494.html
    jlmyra:   https://github.com/jlmyra/Arduino-Blynk-Joystick-4-Motor-Robot-Rover

 
 *************************************************************
  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc
    Blynk community:            http://community.blynk.cc
    Social networks:            http://www.fb.com/blynkapp
                                http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.
  
 *************************************************************

  This example shows how to use Arduino with HC-08
  Bluetooth BLE module to connect your project to Blynk.

  Feel free to apply it to any other example. It's simple!

  NOTE: Bluetooth support is in beta!

  You can receive x and y coords for joystick movement within App.

  App project setup:
    Two Axis Joystick on V2 in MERGE output mode.
    MERGE mode means device will receive both x and y within 1 message
 *************************************************************/

/* Comment this out to disable prints and save space */

/* Tim Ohling
  V11 cleans up the IF structure some, adds methods for commonly repeated code
  in the provided base, and uses the Adafruit I2C V2 motor shield
*/

#define BLYNK_USE_DIRECT_CONNECT
#define BLYNK_PRINT Serial
#include <BlynkSimpleSerialBLE.h>
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include <SoftwareSerial.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "d3ed6502233241faad6a0f1ffdd2523c"; // <<< INSERT YOUR AUTH CODE HERE

SoftwareSerial SerialBLE(10, 11); // RX, TX

//*************************************************************

Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *motorDriveRight = AFMS.getMotor(3); 
Adafruit_DCMotor *motorDriveLeft = AFMS.getMotor(4);
int state = 0;

//######### SETUP ######################################
void setup()
{
  Serial.begin(9600);
  delay(1000);
  
  SerialBLE.begin(9600);
  delay(5000);
  SerialBLE.print("AT+NAMEOhling");
  Blynk.begin(SerialBLE, auth);
  Serial.println("Waiting for connections...");
  Serial.println("Adafruit Motorshield v1 - DC Motor");

  AFMS.begin();

  // Set the speed to start, from 0 (off) to 255 (max speed)
  motor_set_speed(0,0);
}

//**********VOID LOOP**********
void loop()
{
  Blynk.run(); // To Run Blynk
}
//**********END VOID LOOP**********

//*******************************************************
// common joystick logging to serial (use serial monitor on PC)
// tro 6/7/2018
//
void joystick_log(String joystickTxt, int x_value, int y_value) {
  Serial.print(joystickTxt);
  Serial.print("  x_direction  ");
  Serial.print(x_value);
  Serial.print("  y_direction  ");
  Serial.println(y_value);
}

//*******************************************************
// common motor run passing direction values (defined as integers)
// tro 6/7/2018
//
void motor_run(int leftMotorDir, int rightMotorDir) {
  motorDriveRight->run(rightMotorDir);
  motorDriveLeft->run(leftMotorDir); 
}

void motor_set_speed(int leftSpeed, int rightSpeed) {
  motorDriveRight->setSpeed(rightSpeed);
  motorDriveLeft->setSpeed(leftSpeed);  
}

//**********Blynk Subroutines**********

//**********Set the Motor Speed**********
// This function sets the motor speed via Blynk Slider Object
// the variable pinValue ranges from 0-255
// On the phone app set the Slider Ouput to Virtual V2
// This function will set the speed

BLYNK_WRITE(V2)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  // You can also use:
  // String i = param.asStr();
  // double d = param.asDouble();
  Serial.print("V2 Slider value is: ");
  Serial.println(pinValue);
  motor_set_speed(pinValue, pinValue);
}
//**********END Set the Motor Speed**********


//**********Translate the Joystick Position to a Rover Direction**********
//
//This function translates the joystick movement to a Rover direction.
//Blynk Joysick is centered at y=128, x=128 with a range of 0-255. Thresholds set the Joystick 
//sensitivity. These are my adjustments for my touch, you may need something different. Making
//the range too tight will make the rover hard to control. Note: Y values greater than 128 will
//drive the motors FOWARD. Y values less than 128 will drive the motorS in REVERSE. The Rover will
//turn in the direction of the "slow" or unpowered (RELEASE) wheels.
//
//  Joystick Movement along x, y Axis
// (Inside the * is the Threshold Area)
//            y=255--(y_position=255, x_position=128; y_direction=+1, x_direction=0)
//           * | *
//           * | *
//           * | *
//   ********* | *********
//x=0---------------------x=255--(y_position=128, x_position=255; y_direction=0, x_direction=0)
//   ********* | *********
//           * | *
//           * | * (Inside the * is the Threshold Area)
//           * | *
//            y=0--(y_position=0, x_position=128; y_direction=-1, x_direction=0)

BLYNK_WRITE(V0) {
  const int X_THRESHOLD_LOW = 108; //X: 128 - 20
  const int X_THRESHOLD_HIGH = 148; //X: 128 + 20   

  const int Y_THRESHOLD_LOW = 108;
  const int Y_THRESHOLD_HIGH = 148;
      
  int x_position = param[0].asInt();  //Read the Blynk Joystick x Position 0-255
  int y_position = param[1].asInt();  //Read the Blynk Joystick y Position 0-255

  int x_direction;  //Variable for Direction of Joystick Movement: x= -1, 0, 1
  int y_direction;  //Variable for Direction of Joystick Movement: y= -1, 0, 1
 
  Serial.print("x_position: ");
  Serial.print(x_position);
  Serial.print("  y_position: ");
  Serial.println(y_position);

//Determine the direction of the Joystick Movement

  x_direction = 0;
  y_direction = 0;

  if (x_position > X_THRESHOLD_HIGH) {
    x_direction = 1;
  } else if (x_position < X_THRESHOLD_LOW) {
    x_direction = -1;
  }
  if (y_position > Y_THRESHOLD_HIGH) {
    y_direction = 1;
  } else if (y_position < Y_THRESHOLD_LOW) {
    y_direction = -1;
  }
//if x and y are within the threshold their values then x_direction = 0 and y_direction = 0

//Move the Rover (Rover will move in the direction of the slower wheels)
//0,0(Stop); 0,1(Forward); 0,-1(Backward); 1,1(Right up diagonal); 1,0(Right); 1,-1(Right down diagonal);
//-1,0(Left); -1,1(Left up diagonal); -1,-1(Left down diagonal)

//x = -1 and y = -1 Back Diagonal Left
    if (x_direction == -1) 
    if (y_direction == -1) {
        joystick_log("JOYSTICK: left-down DIRECTION: BACKWARD SOFT LEFT", x_direction, y_direction);
        motor_run(RELEASE, BACKWARD);
//x = -1 and y = 0 Left on x axis     
      } else if (y_direction == 0) {
        joystick_log("JOYSTICK: left DIRECTION: HARD LEFT (ROTATE COUNTER-CLOCKWISE)", x_direction, y_direction);
        motor_run(BACKWARD, FORWARD);
//x = -1 and y = 1 Forward Diagonal Left   
        } else {
        //y_direction == 1
/*        joystick_log("JOYSTICK left-up DIRECTION: FORWARD SOFT LEFT", x_direction, y_direction);
        motor_run(CHIK-FIL-A, FIVE GUYS); // ***FIGURE IT OUT***
*/
//x = 0 and y = -1 Backward
      } else 
        if (x_direction == 0) 
        if (y_direction == -1) {
        joystick_log("JOYSTICK down DIRECTION BACKWARD", x_direction, y_direction);
        motor_run(BACKWARD, BACKWARD);
//x = 0 and y = 0 Stop
      } else if (y_direction == 0) {
        joystick_log("JOYSTICK: centered DIRECTION: STOP", x_direction, y_direction);
        motor_run(RELEASE, RELEASE);
//x = 0 and y = 1 Forward 
      } else {
        //y_direction == 1
        joystick_log("JOYSTICK: up DIRECTION: FORWARD", x_direction, y_direction);
        motor_run(FORWARD, FORWARD);
//x = 1 and y = -1 Backward Diagonal Right
    } else 
        //x_direction == 1
        if (y_direction == -1){ 
/*        joystick_log("JOYSTICK right-down DIRECTION: BACKWARD SOFT RIGHT", x_direction, y_direction);
          motor_run(OLIVE GARDEN, MUCHAS GRACIAS);  ***FIGURE IT OUT***
*/        
//x = 1 and y = 0 Right on x-axis
      } else 
        if (y_direction == 0){ 
        joystick_log("JOYSITCK: right DIRECTION: HARD RIGHT (CLOCKWISE SPIN)", x_direction, y_direction);
        motor_run(FORWARD, BACKWARD);
//x = 1 and y = 1 Forward Diagonal Right
    } else { 
        //y_direction == 1
/*      joystick_log("JOYSTICK: right-up DIRECTION: SOFT RIGHT", x_direction, y_direction);
        motor_run(POR QUE NO, I'M HUNGRY, CAN YOU TELL?);  ***FIGURE IT OUT***
*/     
      }
}

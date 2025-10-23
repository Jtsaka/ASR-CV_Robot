#include "contServoNoPID.h"
#include "encoder.h"

//Module 1
Encoder encoderA(A1);
contServo servoA(3, encoderA);
Spark sparkA(7); 
//Module 2
Encoder encoderB(A2); 
contServo servoB(4, encoderB); 
Spark sparkB(8); 
//Module 3
Encoder encoderC(A3); 
contServo servoC(5, encoderC); 
Spark sparkC(9); 
//Module 4
Encoder encoderD(A4); 
contServo servoD(6, encoderD); 
Spark sparkD(10); 

const int MOD_1_OFFSET = 290;
const int MOD_2_OFFSET = 47;
//Module 3 needs to be handled with a map
const int MOD_4_OFFSET = 200;

int targetAngle1 = MOD_1_OFFSET; //module1
int targetAngle2 = MOD_2_OFFSET; //module2
int targetAngle3 = 60; //module3
int targetAngle4 = MOD_4_OFFSET; //module4


int defaultSpd = 1500;
int motorSpd = defaultSpd;
int inc = 500; 

void setup() {
  Serial.begin(500000); //baud rate

  servoA.initialize();
  servoB.initialize();
  servoC.initialize();
  servoD.initialize();

  sparkA.initialize();
  sparkB.initialize();
  sparkC.initialize();
  sparkD.initialize();
  
  Serial.println("Starting");

}

void loop() {
  // if (Serial.available() > 0) {
  //     String data = Serial.readStringUntil('\n');
  //     commands(data);      
  // }
  bool rotate_right = false;
  bool rotate_left = true;

  if (rotate_right) {
    setRotation(-1); //-1 means clockwise
  }
  else if (rotate_left) {
    setRotation(0); //0 means counter-clockwise
  }

  // updateWheelPos(135, 10);
  servoA.goToAngle(targetAngle1);
  servoB.goToAngle(targetAngle2);
  servoC.goToAngle(targetAngle3);
  servoD.goToAngle(targetAngle4);

  Serial.println(encoderA.readAngle());  

}

void updateWheelPos(int ang, float dist) { //might not just end up using distance here
    int steeringTargetAng = ang;
    bool reverseDirection = false;
    
    if(ang >= 0 && ang < 40){
      steeringTargetAng = ang + 180;
      reverseDirection = true;
    }

    targetAngle1 = (steeringTargetAng + MOD_1_OFFSET) % 360;
    targetAngle2 = (steeringTargetAng + MOD_2_OFFSET) % 360;
    targetAngle4 = (steeringTargetAng + MOD_4_OFFSET) % 360;

    if (steeringTargetAng >= 0 && steeringTargetAng < 90) {
      targetAngle3 = map(steeringTargetAng, 0, 90, 60, 140);
    } 
    else if (steeringTargetAng >= 90 && steeringTargetAng < 180) {
      targetAngle3 = map(steeringTargetAng, 90, 180, 140, 220);
    } 
    else if (steeringTargetAng >= 180 && steeringTargetAng < 270) {
      targetAngle3 = map(steeringTargetAng, 180, 270, 220, 300);
    } 
    else { // steeringTargetAng is 270-359 | map 270-360 to 300-420 (which is 300 --> 60)
      int mappedAngle = map(steeringTargetAng, 270, 360, 300, 420);
      targetAngle3 = mappedAngle % 360; // Wraps 420 to 60
    }
    
    if(reverseDirection){
      motorSpd = defaultSpd - inc;
    }

    else{
      motorSpd = defaultSpd + inc;
    }
    
    sparkA.setSpeed(motorSpd);
    sparkB.setSpeed(motorSpd);
    sparkC.setSpeed(motorSpd);
    sparkD.setSpeed(motorSpd);
  }

void setRotation(int setting) {
    int ang1, ang2, ang3, ang4; //target angles for M1(TopLeft), M2(BottomLeft), M3(BottomRight), M4(TopRight)
    
    if (setting == -1) {
        ang1 = 315; ang2 = 45; ang3 = 135; ang4 = 225;
    } else { //counter-clockwise
        ang1 = 135; ang2 = 225; ang3 = 315; ang4 = 45;
    }
    
    targetAngle1 = (ang1 + MOD_1_OFFSET) % 360; //offset for 1,2,4 - 3 will be mapped
    targetAngle2 = (ang2 + MOD_2_OFFSET) % 360;
    targetAngle4 = (ang4 + MOD_4_OFFSET) % 360;
    
    int steeringTarget = ang3; //will be 135 clockwise or 315 counter-clockwise
    
    if (steeringTarget >= 0 && steeringTarget < 90) {
        //map(135, 0, 90, 60, 140) --> 45 equivalent to 100 since halfway
        targetAngle3 = map(steeringTarget, 0, 90, 60, 140);
    } else if (steeringTarget >= 90 && steeringTarget < 180) {
        targetAngle3 = map(steeringTarget, 90, 180, 140, 220);
    } else if (steeringTarget >= 180 && steeringTarget < 270) {
        // map(335, 180, 270, 220, 300) -> 225 evuia260
        targetAngle3 = map(steeringTarget, 180, 270, 220, 300);
    } else {
        targetAngle3 = map(steeringTarget, 270, 360, 300, 420) % 360;
    }
    
    sparkA.setSpeed(defaultSpd + inc);
    sparkB.setSpeed(defaultSpd + inc);
    sparkC.setSpeed(defaultSpd + inc);
    sparkD.setSpeed(defaultSpd + inc);
}
#include <PID_v2.h>
#include "contServoNoPID.h"
#include "encoder.h"
#include "spark.h"

double dist_setpoint = 0.2; //0.2m away from object
double ang_setpoint = 0.0; //center alignment
double strafe_setpoint = 0.0; //center alignment

double Kp_dist = 2.0, Ki_dist = 0.1, Kd_dist = 0.1;
double Kp_ang = 3.0, Ki_ang = 0.2, Kd_ang = 0.1;
double Kp_strafe = 2.5, Ki_strafe = 0.1, Kd_strafe = 0.1;

float camera_dist, camera_ang, camera_strafe;
float fwd_command, str_command, rot_command;

PID distPID(&camera_dist, &fwd_command, &dist_setpoint, Kp_dist, Ki_dist, Kd_dist, PID::P_ON_M, PID::D_ON_M, PID::REVERSE);

PID anglePID(&camera_ang, &rot_command, &angle_setpoint, Kp_ang, Ki_ang, Kd_ang, PID::P_ON_M, PID::D_ON_M, PID::DIRECT);

PID strafePID(&camera_strafe, &str_command, &strafe_setpoint, Kp_strafe, Ki_strafe, Kd_strafe, PID::P_ON_M, PID::D_ON_M, PID::DIRECT);

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
const int MOD_3_OFFSET = 60;
const int MOD_4_OFFSET = 200;

int targetAngle1 = MOD_1_OFFSET; //module1
int targetAngle2 = MOD_2_OFFSET; //module2
int targetAngle3 = MOD_3_OFFSET; //module3 deal with map
int targetAngle4 = MOD_4_OFFSET; //module4


int defaultSpd = 1500;
int motorSpd = defaultSpd;
int inc = 500; 

void setup() {
  Serial.begin(500000); //baud rate

  distancePID.SetOutputLimits(-1.0, 1.0);
  anglePID.SetOutputLimits(-1.0, 1.0);
  strafePID.SetOutputLimits(-1.0, 1.0);
  
  distancePID.SetMode(PID::AUTOMATIC);
  anglePID.SetMode(PID::AUTOMATIC);
  strafePID.SetMode(PID::AUTOMATIC);

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
  bool received_data = false;
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');    
    received_data = true;

    bool detectObj = getCameraData(data);

    if(detectObj) {
      if (camera_ang == 0.0 && camera_dist == 0.0) {
        setSwerve(0.0, 0.0, 0.0);
      } 
      else {
        distancePID.Compute();
        anglePID.Compute();
        strafePID.Compute();
      
        setSwerve(fwd_command, str_command, rot_command);
      }
    }

    else {
      setSwerve(0.0, 0.0, 0.0);
    }
  }

  if(!received_data){
    setSwerve(0.0, 0.0, 0.0);
  }
}

bool getCameraData(String data){ //data comes in format: (angle, distance, strafe)
  int comma1 = data.indexOf(",");
  if (comma1 == -1){
    return false; //No first comma
  }

  int comma2 = data.indexOf(",", comma1 + 1);
  if (comma2 == -1) return false; //No second comma
  
  String ang_str = data.substring(0, comma1);
  String dist_str = data.substring(comma1 + 1, comma2);
  String strafe_str = data.substring(comma2 + 1);

  if (ang_str == "" || dist_str == "" || strafe_str == "") {
    return false;
  }

  camera_ang = ang_str.toFloat();
  camera_dist = dist_str.toFloat();
  camera_strafe = strafe_str.toFloat();
  
  return true;
}

void updateWheelPos(float ang, float dist, float strafe) {
    int steeringTargetAng = ang;
    bool baseReverse = false;
    
    if ((ang >= 0 && ang < 40) || (ang >= 180 && ang < 220)) {
        steeringTargetAng = (ang + 180) % 360;
        baseReverse = true;
    }

    //Module 1 
    int targetAngMod1 = (steeringTargetAng + MOD_1_OFFSET) % 360;
    int currentAngMod1 = encoderA.readAngle();
    int error1 = closestAngle(targetAngMod1, currentAngMod1);
    bool reverse1 = baseReverse;

    //run optimization if not in hardware deadzone
    if (!baseReverse && abs(error1) > 90) {
        targetAngle1 = (targetAngMod1 + 180) % 360;
        reverse1 = true;
    } 
    else {
        //no optimization (in deadzone)
        targetAngle1 = targetAngMod1;
    }

    //Module 2 
    int targetAngMod2 = (steeringTargetAng + MOD_2_OFFSET) % 360;
    int currentAngMod2 = encoderB.readAngle();
    int error2 = closestAngle(targetAngMod2, currentAngMod2);
    bool reverse2 = baseReverse;

    if (!baseReverse && abs(error2) > 90) {
        targetAngle2 = (targetAngMod2 + 180) % 360;
        reverse2 = true;
    } 
    else {
        targetAngle2 = targetAngMod2;
    }

    //Module 3 (mapped)
    int targetAngMod3;
    if (steeringTargetAng >= 0 && steeringTargetAng < 90) {
      targetAngMod3 = map(steeringTargetAng, 0, 90, 60, 140);
    } 
    else if (steeringTargetAng >= 90 && steeringTargetAng < 180) {
      targetAngMod3 = map(steeringTargetAng, 90, 180, 140, 220);
    } 
    else if (steeringTargetAng >= 180 && steeringTargetAng < 270) {
      targetAngMod3 = map(steeringTargetAng, 180, 270, 220, 300);
    } 
    else {
      targetAngMod3 = map(steeringTargetAng, 270, 360, 300, 420) % 360;
    }

    int currentAngMod3 = encoderC.readAngle();
    int error3 = closestAngle(targetAngMod3, currentAngMod3);
    bool reverse3 = baseReverse;

    if (!baseReverse && abs(error3) > 90) {
        targetAngle3 = (targetAngMod3 + 180) % 360;
        reverse3 = true;
    } 
    else {
        targetAngle3 = targetAngMod3;
    }

    //Module 4
    int targetAngMod4 = (steeringTargetAng + MOD_4_OFFSET) % 360;
    int currentAngMod4 = encoderD.readAngle();
    int error4 = closestAngle(targetAngMod4, currentAngMod4);
    bool reverse4 = baseReverse;

    if (!baseReverse && abs(error4) > 90) {
        targetAngle4 = (targetAngMod4 + 180) % 360;
        reverse4 = true;
    } 
    else {
        targetAngle4 = targetAngMod4;
    }

    setSparkSpeed(sparkA, dist, reverse1);
    setSparkSpeed(sparkB, dist, reverse2);
    setSparkSpeed(sparkC, dist, reverse3);
    setSparkSpeed(sparkD, dist, reverse4);
}

void setRotation(int setting) {
    int ang1, ang2, ang3, ang4; //target angles for M1(TopLeft), M2(BottomLeft), M3(BottomRight), M4(TopRight)
    
    if (setting == -1) {
        ang1 = 315; ang2 = 45; ang3 = 135; ang4 = 225;
    } 
    else { //counter-clockwise
        ang1 = 135; ang2 = 225; ang3 = 315; ang4 = 45;
    }
    
    targetAngle1 = (ang1 + MOD_1_OFFSET) % 360; //offset for 1,2,4 - 3 will be mapped
    targetAngle2 = (ang2 + MOD_2_OFFSET) % 360;
    targetAngle4 = (ang4 + MOD_4_OFFSET) % 360;
    
    int steeringTarget = ang3; //will be 135 clockwise or 315 counter-clockwise
    
    if (steeringTarget >= 0 && steeringTarget < 90) {
        //map(135, 0, 90, 60, 140) --> 45 equivalent to 100 since halfway
        targetAngle3 = map(steeringTarget, 0, 90, 60, 140);
    } 
    else if (steeringTarget >= 90 && steeringTarget < 180) {
        targetAngle3 = map(steeringTarget, 90, 180, 140, 220);
    } 
    else if (steeringTarget >= 180 && steeringTarget < 270) {
        // map(335, 180, 270, 220, 300) --> 225 equivalent to 260 since halfway
        targetAngle3 = map(steeringTarget, 180, 270, 220, 300);
    } 
    else {
        targetAngle3 = map(steeringTarget, 270, 360, 300, 420) % 360;
    }
    
    sparkA.setSpeed(defaultSpd + inc);
    sparkB.setSpeed(defaultSpd + inc);
    sparkC.setSpeed(defaultSpd + inc);
    sparkD.setSpeed(defaultSpd + inc);
}

void setSparkSpeed(Spark &spark, float dist, bool reverse){

  int motorSpdOffset = (int)(dist/2 * inc); //need to replace this line because the target distance is going to be 0.2m and inc is 200 so 0.2/2*200 = 20

  if(reverse) {
    spark.setSpeed(defaultSpd - motorSpdOffset);
  }
  else {
    spark.setSpeed(defaultSpd + motorSpdOffset);
  }
}

int closestAngle(int target, int current) {
    return (target - current + 540) % 360 - 180;
}

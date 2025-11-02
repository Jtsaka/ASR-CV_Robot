#ifndef SPARK_H
#define SPARK_H

#include <Servo.h>

class Spark {
  private:
    int sparkPin;
    int defaultSpd;
    Servo spark;

  public:
    Spark(int pin);
    void initialize();
    void setSpeed(int speed);
    void stopMotor();
};

#endif

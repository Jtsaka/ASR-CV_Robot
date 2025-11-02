#include "spark.h"
#include <Servo.h>
#include "Arduino.h"

Spark::Spark(int pin) : sparkPin(pin){
}

void Spark::initialize() {
  spark.attach(sparkPin);
  setSpeed(1500);
}

void Spark::setSpeed(int speed) {
  spark.writeMicroseconds(speed);
}

void Spark::stopMotor() {
  setSpeed(1500);
}

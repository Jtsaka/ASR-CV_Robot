#include <Arduino.h> 
#include "encoder.h"

Encoder::Encoder(int pin) : encoderPin(pin), currentAngle(0){
}

int Encoder::readAngle(){
  int rawValue = analogRead(encoderPin);
  currentAngle = (int)round((rawValue / 1023.0) * 360.0); 
  return currentAngle;
}
#include "UltrasonicSensor.h"

UltrasonicSensor::UltrasonicSensor(int trigPin, int echoPin) {
  _trigPin = trigPin;
  _echoPin = echoPin;
  
  pinMode(_trigPin, OUTPUT);
  pinMode(_echoPin, INPUT);
}

int UltrasonicSensor::getDistance() {
  digitalWrite(_trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(_trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(_trigPin, LOW);
  
  return pulseIn(_echoPin, HIGH, 30000) / 58;
}

int UltrasonicSensor::printDistance() {
  int distance = getDistance();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  return distance;
}
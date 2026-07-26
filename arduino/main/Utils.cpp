#include "Utils.h"
#include <RoboCore_Vespa.h>


void CurveTest(VespaMotors& motors) {
  motors.stop();
  delay(1000);
  motors.turn(100,50);
  delay(3000);
  motors.stop();
  motors.turn(50,100);
  delay(3000);
}

void SonarTest(int obstacleFront, int obstacleLeft, int obstacleRight) {
  Serial.print("Sonar -> Front: ");
  Serial.print(obstacleFront);
  Serial.print(" cm | Left: ");
  Serial.print(obstacleLeft);
  Serial.print(" cm | Right: ");
  Serial.print(obstacleRight);
  Serial.println(" cm");
}
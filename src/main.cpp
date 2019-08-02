#include <Arduino.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
//#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

#include "ParameterProvider.h"
#include "HttpServer.h"
#include "StepperManager.h"

// The port to listen for incoming TCP connections
#define LISTEN_PORT           80

// Acceleration and deceleration values are always in FULL steps / s^2
#define MOTOR_ACCEL 2000
#define MOTOR_DECEL 1000

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 64
#define RPM 937

// Since microstepping is set externally, make sure this matches the selected mode
// If it doesn't, the motor will move at a different RPM than chosen
// 1=full step, 2=half step etc.
#define MICROSTEPS 1

// All the wires needed for full functionality
#define DIR   D1
#define STEP  D2

//enable/disable functionality
#define SLEEP D3

#define MS1 10
#define MS2 11
#define MS3 12

ParameterProvider _paramsProvider;
StepperManager _stepperManager(_paramsProvider);
HttpServer _httpServer(_paramsProvider, _stepperManager);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset saved settings
  //wifiManager.resetSettings();
  
  //TODO : add to system parameters
  wifiManager.autoConnect("stepperMotor_AP");
  
  //if you get here you have connected to the WiFi
  Serial.println("connected...");

  ArduinoOTA.setHostname("wemos_stepperMotor"); 
  ArduinoOTA.begin();

  _paramsProvider.setup();
  Serial.println("_paramProvider.params().motorStepNumber: " + String(_paramsProvider.params().motorStepNumber));
  _stepperManager.setup();
  _httpServer.setup();
  // pinMode(DIR, OUTPUT);
  // pinMode(STEP, OUTPUT);
  // pinMode(SLEEP, OUTPUT);
  // digitalWrite(DIR, 1);
  // digitalWrite(SLEEP, 0);

}

void loop() {
  
  //Serial.println("loop...");
  ArduinoOTA.handle();
  _stepperManager.handle();
  _httpServer.handle();
  delay(100);
  // stepper.move(-MOTOR_STEPS);
  // delay(1000);

  // int minTime = 700;
  // int startAccel = 2000;
  // int endAccel = minTime;
  // Serial.println("accel...");
  // for (int i = startAccel; i  > endAccel; i -= 10)
  // {
  //   digitalWrite(STEP, 1);
  //   delayMicroseconds(i);
  //   digitalWrite(STEP, 0);
  //   delayMicroseconds(i);
  //   yield();
  // }
  // Serial.println("cste...");
  // for (int i = 0; i < 20000; ++i)
  // {
  //   digitalWrite(STEP, 1);
  //   delayMicroseconds(minTime);
  //   digitalWrite(STEP, 0);
  //   delayMicroseconds(minTime);
  //   yield();
  // }
}

//http://www.jangeox.be/2013/10/change-unipolar-28byj-48-to-bipolar.html


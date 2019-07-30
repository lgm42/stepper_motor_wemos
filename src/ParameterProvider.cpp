#include <ArduinoJson.h>
#include <FS.h>

#include "ParameterProvider.h"

const String ParameterProvider::Filename = "parameters.json";
const String ParameterProvider::SystemFilename = "system.json";

ParameterProvider::ParameterProvider()
{}

ParameterProvider::~ParameterProvider()
{}

void ParameterProvider::setup()
{
  SPIFFS.begin(); 
}

void ParameterProvider::loadSystemParameters()
{
  // Open file for reading
  File file = SPIFFS.open(SystemFilename, "r");

  DynamicJsonDocument doc(1024);

  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    createDefaultSystemValues();
    return;
  }

  _currentSystemParameters.hostname = doc["hostname"] | "stepperMotor";
  _currentSystemParameters.ftpLogin = doc["ftpLogin"] | "stepper";
  _currentSystemParameters.ftpPassword = doc["ftpPassword"] | "stepper";

  // Close the file (File's destructor doesn't close the file)
  file.close();
}

void ParameterProvider::saveSystemParameters()
{
  // Delete existing file, otherwise the configuration is appended to the file
  SPIFFS.remove(SystemFilename);

  // Open file for writing
  File file = SPIFFS.open(SystemFilename, "w");
  if (!file) {
      Serial.println(F("Failed to create file"));
      return;
  }

  DynamicJsonDocument doc(1024);
  deserializeJson(doc, file);
  doc["hostname"] = _currentSystemParameters.hostname;
  doc["ftpLogin"] = _currentSystemParameters.ftpLogin;
  doc["ftpPassword"] = _currentSystemParameters.ftpPassword;

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
      Serial.println(F("Failed to write to file"));
  }

  // Close the file (File's destructor doesn't close the file)
  file.close();
}

void ParameterProvider::createDefaultSystemValues()
{
  _currentSystemParameters.hostname = "stepperMotor";
  _currentSystemParameters.hostname = "stepper";
  _currentSystemParameters.hostname = "stepper";
}

void ParameterProvider::load()
{
  // Open file for reading
  File file = SPIFFS.open(Filename, "r");

  DynamicJsonDocument doc(1024);

  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    createDefaultValues();
    return;
  }

  _currentParameters.counterClockWizeAngleAmplitude = doc["counterClockWizeAngleAmplitude"] | 90;
  _currentParameters.clockWizeAngleAmplitude = doc["clockWizeAngleAmplitude"] | 90;
  _currentParameters.currentPosition = doc["currentPosition"] | 0;
  _currentParameters.reductionRate = doc["reductionRate"] | 64;
  _currentParameters.motorStepNumber = doc["motorStepNumber"] | 64;

  // Close the file (File's destructor doesn't close the file)
  file.close();
}

void ParameterProvider::save()
{
  // Delete existing file, otherwise the configuration is appended to the file
  SPIFFS.remove(Filename);

  // Open file for writing
  File file = SPIFFS.open(Filename, "w");
  if (!file) {
      Serial.println(F("Failed to create file"));
      return;
  }

  DynamicJsonDocument doc(1024);
  deserializeJson(doc, file);
  doc["counterClockWizeAngleAmplitude"] = _currentParameters.counterClockWizeAngleAmplitude;
  doc["clockWizeAngleAmplitude"] = _currentParameters.clockWizeAngleAmplitude;
  doc["currentPosition"] = _currentParameters.currentPosition;
  doc["reductionRate"] = _currentParameters.reductionRate;
  doc["motorStepNumber"] = _currentParameters.motorStepNumber;

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
      Serial.println(F("Failed to write to file"));
  }

  // Close the file (File's destructor doesn't close the file)
  file.close();
}

void ParameterProvider::createDefaultValues()
{
  _currentParameters.counterClockWizeAngleAmplitude = 90;
  _currentParameters.clockWizeAngleAmplitude = 90;
  _currentParameters.currentPosition = 0;
  _currentParameters.reductionRate = 64;
  _currentParameters.motorStepNumber = 64;
}

ParameterProvider::Parameters & ParameterProvider::params()
{
  return _currentParameters;
}

const ParameterProvider::Parameters & ParameterProvider::params() const 
{
  return _currentParameters;
}

ParameterProvider::SystemParameters & ParameterProvider::systemParams()
{
  return _currentSystemParameters;
}

const ParameterProvider::SystemParameters & ParameterProvider::systemParams() const 
{
  return _currentSystemParameters;
}
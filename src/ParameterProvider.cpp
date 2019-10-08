#include <ArduinoJson.h>
#include <FS.h>

#include "ParameterProvider.h"

const String ParameterProvider::Filename = "/parameters.json";
const String ParameterProvider::PositionFilename = "/position.txt";
const String ParameterProvider::SystemFilename = "/system.json";

ParameterProvider::ParameterProvider()
{}

ParameterProvider::ParameterProvider(const ParameterProvider & copy)
{}

ParameterProvider::~ParameterProvider()
{}

void ParameterProvider::setup()
{
  if (!SPIFFS.begin()) 
  {
    Serial.println("Unable to begin SPIFFS");
  }
  loadSystemParameters();
  //TODO : remove when ok
  createDefaultValues();
  save();

  load();

  loadPosition();
}

void ParameterProvider::loadSystemParameters()
{
  // Open file for reading
  File file = SPIFFS.open(SystemFilename, "r");
  if (!file)
  {
    Serial.println("Exception during opening system configuration, resetting to factory settings");
    createDefaultSystemValues();
    saveSystemParameters();
    return;
  }

  DynamicJsonDocument doc(1024);

  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    Serial.println("Exception during deserializing system configuration, resetting to factory settings");
    createDefaultSystemValues();
    saveSystemParameters();
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
  // Open file for writing
  File file = SPIFFS.open(SystemFilename, "w");
  if (!file) {
      Serial.println(F("Failed to create file"));
      return;
  }

  DynamicJsonDocument doc(1024);
  //deserializeJson(doc, file);
  doc["hostname"] = _currentSystemParameters.hostname;
  doc["ftpLogin"] = _currentSystemParameters.ftpLogin;
  doc["ftpPassword"] = _currentSystemParameters.ftpPassword;

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) 
  {
      Serial.println(F("Failed to write to file"));
  }

  // Close the file (File's destructor doesn't close the file)
  file.close();
}

void ParameterProvider::createDefaultSystemValues()
{
  _currentSystemParameters.hostname = "stepperMotor";
  _currentSystemParameters.ftpLogin = "stepper";
  _currentSystemParameters.ftpPassword = "stepper";
}

void ParameterProvider::load()
{
  // Open file for reading
  File file = SPIFFS.open(Filename, "r");
  if (!file)
  {
    Serial.println("Exception during opening configuration file, resetting to factory settings");
    createDefaultValues();
    save();
    return;
  }

  DynamicJsonDocument doc(1024);

  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    Serial.println("Exception during deserializing configuration, resetting to factory settings");
    createDefaultValues();
    save();
    return;
  }

  _currentParameters.counterClockWizeAngleAmplitude = doc["counterClockWizeAngleAmplitude"] | 90.0;
  _currentParameters.clockWizeAngleAmplitude = doc["clockWizeAngleAmplitude"] | 90.0;
  _currentParameters.reductionRate = doc["reductionRate"] | 384.0;
  _currentParameters.motorStepNumber = doc["motorStepNumber"] | 64.0;
  _currentParameters.originAngle = doc["originAngle"] | 0.0;

  // Close the file (File's destructor doesn't close the file)
  file.close();
  
}

void ParameterProvider::save()
{
  // Open file for writing
  File file = SPIFFS.open(Filename, "w");
  if (!file) {
      Serial.println(F("Failed to create file"));
      return;
  }

  DynamicJsonDocument doc(1024);
  //deserializeJson(doc, file);
  doc["counterClockWizeAngleAmplitude"] = _currentParameters.counterClockWizeAngleAmplitude;
  doc["clockWizeAngleAmplitude"] = _currentParameters.clockWizeAngleAmplitude;
  doc["reductionRate"] = _currentParameters.reductionRate;
  doc["motorStepNumber"] = _currentParameters.motorStepNumber;
  doc["originAngle"] = _currentParameters.originAngle;

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
      Serial.println(F("Failed to write to file"));
  }

  // Close the file (File's destructor doesn't close the file)
  file.close();
}

void ParameterProvider::createDefaultValues()
{
  _currentParameters.counterClockWizeAngleAmplitude = 90.0;
  _currentParameters.clockWizeAngleAmplitude = 90.0;
  _currentParameters.reductionRate = 165.0;
  _currentParameters.motorStepNumber = 64.0;
  _currentParameters.originAngle = 0.0;
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

void ParameterProvider::loadPosition()
{
  File file = SPIFFS.open(PositionFilename, "r");
  if (!file)
  {
    Serial.println("Exception during opening position file, resetting to factory settings");
    _currentPosition = 0.0;
    savePosition();
    return;
  }
  String fileContent;

  while (file.available())
  {
    fileContent += char(file.read());
  }
  file.close();
  _currentPosition = fileContent.toFloat();
}

void ParameterProvider::savePosition()
{
  File file = SPIFFS.open(PositionFilename, "w");
  if (!file)
  {
    Serial.println("Exception during opening position file");
    return;
  }
  file.printf("%2.3f", _currentPosition);
  file.close();
}

void ParameterProvider::position(const double angle)
{
  _currentPosition = angle;
}

double ParameterProvider::position() const
{
  return _currentPosition;
}

String ParameterProvider::toJson() const
{
	String response = "{\"hostname\":\"" + _currentSystemParameters.hostname + "\", 					\
	\"ftp-login\":\"" + _currentSystemParameters.ftpLogin + "\",					\
	\"ftp-password\":\"" + _currentSystemParameters.ftpPassword + "\",					\
	\"positive-angle-amplitude\":\"" + _currentParameters.clockWizeAngleAmplitude + "\",					\
	\"negative-angle-amplitude\":\"" + _currentParameters.counterClockWizeAngleAmplitude + "\",					\
	\"motor-step-number\":\"" + _currentParameters.motorStepNumber + "\",					\
	\"origin-angle\":\"" + _currentParameters.originAngle + "\",					\
	\"reduction-rate\":\"" + _currentParameters.reductionRate + "\"}";
	return response;
}
#pragma once

#include <Arduino.h>

class ParameterProvider
{
  public:

    typedef struct
    {
        int counterClockWizeAngleAmplitude;
        int clockWizeAngleAmplitude;
        int currentPosition;
        double reductionRate;
        double motorStepNumber;
    } Parameters;

    typedef struct 
    {
        String hostname;
        String ftpLogin;
        String ftpPassword;
    } SystemParameters;

    ParameterProvider();
    virtual ~ParameterProvider();

    void setup();

    void loadSystemParameters();

    void saveSystemParameters();
    void createDefaultSystemValues();

    void load();
    void save();

    void createDefaultValues();
    Parameters & params();

    const Parameters & params() const ;
    SystemParameters & systemParams();
    const SystemParameters & systemParams() const ;

  private:
    Parameters _currentParameters;
    SystemParameters _currentSystemParameters;
    static const String Filename;
    static const String SystemFilename;
};
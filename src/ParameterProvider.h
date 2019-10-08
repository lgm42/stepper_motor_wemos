#pragma once

#include <Arduino.h>

class ParameterProvider
{
  public:

    struct Parameters
    {
        double counterClockWizeAngleAmplitude;
        double clockWizeAngleAmplitude;
        double reductionRate;
        double motorStepNumber;
        double originAngle;
    };

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

    void loadPosition();
    void savePosition();

    void createDefaultValues();
    Parameters & params();

    const Parameters & params() const ;
    SystemParameters & systemParams();
    const SystemParameters & systemParams() const ;

    void position(const double angle);
    double position() const;

    String toJson() const;

  private:
    ParameterProvider(const ParameterProvider & copy);

    Parameters _currentParameters;
    SystemParameters _currentSystemParameters;
    double _currentPosition;
    static const String Filename;
    static const String PositionFilename;
    static const String SystemFilename;
};
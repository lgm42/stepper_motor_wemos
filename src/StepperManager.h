#pragma once

#include "Arduino.h"
#include "stdint.h"

#include "ParameterProvider.h"

class StepperManager
{
    public:
        StepperManager(ParameterProvider & paramsProvider);
        virtual ~StepperManager();

        virtual void setup(void);
	    virtual void handle(void);

        bool moving() const;

        void startRotate(const double angle);
        
        static StepperManager *Instance;

        void onTimer();

        double position() const;
    private:
        typedef enum 
        {
            kClockWize = 0,
            kCounterClockWize = 1
        } Direction;

        ParameterProvider _paramProvider;
        Direction _direction;
        int _stepsToObjective; 
        double _currentAngle;

        void setDirection();
        int stepsToAngle(const int steps);

        static const uint8_t DirectionPin;
        static const uint8_t StepPin;
        static const uint8_t EnablePin;
        static const uint8_t Ms1Pin;
        static const uint8_t Ms2Pin;
        static const uint8_t Ms3Pin;
};
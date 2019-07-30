
#include "StepperManager.h"

const uint8_t StepperManager::DirectionPin = D1;
const uint8_t StepperManager::StepPin = D2;
const uint8_t StepperManager::EnablePin = D3;

const uint8_t StepperManager::Ms1Pin = D4;
const uint8_t StepperManager::Ms2Pin = D5;
const uint8_t StepperManager::Ms3Pin = D6;

void ICACHE_RAM_ATTR onTimerISR();

StepperManager *StepperManager::Instance = NULL;

StepperManager::StepperManager(ParameterProvider & paramProvider)
	: _paramProvider(paramProvider), _currentAngle(0.0)
{
}

StepperManager::~StepperManager()
{
}

void StepperManager::setup()
{
    pinMode(DirectionPin, OUTPUT);
    pinMode(StepPin, OUTPUT);
    pinMode(EnablePin, OUTPUT);
    digitalWrite(DirectionPin, 1);
    digitalWrite(EnablePin, 0);

    Instance = this;
    timer1_isr_init();
    timer1_attachInterrupt(onTimerISR);
    timer1_enable(TIM_DIV1, TIM_EDGE, TIM_LOOP);
    timer1_write(80000); //1ms
}

void StepperManager::handle()
{
    //TODO : save json if moving ?
}

void ICACHE_RAM_ATTR onTimerISR()
{
    StepperManager::Instance->onTimer();
}

void StepperManager::onTimer()
{
    if (_stepsToObjective > 0)
    {
        if (digitalRead(StepPin) == 0)
        {
            digitalWrite(DirectionPin, 1);
        }
        else
        {
            digitalWrite(DirectionPin, 0);
            //we reduce number of step by 1
            _stepsToObjective--;
            _currentAngle += stepsToAngle(1);
        }
    }

    timer1_write(80000); //1ms
}

int StepperManager::stepsToAngle(const int steps)
{
    return (steps * 360.0) / (_paramProvider.params().motorStepNumber * _paramProvider.params().reductionRate);
}

bool StepperManager::moving() const
{
    return _stepsToObjective != 0;
}

void StepperManager::startRotate(const double angle)
{
    //we compute the number of step to do
    double delta = angle - _currentAngle;
    _direction = (delta > 0?kClockWize:kCounterClockWize);
    setDirection();
    _stepsToObjective = delta * (_paramProvider.params().motorStepNumber * _paramProvider.params().reductionRate) / 360.0;
}

void StepperManager::setDirection()
{
    switch (_direction)
    {
        default:
        case kClockWize:
            digitalWrite(DirectionPin, 1);
            break;
        case kCounterClockWize:
            digitalWrite(DirectionPin, 0);
            break;
    }
}

double StepperManager::position() const
{
    return _currentAngle;
}
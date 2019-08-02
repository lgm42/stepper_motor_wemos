
#include "StepperManager.h"

const uint8_t StepperManager::DirectionPin = D1;
const uint8_t StepperManager::StepPin = D2;
const uint8_t StepperManager::EnablePin = D3;

const uint8_t StepperManager::Ms1Pin = D4;
const uint8_t StepperManager::Ms2Pin = D5;
const uint8_t StepperManager::Ms3Pin = D6;

void ICACHE_RAM_ATTR onTimerISR();

StepperManager *StepperManager::Instance = NULL;

StepperManager::StepperManager(ParameterProvider & paramsProvider)
	: _paramsProvider(paramsProvider)
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
    digitalWrite(EnablePin, 1);

    Instance = this;
    timer1_isr_init();
    timer1_attachInterrupt(onTimerISR);
    timer1_enable(TIM_DIV1, TIM_EDGE, TIM_SINGLE);
    timer1_write(80000); //1ms
}

void StepperManager::handle()
{
    //TODO : save json if moving ?
    Serial.println("progress angle: " + String(_paramsProvider.position()));
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
            digitalWrite(StepPin, 1);
        }
        else
        {
            digitalWrite(StepPin, 0);
            //we reduce number of step by 1
            _stepsToObjective--;
            _paramsProvider.position(_paramsProvider.position() + stepsToAngle(1));
            if (_stepsToObjective == 0)
            {
                digitalWrite(EnablePin, 1);
                Serial.println("finish, angle: " + String(_paramsProvider.position()));
                _paramsProvider.savePosition();
            }
                
        }
    }

    timer1_write(80000); //1ms
}

double StepperManager::stepsToAngle(const int steps)
{
    return (steps * 360.0) / (_paramsProvider.params().motorStepNumber * _paramsProvider.params().reductionRate);
}

bool StepperManager::moving() const
{
    return _stepsToObjective != 0;
}

void StepperManager::startRotateAbsolute(const double angle)
{
    //we compute the number of step to do
    double delta = angle - _paramsProvider.position();
    _direction = (delta > 0?kClockWize:kCounterClockWize);
    setDirection();
    digitalWrite(EnablePin, 0);
    _stepsToObjective = abs(delta) * (_paramsProvider.params().motorStepNumber * _paramsProvider.params().reductionRate) / 360.0;
    Serial.println("startRotate, absolute angle: " + String(angle) + ", steps: " + String(_stepsToObjective));
}

void StepperManager::startRotateRelative(const double angle)
{
    //we compute the number of step to do
    _direction = (angle > 0?kClockWize:kCounterClockWize);
    setDirection();
    digitalWrite(EnablePin, 0);
    _stepsToObjective = abs(angle) * (_paramsProvider.params().motorStepNumber * _paramsProvider.params().reductionRate) / 360.0;
    Serial.println("startRotate, relative angle: " + String(angle) + ", steps: " + String(_stepsToObjective));
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
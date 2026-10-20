#ifndef FLOWSENSOR_H
#define FLOWSENSOR_H
#include <Arduino.h>
#include <functional>

// Digmesa Nano Brass 9NB-0120-01A (1.2 mm nozzle, no pulse divider):
// datasheet ~31'100 pulses/L => ~0.03215 ml/pulse. Override via setMlPerPulse().
constexpr float DIGMESA_NB_0120_ML_PER_PULSE = 1.0f / 31.1f;
constexpr float ML_PER_PULSE = DIGMESA_NB_0120_ML_PER_PULSE;

using flow_amount_callback_t = std::function<void(float)>;

class FlowSensor {
  public:
    FlowSensor(uint8_t pin, flow_amount_callback_t callback = nullptr);
    ~FlowSensor() = default;

    void setup();
    void loop();
    void tare();

    static void IRAM_ATTR onInterrupt();

    float getVolume() const { return static_cast<float>(_ticks) * _mlPerPulse; }
    float getFlow() const { return _currentFlow; }
    float *getFlowPtr() { return &_currentFlow; }
    void setMlPerPulse(float mlPerPulse) { _mlPerPulse = mlPerPulse; }
    float getMlPerPulse() const { return _mlPerPulse; }

  private:
    void updateValue(int ticks);
    void addPulse(uint32_t nowUs);

    uint8_t _pin;
    flow_amount_callback_t _callback;
    volatile int _ticks = 0;
    volatile uint32_t _lastPulseUs = 0;
    volatile uint32_t _lastPeriodUs = 0;
    xTaskHandle taskHandle = nullptr;
    float _currentFlow = 0.0f;
    float _mlPerPulse = ML_PER_PULSE;
    // No pulse for this long => report 0 ml/s.
    static constexpr uint32_t ZERO_FLOW_TIMEOUT_US = 400000;

    const char *LOG_TAG = "FlowSensor";
    static void loopTask(void *arg);

    static FlowSensor *_instance;
};

#endif // FLOWSENSOR_H

#include "FlowSensor.h"

FlowSensor *FlowSensor::_instance = nullptr;

void IRAM_ATTR FlowSensor::onInterrupt() {
    if (_instance == nullptr) {
        return;
    }
    const uint32_t now = micros();
    _instance->addPulse(now);
}

FlowSensor::FlowSensor(uint8_t pin, flow_amount_callback_t callback) : _pin(pin), _callback(std::move(callback)) {
    _instance = this;
}

void FlowSensor::setup() {
    pinMode(_pin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(_pin), FlowSensor::onInterrupt, FALLING);
    xTaskCreate(loopTask, "FlowSensor::loop", configMINIMAL_STACK_SIZE * 4, this, 1, &taskHandle);
    ESP_LOGI(LOG_TAG, "FlowSensor on GPIO %u, ml/pulse=%.5f", _pin, _mlPerPulse);
}

void FlowSensor::addPulse(uint32_t nowUs) {
    if (_lastPulseUs != 0) {
        _lastPeriodUs = nowUs - _lastPulseUs;
    }
    _lastPulseUs = nowUs;
    _ticks++;
}

void FlowSensor::loop() {
    noInterrupts();
    const int ticks = _ticks;
    const uint32_t lastPulseUs = _lastPulseUs;
    const uint32_t lastPeriodUs = _lastPeriodUs;
    interrupts();

    const uint32_t now = micros();
    float flow = 0.0f;
    if (lastPulseUs != 0 && (now - lastPulseUs) < ZERO_FLOW_TIMEOUT_US && lastPeriodUs > 0) {
        // flow = ml/pulse / period_s
        flow = _mlPerPulse / (static_cast<float>(lastPeriodUs) * 1e-6f);
    }
    // Light EMA for display/control stability without hiding zero-flow timeout.
    if (flow <= 0.0f) {
        _currentFlow = 0.0f;
    } else {
        _currentFlow = flow * 0.35f + _currentFlow * 0.65f;
    }

    if (_callback) {
        _callback(static_cast<float>(ticks) * _mlPerPulse);
    }

    ESP_LOGV(LOG_TAG, "ticks=%d period_us=%u flow=%.3f", ticks, lastPeriodUs, _currentFlow);
}

void FlowSensor::tare() { updateValue(0); }

void FlowSensor::updateValue(int ticks) {
    noInterrupts();
    _ticks = ticks;
    _lastPulseUs = 0;
    _lastPeriodUs = 0;
    interrupts();
    _currentFlow = 0.0f;
}

void FlowSensor::loopTask(void *arg) {
    auto *sensor = static_cast<FlowSensor *>(arg);
    TickType_t lastWake = xTaskGetTickCount();
    while (true) {
        sensor->loop();
        xTaskDelayUntil(&lastWake, pdMS_TO_TICKS(50));
    }
}

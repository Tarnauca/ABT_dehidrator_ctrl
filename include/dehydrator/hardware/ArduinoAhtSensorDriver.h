#pragma once

#include <Adafruit_AHTX0.h>
#include <stdint.h>

#include "dehydrator/interfaces/AhtSensorDriver.h"

namespace dehydrator {

/**
 * @brief Arduino AHT20/AHT21 library adapter for the project AHT driver
 * interface.
 *
 * The adapter uses the proven `Adafruit_AHTX0` library on the shared I2C bus.
 * It keeps library calls out of the higher-level reader and converts floating
 * point library values into the fixed-point centi-units used by the project.
 */
class ArduinoAhtSensorDriver final : public AhtSensorDriver {
 public:
  /**
   * @brief Initializes the underlying AHT library on the default I2C bus.
   *
   * @return true when the sensor responds and future reads may succeed.
   */
  bool begin() { return initialized_ = aht_.begin(); }

  /**
   * @brief Reports whether the sensor initialized successfully.
   *
   * @return true when `begin()` succeeded.
   */
  bool initialized() const { return initialized_; }

  /**
   * @brief Reads one raw temperature/RH sample from the AHT sensor.
   *
   * @return Raw fixed-point sample with validity flag.
   */
  AhtRawSample readSample() override {
    AhtRawSample sample;
    if (!initialized_) {
      return sample;
    }

    sensors_event_t humidityEvent;
    sensors_event_t temperatureEvent;
    aht_.getEvent(&humidityEvent, &temperatureEvent);

    sample.tempCentiC = toCenti(temperatureEvent.temperature);
    sample.rhCentiPercent = toUnsignedCenti(humidityEvent.relative_humidity);
    sample.valid = true;
    return sample;
  }

 private:
  static int16_t toCenti(float value) {
    const float scaled = value * 100.0f;
    if (scaled >= 0.0f) {
      return static_cast<int16_t>(scaled + 0.5f);
    }

    return static_cast<int16_t>(scaled - 0.5f);
  }

  static uint16_t toUnsignedCenti(float value) {
    if (value <= 0.0f) {
      return 0U;
    }

    return static_cast<uint16_t>((value * 100.0f) + 0.5f);
  }

  Adafruit_AHTX0 aht_;
  bool initialized_ = false;
};

}  // namespace dehydrator

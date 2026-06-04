#pragma once

#include <DHT.h>
#include <stdint.h>

#include "dehydrator/interfaces/TempRhSensorDriver.h"

namespace dehydrator {

/**
 * @brief Arduino DHT22/AM2302 library adapter for the project temp/RH driver
 * interface.
 *
 * The adapter uses the proven Adafruit `DHT` library and converts floating
 * point values into the fixed-point centi-units used by the project.
 */
class ArduinoDhtSensorDriver final : public TempRhSensorDriver {
 public:
  /** Placeholder pin value used when the sensor wiring is not assigned yet. */
  static constexpr uint8_t UNASSIGNED_PIN = 255U;

  /**
   * @brief Creates a DHT22/AM2302 adapter for one MCU pin.
   *
   * @param pin MCU pin connected to the sensor data line.
   */
  explicit ArduinoDhtSensorDriver(uint8_t pin) : pin_(pin), dht_(pin, DHT22) {}

  /**
   * @brief Initializes the underlying DHT library.
   *
   * @return true when the pin is assigned and future reads may succeed.
   */
  bool begin() {
    if (pin_ == UNASSIGNED_PIN) {
      initialized_ = false;
      return false;
    }

    dht_.begin();
    initialized_ = true;
    return true;
  }

  /**
   * @brief Reports whether initialization was attempted on an assigned pin.
   *
   * @return true when `begin()` accepted the current pin.
   */
  bool initialized() const { return initialized_; }

  /**
   * @brief Reads one raw temperature/RH sample from the DHT22 sensor.
   *
   * @return Raw fixed-point sample with validity flag.
   */
  TempRhRawSample readSample() override {
    TempRhRawSample sample;
    if (!initialized_) {
      return sample;
    }

    const float humidity = dht_.readHumidity();
    const float temperature = dht_.readTemperature();
    if (isnan(humidity) || isnan(temperature)) {
      return sample;
    }

    sample.tempCentiC = toCenti(temperature);
    sample.rhCentiPercent = toUnsignedCenti(humidity);
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

  uint8_t pin_;
  DHT dht_;
  bool initialized_ = false;
};

}  // namespace dehydrator

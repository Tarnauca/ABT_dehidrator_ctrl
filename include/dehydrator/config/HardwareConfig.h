#pragma once

#include <stdint.h>

namespace dehydrator {
namespace config {

/**
 * @brief Logical relay activation polarity.
 *
 * Relay boards may energize on either a high or low MCU pin level. Application
 * logic must use logical ON/OFF commands; only hardware adapters should apply
 * this polarity.
 */
enum class ActiveLevel {
  /** The physical output is active when the MCU pin is driven low. */
  ActiveLow,
  /** The physical output is active when the MCU pin is driven high. */
  ActiveHigh,
};

/**
 * @brief Pin placeholders for the Arduino Mega2560 hardware wiring.
 *
 * Values marked TBD are intentionally placeholders. They centralize wiring
 * decisions before the real hardware adapter implementation begins.
 */
struct HardwarePins {
  /** Built-in LED used by the current scheduler shell. */
  uint8_t statusLed;
  /** Analog input for the PT50 voltage divider. */
  uint8_t pt50Analog;
  /** Heater relay output pin placeholder. */
  uint8_t heaterRelay;
  /** Fan relay output pin placeholder. */
  uint8_t fanRelay;
  /** LCD backlight FET output pin placeholder. */
  uint8_t lcdBacklight;
  /** Piezo buzzer output pin placeholder. */
  uint8_t buzzer;
  /** Rotary encoder A signal pin placeholder. */
  uint8_t encoderA;
  /** Rotary encoder B signal pin placeholder. */
  uint8_t encoderB;
  /** Rotary encoder pushbutton pin placeholder. */
  uint8_t encoderButton;
};

/**
 * @brief Hardware-level configuration placeholders.
 */
struct HardwareConfig {
  /** Pin assignment placeholders for the Mega2560 target. */
  HardwarePins pins;
  /** Heater relay active polarity. */
  ActiveLevel heaterRelayActiveLevel;
  /** Fan relay active polarity. */
  ActiveLevel fanRelayActiveLevel;
  /** LCD backlight FET active polarity. */
  ActiveLevel lcdBacklightActiveLevel;
  /** Piezo buzzer active polarity. */
  ActiveLevel buzzerActiveLevel;
  /** LCD I2C address placeholder. */
  uint8_t lcdI2cAddress;
};

/**
 * @brief Default hardware placeholders for early firmware bring-up.
 *
 * Placeholder values must be reviewed before real heater/fan hardware is
 * connected. Heater and fan relay pins remain unassigned for safe bench
 * bring-up; LCD, encoder, buzzer, and backlight use common temporary pins.
 */
constexpr HardwareConfig HARDWARE = {
    {
        13U,   // statusLed
        0U,    // pt50Analog: analog channel placeholder
        255U,  // heaterRelay: TBD
        255U,  // fanRelay: TBD
        7U,    // lcdBacklight: temporary bench pin
        8U,    // buzzer: temporary bench pin
        2U,    // encoderA: temporary interrupt-capable bench pin
        3U,    // encoderB: temporary interrupt-capable bench pin
        4U,    // encoderButton: temporary bench pin with INPUT_PULLUP
    },
    ActiveLevel::ActiveLow,
    ActiveLevel::ActiveLow,
    ActiveLevel::ActiveHigh,
    ActiveLevel::ActiveHigh,
    0x27U,
};

}  // namespace config
}  // namespace dehydrator

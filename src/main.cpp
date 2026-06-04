#include <Arduino.h>
#include <Bounce2.h>
#include <DHT.h>
#include <Encoder.h>
#include <LiquidCrystal_I2C.h>
#include <string.h>

#include "dehydrator/app/EncoderStepFilter.h"
#include "dehydrator/domain/ControlStateMachine.h"
#include "dehydrator/app/PresetRunController.h"
#include "dehydrator/app/PeriodicTask.h"
#include "dehydrator/config/HardwareConfig.h"
#include "dehydrator/config/RuntimeConfig.h"
#include "dehydrator/domain/NtcSensorModel.h"
#include "dehydrator/hardware/RelayOutputs.h"
#include "dehydrator/hardware/ArduinoAnalogInput.h"
#include "dehydrator/hardware/ArduinoDhtSensorDriver.h"
#include "dehydrator/interfaces/DigitalOutput.h"
#include "dehydrator/interfaces/CharacterDisplay.h"
#include "dehydrator/logging/LogDispatcher.h"
#include "dehydrator/logging/LogFormatter.h"
#include "dehydrator/logging/LogSink.h"
#include "dehydrator/sensors/TempRhReader.h"
#include "dehydrator/sensors/NtcReader.h"
#include "dehydrator/presets/PresetCatalog.h"
#include "dehydrator/ui/LcdManualView.h"
#include "dehydrator/ui/LcdPresetView.h"
#include "dehydrator/ui/LcdMenuView.h"
#include "dehydrator/ui/LcdStatusView.h"
#include "dehydrator/ui/ManualModeController.h"
#include "dehydrator/ui/PresetSelectController.h"
#include "dehydrator/ui/MenuController.h"

constexpr char LOG_TRUNCATED_EVENT[] = "WARN code=log_truncated source=event";
constexpr char LOG_TRUNCATED_STATE[] = "WARN code=log_truncated source=state";

dehydrator::PeriodicTask ledTask(
    dehydrator::config::SCHEDULER.statusLedIntervalMs);
dehydrator::PeriodicTask stateLogTask(
    dehydrator::config::SCHEDULER.stateLogIntervalMs);
dehydrator::PeriodicTask sensorSampleTask(
    dehydrator::config::SCHEDULER.sensorSampleIntervalMs);
dehydrator::PeriodicTask tempRhSampleTask(
    dehydrator::config::SCHEDULER.tempRhSampleIntervalMs);
dehydrator::PeriodicTask lcdRefreshTask(
    dehydrator::config::SCHEDULER.lcdRefreshIntervalMs);
dehydrator::PeriodicTask inputScanTask(
    dehydrator::config::SCHEDULER.inputScanIntervalMs);
dehydrator::PeriodicTask runControlTask(
    dehydrator::config::SCHEDULER.sensorSampleIntervalMs);

bool ledOn = false;
bool heartbeatOn = false;
long lastEncoderPosition = 0L;
uint32_t buttonPressedAtMs = 0UL;
bool buttonPressed = false;
dehydrator::NtcReading latestNtc;
dehydrator::TempRhReading latestTempRh;
dehydrator::EncoderStepFilter encoderStepFilter(4);
dehydrator::ControlStateMachine controlStateMachine;
dehydrator::PresetRunController presetRunController;
dehydrator::OutputCommand activeOutputCommand;

enum class BringupScreen {
  Status,
  Menu,
  Manual,
  Preset,
};

BringupScreen currentScreen = BringupScreen::Status;

LiquidCrystal_I2C lcd(dehydrator::config::HARDWARE.lcdI2cAddress,
                      dehydrator::LcdStatusView::COLUMNS,
                      dehydrator::LcdStatusView::ROWS);
Encoder rotaryEncoder(dehydrator::config::HARDWARE.pins.encoderA,
                      dehydrator::config::HARDWARE.pins.encoderB);
Bounce buttonDebouncer;
byte heartbeatGlyph[8] = {
    B00000,
    B01010,
    B11111,
    B11111,
    B01110,
    B00100,
    B00000,
    B00000,
};

/**
 * @brief Arduino LCD adapter for the project character display interface.
 */
class ArduinoLcdCharacterDisplay final : public dehydrator::CharacterDisplay {
 public:
  /**
   * @brief Creates an adapter around a LiquidCrystal_I2C instance.
   *
   * @param lcd Hardware LCD driver.
   */
  explicit ArduinoLcdCharacterDisplay(LiquidCrystal_I2C& lcd) : lcd_(lcd) {}

  /**
   * @brief Moves the LCD cursor.
   *
   * @param column Zero-based display column.
   * @param row Zero-based display row.
   */
  void setCursor(uint8_t column, uint8_t row) override {
    lcd_.setCursor(column, row);
  }

  /**
   * @brief Writes one normal character.
   *
   * @param value Character byte to write.
   */
  void writeChar(char value) override { lcd_.write(static_cast<uint8_t>(value)); }

  /**
   * @brief Writes one custom character slot.
   *
   * @param code Custom character slot code.
   */
  void writeCustom(uint8_t code) override { lcd_.write(code); }

 private:
  LiquidCrystal_I2C& lcd_;
};

/**
 * @brief Arduino digital output adapter used by relay output helpers.
 */
class ArduinoDigitalOutput final : public dehydrator::DigitalOutput {
 public:
  /**
   * @brief Preloads one pin level and configures it as OUTPUT.
   *
   * @param pin MCU pin number.
   * @param initialHigh Initial logical high/low level.
   */
  void configureOutput(uint8_t pin, bool initialHigh) override {
    digitalWrite(pin, initialHigh ? HIGH : LOW);
    pinMode(pin, OUTPUT);
  }

  /**
   * @brief Writes one logical level to a configured output pin.
   *
   * @param pin MCU pin number.
   * @param high True for HIGH, false for LOW.
   */
  void write(uint8_t pin, bool high) override {
    digitalWrite(pin, high ? HIGH : LOW);
  }
};

ArduinoLcdCharacterDisplay lcdDisplay(lcd);
ArduinoDigitalOutput digitalOutput;
dehydrator::RelayOutputs relayOutputs(digitalOutput, dehydrator::config::HARDWARE);
dehydrator::LcdStatusView statusView(lcdDisplay);
dehydrator::ArduinoAnalogInput analogInput;
dehydrator::ArduinoDhtSensorDriver tempRhDriver(
    dehydrator::config::HARDWARE.pins.tempRhData);
dehydrator::NtcReader ntcReader(
    analogInput, dehydrator::config::HARDWARE.pins.ntcAnalog,
    dehydrator::config::CALIBRATION);
dehydrator::TempRhReader tempRhReader(tempRhDriver,
                                      dehydrator::config::CALIBRATION);
dehydrator::MenuController menuController;
dehydrator::ManualModeController manualModeController;
dehydrator::PresetSelectController presetSelectController;

/**
 * @brief Arduino `Stream` adapter for the project log sink interface.
 */
class ArduinoSerialLogSink final : public dehydrator::LogSink {
 public:
  /**
   * @brief Creates a sink that writes lines to an Arduino stream.
   *
   * @param serial Arduino stream used as the destination.
   */
  explicit ArduinoSerialLogSink(Stream& serial) : serial_(serial) {}

  /**
   * @brief Writes a log line followed by the stream newline.
   *
   * @param line Null-terminated structured log line.
   */
  void writeLine(const char* line) override { serial_.println(line); }

 private:
  Stream& serial_;
};

ArduinoSerialLogSink usbLogSink(Serial);
ArduinoSerialLogSink telemetryLogSink(Serial1);
dehydrator::LogSink* logSinks[dehydrator::config::LOGGING.sinkCapacity] = {};
dehydrator::LogDispatcher logger(logSinks,
                                 dehydrator::config::LOGGING.sinkCapacity);
dehydrator::LcdMenuView menuView(lcdDisplay);
dehydrator::LcdManualView manualView(lcdDisplay);
dehydrator::LcdPresetView presetView(lcdDisplay);

/**
 * @brief Returns whether a pin has been assigned in hardware configuration.
 *
 * @param pin Pin value from `HardwareConfig`.
 * @return true when the pin is usable.
 */
bool isAssignedPin(uint8_t pin) {
  return pin != 255U;
}

/**
 * @brief Converts an active-level logical state into a physical pin level.
 *
 * @param activeLevel Configured active polarity.
 * @param active Logical active state.
 * @return Arduino HIGH/LOW value for the pin.
 */
uint8_t activeLevelToPinValue(dehydrator::config::ActiveLevel activeLevel,
                              bool active) {
  const bool high = activeLevel == dehydrator::config::ActiveLevel::ActiveHigh
                        ? active
                        : !active;
  return high ? HIGH : LOW;
}

/**
 * @brief Writes one structured log line to all configured sinks.
 *
 * @param line Null-terminated structured log line.
 */
void writeLogLine(const char* line) {
  logger.writeLine(line);
}

/**
 * @brief Formats and writes a structured event line.
 *
 * @param type Stable event type token.
 * @param detail Stable event detail token.
 */
void logEvent(const char* type, const char* detail) {
  char line[dehydrator::config::LOGGING.lineSize] = {};
  if (dehydrator::LogFormatter::formatEvent(line, sizeof(line), type, detail)) {
    writeLogLine(line);
    return;
  }

  writeLogLine(LOG_TRUNCATED_EVENT);
}

/**
 * @brief Maps a UI action to a structured log event when needed.
 *
 * @param result Action produced by the menu controller.
 */
void logUiResult(const dehydrator::UiResult& result) {
  switch (result.action) {
    case dehydrator::UiAction::OpenMenu:
      logEvent("ui", "menu_open");
      return;
    case dehydrator::UiAction::CloseMenu:
      logEvent("ui", "menu_close");
      return;
    case dehydrator::UiAction::MoveSelection:
      logEvent("ui", menuController.currentToken());
      return;
    case dehydrator::UiAction::SelectItem:
      logEvent("menu_select", menuController.currentToken());
      return;
    case dehydrator::UiAction::None:
    default:
      return;
  }
}

/**
 * @brief Logs manual-mode UI transitions and output changes.
 *
 * @param result Result produced by the manual mode controller.
 */
void logManualResult(const dehydrator::ManualUiResult& result) {
  if (result.selectionChanged) {
    logEvent("ui", manualModeController.selectedField() ==
                           dehydrator::ManualField::Fan
                       ? "manual_fan"
                       : "manual_heat");
  }

  if (result.outputChanged) {
    const dehydrator::OutputCommand command = manualModeController.command();
    logEvent("output", command.fanOn ? "fan_on" : "fan_off");
    logEvent("output", command.heaterOn ? "heat_on" : "heat_off");
  }

  if (result.exitToMenu) {
    logEvent("ui", "manual_close");
  }
}

/**
 * @brief Logs preset selection UI transitions and choices.
 *
 * @param result Result produced by the preset selection controller.
 */
void logPresetResult(const dehydrator::PresetUiResult& result) {
  if (result.selectionChanged) {
    logEvent("ui", presetSelectController.currentToken());
  }

  if (result.presetSelected) {
    logEvent("preset_select", presetSelectController.currentToken());
  }

  if (result.exitToMenu) {
    logEvent("ui", "preset_close");
  }
}

/**
 * @brief Logs lifecycle state changes for the active preset-run shell.
 *
 * @param previous Previous lifecycle state token.
 * @param current Current lifecycle state token.
 */
void logRunStateChange(const char* previous, const char* current) {
  if (previous == nullptr || current == nullptr || strcmp(previous, current) == 0) {
    return;
  }

  logEvent("run_state", current);
}

/**
 * @brief Cooperative LED task used by the scheduler shell.
 *
 * @param nowMs Current firmware uptime in milliseconds.
 */
void updateLedTask(uint32_t nowMs) {
  if (!ledTask.shouldRun(nowMs)) {
    return;
  }

  ledOn = !ledOn;
  digitalWrite(dehydrator::config::HARDWARE.pins.statusLed, ledOn ? HIGH : LOW);
  logEvent("led", ledOn ? "on" : "off");
}

/**
 * @brief Cooperative primary thermistor sampling task.
 *
 * @param nowMs Current firmware uptime in milliseconds.
 */
void updateSensorTask(uint32_t nowMs) {
  if (!sensorSampleTask.shouldRun(nowMs)) {
    return;
  }

  latestNtc = ntcReader.read();
}

/**
 * @brief Cooperative secondary temp/RH sampling task.
 *
 * @param nowMs Current firmware uptime in milliseconds.
 */
void updateTempRhTask(uint32_t nowMs) {
  if (!tempRhSampleTask.shouldRun(nowMs)) {
    return;
  }

  latestTempRh = tempRhReader.read();
}

/**
 * @brief Advances the active preset run and applies logical relay outputs.
 *
 * @param nowMs Current firmware uptime in milliseconds.
 */
void updateRunControlTask(uint32_t nowMs) {
  if (!runControlTask.shouldRun(nowMs)) {
    return;
  }

  const char* previousState = presetRunController.stateToken();
  presetRunController.update(1U, latestNtc.valid, latestNtc.tempC);
  logRunStateChange(previousState, presetRunController.stateToken());

  activeOutputCommand = presetRunController.outputCommand();
  relayOutputs.apply(activeOutputCommand);
}

/**
 * @brief Cooperative LCD status refresh task.
 *
 * @param nowMs Current firmware uptime in milliseconds.
 */
void updateLcdTask(uint32_t nowMs) {
  if (!lcdRefreshTask.shouldRun(nowMs)) {
    return;
  }

  heartbeatOn = !heartbeatOn;
  if (currentScreen == BringupScreen::Manual) {
    dehydrator::LcdManualSnapshot manualSnapshot;
    manualSnapshot.selectedField = manualModeController.selectedField();
    manualSnapshot.command = manualModeController.command();
    manualSnapshot.heartbeatOn = heartbeatOn;
    manualView.render(manualSnapshot);
    return;
  }

  if (currentScreen == BringupScreen::Preset) {
    dehydrator::LcdPresetSnapshot presetSnapshot;
    presetSnapshot.presets = dehydrator::PresetCatalog::items();
    presetSnapshot.presetCount = dehydrator::PresetCatalog::PRESET_COUNT;
    presetSnapshot.selectedIndex = presetSelectController.selectedIndex();
    presetSnapshot.heartbeatOn = heartbeatOn;
    presetView.render(presetSnapshot);
    return;
  }

  if (currentScreen == BringupScreen::Menu) {
    dehydrator::LcdMenuSnapshot menuSnapshot;
    menuSnapshot.items = dehydrator::MenuController::items();
    menuSnapshot.itemCount = dehydrator::MenuController::ITEM_COUNT;
    menuSnapshot.selectedIndex = menuController.selectedIndex();
    menuSnapshot.heartbeatOn = heartbeatOn;
    menuView.render(menuSnapshot);
    return;
  }

  dehydrator::LcdStatusSnapshot snapshot;
  snapshot.stateLabel = presetRunController.stateLabelRo();
  snapshot.ntcTempC = latestNtc.tempC;
  snapshot.ntcValid = latestNtc.valid;
  snapshot.rhPercent = latestTempRh.rhPercent;
  snapshot.rhValid = latestTempRh.valid;
  snapshot.heaterOn = activeOutputCommand.heaterOn;
  snapshot.fanOn = activeOutputCommand.fanOn;
  snapshot.heartbeatOn = heartbeatOn;
  statusView.render(snapshot);
}

/**
 * @brief Logs rotary encoder and pushbutton events without blocking.
 *
 * @param nowMs Current firmware uptime in milliseconds.
 */
void updateInputTask(uint32_t nowMs) {
  if (!inputScanTask.shouldRun(nowMs)) {
    return;
  }

  const long position = rotaryEncoder.read();
  const int8_t delta = encoderStepFilter.update(position);
  if (delta != 0) {
    logEvent("input", delta > 0 ? "encoder_cw" : "encoder_ccw");
    if (currentScreen == BringupScreen::Manual) {
      logManualResult(manualModeController.onRotate(delta));
    } else if (currentScreen == BringupScreen::Preset) {
      logPresetResult(presetSelectController.onRotate(delta));
    } else if (currentScreen == BringupScreen::Menu) {
      logUiResult(menuController.onRotate(delta));
    }
  }

  buttonDebouncer.update();
  if (buttonDebouncer.fell()) {
    buttonPressed = true;
    buttonPressedAtMs = nowMs;
    logEvent("input", "button_down");
  }

  if (buttonDebouncer.rose()) {
    const uint32_t pressedMs = nowMs - buttonPressedAtMs;
    buttonPressed = false;
    if (pressedMs >= 1000UL) {
      logEvent("input", "button_long");
      if (currentScreen == BringupScreen::Manual) {
        const dehydrator::ManualUiResult result = manualModeController.onLongPress();
        logManualResult(result);
        if (result.exitToMenu) {
          currentScreen = BringupScreen::Menu;
        }
      } else if (currentScreen == BringupScreen::Preset) {
        const dehydrator::PresetUiResult result = presetSelectController.onLongPress();
        logPresetResult(result);
        if (result.exitToMenu) {
          currentScreen = BringupScreen::Menu;
        }
      } else if (currentScreen == BringupScreen::Menu) {
        return;
      }
      return;
    }

    logEvent("input", "button_short");
    if (currentScreen == BringupScreen::Manual) {
      const dehydrator::ManualUiResult result = manualModeController.onShortPress();
      logManualResult(result);
      if (result.exitToMenu) {
        currentScreen = BringupScreen::Menu;
      }
      return;
    }

    if (currentScreen == BringupScreen::Preset) {
      const dehydrator::PresetUiResult result = presetSelectController.onShortPress();
      logPresetResult(result);
      if (result.presetSelected) {
        const dehydrator::PresetDefinition* preset =
            presetSelectController.currentPreset();
        if (preset != nullptr && presetRunController.startPreset(*preset)) {
          activeOutputCommand = presetRunController.outputCommand();
          relayOutputs.apply(activeOutputCommand);
          logEvent("preset_start", preset->token);
          logEvent("run_state", presetRunController.stateToken());
          currentScreen = BringupScreen::Status;
        } else {
          logEvent("preset_start", "rejected");
        }
      } else if (result.exitToMenu) {
        currentScreen = BringupScreen::Menu;
      }
      return;
    }

    const dehydrator::UiResult result = menuController.onShortPress();

    if (currentScreen == BringupScreen::Status &&
        presetRunController.snapshot().state ==
            dehydrator::RunState::FinishedAlarm &&
        result.action == dehydrator::UiAction::OpenMenu) {
      if (presetRunController.acknowledgeFinished()) {
        activeOutputCommand = presetRunController.outputCommand();
        relayOutputs.apply(activeOutputCommand);
        logEvent("run_ack", "finished");
      }
      return;
    }

    logUiResult(result);
    if (result.action == dehydrator::UiAction::OpenMenu) {
      currentScreen = BringupScreen::Menu;
      return;
    }

    if (result.action == dehydrator::UiAction::SelectItem &&
        menuController.currentToken() == nullptr) {
      return;
    }

    if (result.action == dehydrator::UiAction::SelectItem &&
        menuController.currentToken() != nullptr &&
        strcmp(menuController.currentToken(), "mod_manual") == 0) {
      currentScreen = BringupScreen::Manual;
      logEvent("ui", "manual_open");
    } else if (result.action == dehydrator::UiAction::SelectItem &&
               menuController.currentToken() != nullptr &&
               strcmp(menuController.currentToken(), "pornire_preset") == 0) {
      currentScreen = BringupScreen::Preset;
      logEvent("ui", "preset_open");
    } else if (result.action == dehydrator::UiAction::CloseMenu) {
      currentScreen = BringupScreen::Status;
    }
  }
}

/**
 * @brief Cooperative periodic state logging task.
 *
 * @param nowMs Current firmware uptime in milliseconds.
 */
void updateStateLogTask(uint32_t nowMs) {
  if (!stateLogTask.shouldRun(nowMs)) {
    return;
  }

  const dehydrator::PresetDefinition* activePreset = presetRunController.activePreset();
  char line[dehydrator::config::LOGGING.lineSize] = {};
  if (dehydrator::LogFormatter::formatBringupState(
          line, sizeof(line), nowMs, ledOn, latestNtc.valid,
          latestNtc.tempC, latestNtc.adcCount, latestTempRh.valid,
          latestTempRh.tempC, latestTempRh.rhPercent,
          presetRunController.stateToken(),
          activePreset != nullptr ? activePreset->token : nullptr,
          activeOutputCommand.heaterOn, activeOutputCommand.fanOn)) {
    writeLogLine(line);
    return;
  }

  writeLogLine(LOG_TRUNCATED_STATE);
}

void setup() {
  pinMode(dehydrator::config::HARDWARE.pins.statusLed, OUTPUT);
  digitalWrite(dehydrator::config::HARDWARE.pins.statusLed, LOW);

  if (isAssignedPin(dehydrator::config::HARDWARE.pins.lcdBacklight)) {
    pinMode(dehydrator::config::HARDWARE.pins.lcdBacklight, OUTPUT);
    digitalWrite(
        dehydrator::config::HARDWARE.pins.lcdBacklight,
        activeLevelToPinValue(
            dehydrator::config::HARDWARE.lcdBacklightActiveLevel, true));
  }

  if (isAssignedPin(dehydrator::config::HARDWARE.pins.encoderButton)) {
    buttonDebouncer.attach(dehydrator::config::HARDWARE.pins.encoderButton,
                           INPUT_PULLUP);
    buttonDebouncer.interval(5U);
  }

  Serial.begin(dehydrator::config::SERIAL_PORTS.baudRate);
  Serial1.begin(dehydrator::config::SERIAL_PORTS.baudRate);
  unsigned long serial_start = millis();
  while (!Serial &&
         millis() - serial_start <
             dehydrator::config::SCHEDULER.serialStartupWaitMs) {
    // Wait briefly for native USB serial boards, then continue anyway.
  }

  logger.addSink(usbLogSink);
  logger.addSink(telemetryLogSink);

  if (tempRhDriver.begin()) {
    logEvent("sensor", "temp_rh_ready");
  } else {
    logEvent("sensor", "temp_rh_missing");
  }

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.createChar(dehydrator::LcdStatusView::HEARTBEAT_CHAR, heartbeatGlyph);
  relayOutputs.begin();
  lastEncoderPosition = rotaryEncoder.read();
  encoderStepFilter.reset(lastEncoderPosition);

  latestNtc = ntcReader.read();
  latestTempRh = tempRhReader.read();

  controlStateMachine.enterBoot();
  logEvent("run_state", controlStateMachine.stateToken());

  dehydrator::ControlStartupInput startupInput;
  startupInput.configValid = true;
  startupInput.outputsSafe = true;
  startupInput.primarySensorValid = latestNtc.valid;
  startupInput.buttonSafe = !buttonPressed;
  startupInput.watchdogResetDuringRun = false;
  startupInput.interruptedRunAvailable = false;

  const dehydrator::ControlStartupResult startupResult =
      controlStateMachine.completeSelfCheck(startupInput);
  logEvent("run_state", controlStateMachine.stateToken());
  if (!startupResult.passed && startupResult.faultToken != nullptr) {
    logEvent("fault", startupResult.faultToken);
  }

  writeLogLine("EVENT type=boot detail=scheduler_shell");
}

void loop() {
  const uint32_t nowMs = millis();

  updateLedTask(nowMs);
  updateSensorTask(nowMs);
  updateTempRhTask(nowMs);
  updateRunControlTask(nowMs);
  updateStateLogTask(nowMs);
  updateInputTask(nowMs);
  updateLcdTask(nowMs);
}

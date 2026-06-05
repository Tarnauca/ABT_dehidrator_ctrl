#include <Arduino.h>
#include <Bounce2.h>
#include <DHT.h>
#include <EEPROM.h>
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
#include "dehydrator/hardware/ArduinoEepromStorage.h"
#include "dehydrator/interfaces/DigitalOutput.h"
#include "dehydrator/interfaces/CharacterDisplay.h"
#include "dehydrator/logging/LogDispatcher.h"
#include "dehydrator/logging/LogFormatter.h"
#include "dehydrator/logging/LogSink.h"
#include "dehydrator/sensors/TempRhReader.h"
#include "dehydrator/sensors/NtcReader.h"
#include "dehydrator/presets/PresetCatalog.h"
#include "dehydrator/persistence/UserProfileStore.h"
#include "dehydrator/ui/LcdBinaryConfirmView.h"
#include "dehydrator/ui/LcdTestView.h"
#include "dehydrator/ui/LcdConfirmReplaceRunView.h"
#include "dehydrator/ui/LcdManualProgramView.h"
#include "dehydrator/ui/LcdPresetView.h"
#include "dehydrator/ui/LcdMenuView.h"
#include "dehydrator/ui/LcdSavePromptView.h"
#include "dehydrator/ui/LcdStatusView.h"
#include "dehydrator/ui/LcdUserProfileDetailView.h"
#include "dehydrator/ui/LcdUserProfileSlotView.h"
#include "dehydrator/ui/ConfirmReplaceRunController.h"
#include "dehydrator/ui/ManualProgramController.h"
#include "dehydrator/ui/SavePromptController.h"
#include "dehydrator/ui/SettingsMenuController.h"
#include "dehydrator/ui/TestModeController.h"
#include "dehydrator/ui/PresetSelectController.h"
#include "dehydrator/ui/MenuController.h"
#include "dehydrator/ui/UserProfileActionController.h"
#include "dehydrator/ui/UserProfileSlotController.h"

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
bool activityBlinkOn = false;
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
  Test,
  ManualProgram,
  Preset,
  SettingsMenu,
  ConfirmReplaceRun,
  SavePrompt,
  UserProfileSlots,
  UserProfileDetail,
  BinaryConfirm,
};

enum class ManualSaveFlowOrigin {
  None,
  ExplicitSave,
  StartRequest,
  BackRequest,
};

enum class UserProfileSlotScreenPurpose {
  Browse,
  Save,
};

enum class BinaryConfirmPurpose {
  None,
  ReplaceRun,
  StopProgram,
  OverwriteSlot,
  DeleteSlot,
};

/**
 * @brief Source/category displayed for the current active program.
 */
enum class ActiveProgramDisplayKind {
  /** No active program is currently associated with the status screen. */
  Inactive,
  /** One built-in preset is active. */
  Preset,
  /** One saved user profile slot is active. */
  UserProfile,
  /** One unsaved/manual editor program is active. */
  Manual,
};

/**
 * @brief Compact status-screen identity for the active program.
 */
struct ActiveProgramDisplayContext {
  /** Current status identity category. */
  ActiveProgramDisplayKind kind = ActiveProgramDisplayKind::Inactive;
  /** Preset label when @ref kind is `Preset`. */
  const char* presetLabel = nullptr;
  /** One-based user profile slot number when @ref kind is `UserProfile`. */
  uint8_t userProfileNumber = 0U;
  /** Manual-program mode when @ref kind is `Manual`. */
  dehydrator::ManualProgramMode manualMode =
      dehydrator::ManualProgramMode::Constant;
};

BringupScreen currentScreen = BringupScreen::Status;

LiquidCrystal_I2C lcd(dehydrator::config::HARDWARE.lcdI2cAddress,
                      dehydrator::LcdStatusView::COLUMNS,
                      dehydrator::LcdStatusView::ROWS);
Encoder rotaryEncoder(dehydrator::config::HARDWARE.pins.encoderA,
                      dehydrator::config::HARDWARE.pins.encoderB);
Bounce buttonDebouncer;
byte playGlyph[8] = {
    B00100,
    B00110,
    B00111,
    B00111,
    B00111,
    B00110,
    B00100,
    B00000,
};
byte pauseGlyph[8] = {
    B00000,
    B01010,
    B01010,
    B01010,
    B01010,
    B01010,
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
dehydrator::SettingsMenuController settingsMenuController;
dehydrator::TestModeController testModeController;
dehydrator::ManualProgramController manualProgramController;
dehydrator::PresetSelectController presetSelectController;
dehydrator::ConfirmReplaceRunController confirmReplaceRunController;
dehydrator::SavePromptController savePromptController;
dehydrator::UserProfileSlotController userProfileSlotController;
dehydrator::UserProfileActionController userProfileActionController;
dehydrator::ArduinoEepromStorage eepromStorage;
dehydrator::UserProfileStore userProfileStore(eepromStorage);
dehydrator::UserProfileSlotRecord
    userProfileSlots[dehydrator::UserProfileStore::SLOT_COUNT] = {};
const dehydrator::PresetDefinition* pendingPresetSelection = nullptr;
bool pendingManualProgramStart = false;
dehydrator::ProfileConfig pendingManualProgramProfile;
BringupScreen replaceRunCancelScreen = BringupScreen::Status;
ManualSaveFlowOrigin manualSaveFlowOrigin = ManualSaveFlowOrigin::None;
UserProfileSlotScreenPurpose userProfileSlotScreenPurpose =
    UserProfileSlotScreenPurpose::Browse;
BinaryConfirmPurpose binaryConfirmPurpose = BinaryConfirmPurpose::None;
uint8_t pendingProfileSlot = 0U;
uint8_t activeProfileDetailSlot = 0U;
ActiveProgramDisplayContext activeProgramDisplay;
ActiveProgramDisplayContext pendingRunDisplay;
uint8_t statusPageIndex = 0U;

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
dehydrator::LcdTestView testView(lcdDisplay);
dehydrator::LcdManualProgramView manualProgramView(lcdDisplay);
dehydrator::LcdPresetView presetView(lcdDisplay);
dehydrator::LcdConfirmReplaceRunView confirmReplaceRunView(lcdDisplay);
dehydrator::LcdSavePromptView savePromptView(lcdDisplay);
dehydrator::LcdUserProfileSlotView userProfileSlotView(lcdDisplay);
dehydrator::LcdUserProfileDetailView userProfileDetailView(lcdDisplay);
dehydrator::LcdBinaryConfirmView binaryConfirmView(lcdDisplay);

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
 * @brief Writes a plain-text splash banner to all serial log sinks.
 */
void writeBootSplash() {
  writeLogLine("+----------------------+");
  writeLogLine("| ABT Dehidrator Ctrl  |");
  writeLogLine("| Mega2560 Boot        |");
  writeLogLine("+----------------------+");
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
 * @brief Logs one startup self-check verdict as a stable structured event.
 *
 * @param check Stable self-check item token.
 * @param passed Whether the check passed.
 */
void logSelfCheckVerdict(const char* check, bool passed) {
  char detail[dehydrator::config::LOGGING.lineSize] = {};
  if (snprintf(detail, sizeof(detail), "%s_%s", check,
               passed ? "ok" : "fail") > 0) {
    logEvent("self_check", detail);
  }
}

/**
 * @brief Logs one startup self-check status token that is not pass/fail.
 *
 * @param check Stable self-check item token.
 * @param status Stable status token for that check.
 */
void logSelfCheckStatus(const char* check, const char* status) {
  char detail[dehydrator::config::LOGGING.lineSize] = {};
  if (snprintf(detail, sizeof(detail), "%s_%s", check, status) > 0) {
    logEvent("self_check", detail);
  }
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
 * @brief Logs direct-output test UI transitions and output changes.
 *
 * @param result Result produced by the test mode controller.
 */
void logTestResult(const dehydrator::TestUiResult& result) {
  if (result.selectionChanged) {
    switch (testModeController.selectedField()) {
      case dehydrator::TestField::NtcTemp:
        logEvent("ui", "test_ntc");
        break;
      case dehydrator::TestField::TempRhTemp:
        logEvent("ui", "test_temp_rh_temp");
        break;
      case dehydrator::TestField::TempRhRh:
        logEvent("ui", "test_temp_rh_rh");
        break;
      case dehydrator::TestField::Fan:
        logEvent("ui", "test_fan");
        break;
      case dehydrator::TestField::Heater:
        logEvent("ui", "test_heat");
        break;
      case dehydrator::TestField::Back:
      default:
        logEvent("ui", "test_back");
        break;
    }
  }

  if (result.outputChanged) {
    const dehydrator::OutputCommand command = testModeController.command();
    logEvent("output", command.fanOn ? "fan_on" : "fan_off");
    logEvent("output", command.heaterOn ? "heat_on" : "heat_off");
  }

  if (result.exitToMenu) {
    logEvent("ui", "test_close");
  }
}

/**
 * @brief Logs manual-program UI transitions and edits.
 *
 * @param result Result produced by the manual program controller.
 */
void logManualProgramResult(const dehydrator::ManualProgramUiResult& result) {
  if (result.selectionChanged) {
    switch (manualProgramController.selectedField()) {
      case dehydrator::ManualProgramField::Mode:
        logEvent("ui", "manual_mode");
        return;
      case dehydrator::ManualProgramField::Temperature:
        logEvent("ui", "manual_temp");
        return;
      case dehydrator::ManualProgramField::Duration:
        logEvent("ui", "manual_duration");
        return;
      case dehydrator::ManualProgramField::BoostDelta:
        logEvent("ui", "manual_boost_delta");
        return;
      case dehydrator::ManualProgramField::BoostDuration:
        logEvent("ui", "manual_boost_duration");
        return;
      case dehydrator::ManualProgramField::UpperTemp:
        logEvent("ui", "manual_tsup");
        return;
      case dehydrator::ManualProgramField::LowerTemp:
        logEvent("ui", "manual_tinf");
        return;
      case dehydrator::ManualProgramField::UpperDuration:
        logEvent("ui", "manual_tsup_duration");
        return;
      case dehydrator::ManualProgramField::LowerDuration:
        logEvent("ui", "manual_tinf_duration");
        return;
      case dehydrator::ManualProgramField::Start:
        logEvent("ui", "manual_start");
        return;
      case dehydrator::ManualProgramField::Save:
        logEvent("ui", "manual_save");
        return;
      case dehydrator::ManualProgramField::Back:
        logEvent("ui", "manual_back");
        return;
    }
  }

  if (result.valueChanged) {
    logEvent("param", "manual_changed");
  }

  if (result.startRequested) {
    logEvent("run_manual", "requested");
  }

  if (result.saveRequested) {
    logEvent("profile_save", "requested");
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
 * @brief Logs replace-run confirmation UI transitions.
 *
 * @param result Result produced by the confirm-replace-run controller.
 */
void logConfirmReplaceRunResult(
    const dehydrator::ConfirmReplaceRunResult& result) {
  if (result.selectionChanged) {
    logEvent("ui", confirmReplaceRunController.confirmSelected() ? "confirm_da"
                                                                 : "confirm_nu");
  }

  if (result.confirmed) {
    logEvent("run_replace", "confirmed");
  } else if (result.cancelled) {
    logEvent("run_replace", "cancelled");
  }
}

/**
 * @brief Reloads the cached 10-slot user-profile directory from EEPROM.
 */
void refreshUserProfileSlots() {
  for (uint8_t slot = 0U; slot < dehydrator::UserProfileStore::SLOT_COUNT;
       slot++) {
    dehydrator::UserProfileSlotRecord record;
    if (!userProfileStore.load(slot, record)) {
      record = {};
    }
    userProfileSlots[slot] = record;
  }
}

/**
 * @brief Returns whether one saved user slot currently contains a profile.
 *
 * @param slotIndex Zero-based slot index.
 * @return true when the cached slot is occupied.
 */
bool isOccupiedUserProfileSlot(uint8_t slotIndex) {
  return slotIndex < dehydrator::UserProfileStore::SLOT_COUNT &&
         userProfileSlots[slotIndex].occupied;
}

/**
 * @brief Opens the user-profile slot list for browsing or saving.
 *
 * @param purpose Whether the slot list is used for browse or save flow.
 * @param preferredSlot Optional preferred slot selection.
 * @param hasPreferredSlot True when `preferredSlot` should be preselected.
 */
void openUserProfileSlots(UserProfileSlotScreenPurpose purpose,
                          uint8_t preferredSlot = 0U,
                          bool hasPreferredSlot = false) {
  refreshUserProfileSlots();
  userProfileSlotScreenPurpose = purpose;
  userProfileSlotController.reset();
  if (hasPreferredSlot) {
    userProfileSlotController.setSelectedSlot(preferredSlot);
  }
  currentScreen = BringupScreen::UserProfileSlots;
}

/**
 * @brief Opens the detail view for one user-profile slot.
 *
 * @param slotIndex Zero-based slot index.
 */
void openUserProfileDetail(uint8_t slotIndex) {
  if (slotIndex >= dehydrator::UserProfileStore::SLOT_COUNT) {
    return;
  }

  activeProfileDetailSlot = slotIndex;
  userProfileActionController.reset();
  userProfileActionController.setOccupied(isOccupiedUserProfileSlot(slotIndex));
  currentScreen = BringupScreen::UserProfileDetail;
}

/**
 * @brief Returns whether one running or resumable program may be stopped.
 */
bool hasStoppableProgram() {
  const dehydrator::RunStateSnapshot snapshot = presetRunController.snapshot();
  return snapshot.state == dehydrator::RunState::Running ||
         snapshot.state == dehydrator::RunState::Paused ||
         snapshot.state == dehydrator::RunState::FinishCooldown ||
         snapshot.state == dehydrator::RunState::FinishedAlarm;
}

/**
 * @brief Returns whether one previously paused/resumable program exists.
 */
bool hasResumableProgram() {
  const dehydrator::RunStateSnapshot snapshot = presetRunController.snapshot();
  return snapshot.state == dehydrator::RunState::Paused && snapshot.resumeAllowed;
}

/**
 * @brief Returns whether one active run may currently be paused by the user.
 */
bool hasPausableProgram() {
  return presetRunController.snapshot().state == dehydrator::RunState::Running;
}

/**
 * @brief Refreshes dynamic main-menu visibility flags from run state.
 */
void syncMainMenuContext() {
  dehydrator::MainMenuContext context;
  context.showStopProgram = hasStoppableProgram();
  context.showPauseProgram = hasPausableProgram();
  context.showResumeProgram = hasResumableProgram();
  menuController.setContext(context);
}

/**
 * @brief Clears the active program identity shown on the status screen.
 */
void clearActiveProgramDisplay() { activeProgramDisplay = {}; }

/**
 * @brief Returns the compact manual-mode label used on the 20-column status LCD.
 *
 * @param mode Current manual mode.
 * @return Compact label that still communicates the selected manual variant.
 */
const char* manualStatusLabel(dehydrator::ManualProgramMode mode) {
  switch (mode) {
    case dehydrator::ManualProgramMode::Boost:
      return "Man. Boost";
    case dehydrator::ManualProgramMode::Fluctuating:
      return "Man. Fluct";
    case dehydrator::ManualProgramMode::Constant:
    default:
      return "Man. Const";
  }
}

/**
 * @brief Builds the display/run-token context for launching the manual editor profile.
 *
 * Saved user slots launched without unsaved changes are shown as user profiles;
 * all other manual-editor launches are shown as manual programs.
 *
 * @return Compact display identity and log token for the pending run.
 */
ActiveProgramDisplayContext manualEditorDisplayContext() {
  ActiveProgramDisplayContext context;
  if (manualProgramController.hasAssociatedSlot() && !manualProgramController.dirty()) {
    context.kind = ActiveProgramDisplayKind::UserProfile;
    context.userProfileNumber =
        static_cast<uint8_t>(manualProgramController.associatedSlot() + 1U);
    return context;
  }

  context.kind = ActiveProgramDisplayKind::Manual;
  context.manualMode = manualProgramController.mode();
  return context;
}

/**
 * @brief Returns the run token matching one active-program display context.
 *
 * @param context Program identity selected for the launch.
 * @return Stable English-ish token for logs.
 */
const char* runTokenForDisplayContext(const ActiveProgramDisplayContext& context) {
  return context.kind == ActiveProgramDisplayKind::UserProfile ? "user_profile"
                                                               : "manual";
}

/**
 * @brief Resets status-page navigation to the summary page.
 */
void resetStatusPage() { statusPageIndex = 0U; }

/**
 * @brief Returns the current number of status pages available to the user.
 *
 * @return Page count including summary and outputs.
 */
uint8_t statusPageCount() {
  uint8_t count = 2U;
  const dehydrator::ProfileConfig* profile = presetRunController.activeProfile();
  if (profile == nullptr) {
    return count;
  }

  count++;
  if (profile->mode == dehydrator::ProfileMode::Fluctuating) {
    count++;
  }
  return count;
}

/**
 * @brief Clamps the current status-page index to the available page count.
 */
void clampStatusPage() {
  const uint8_t count = statusPageCount();
  if (statusPageIndex >= count) {
    statusPageIndex = static_cast<uint8_t>(count - 1U);
  }
}

/**
 * @brief Advances or rewinds the status-page selection.
 *
 * @param delta Positive for clockwise, negative for counter-clockwise.
 */
void navigateStatusPage(int8_t delta) {
  clampStatusPage();
  const uint8_t count = statusPageCount();
  const uint8_t previous = statusPageIndex;

  if (delta > 0 && statusPageIndex + 1U < count) {
    statusPageIndex++;
  } else if (delta < 0 && statusPageIndex > 0U) {
    statusPageIndex--;
  }

  if (statusPageIndex != previous) {
    logEvent("ui", delta > 0 ? "status_page_next" : "status_page_prev");
  }
}

/**
 * @brief Maps the current status-page index to the corresponding LCD page.
 *
 * @return Logical LCD page to render.
 */
dehydrator::StatusPage currentStatusPage() {
  clampStatusPage();
  const dehydrator::ProfileConfig* profile = presetRunController.activeProfile();
  if (statusPageIndex == 0U) {
    return dehydrator::StatusPage::Summary;
  }

  if (profile == nullptr) {
    return dehydrator::StatusPage::Outputs;
  }

  if (statusPageIndex == 1U) {
    return dehydrator::StatusPage::ParametersPrimary;
  }

  if (profile->mode == dehydrator::ProfileMode::Fluctuating &&
      statusPageIndex == 2U) {
    return dehydrator::StatusPage::ParametersSecondary;
  }

  return dehydrator::StatusPage::Outputs;
}

/**
 * @brief Formats the compact active-program label for the status summary page.
 *
 * @param buffer Caller-owned destination buffer.
 * @param bufferSize Destination buffer size.
 * @return Pointer to the formatted buffer contents.
 */
const char* formatActiveProgramLabel(char* buffer, size_t bufferSize) {
  if (bufferSize == 0U) {
    return "";
  }

  buffer[0] = '\0';
  switch (activeProgramDisplay.kind) {
    case ActiveProgramDisplayKind::Preset:
      snprintf(buffer, bufferSize, "%s",
               activeProgramDisplay.presetLabel == nullptr
                   ? ""
                   : activeProgramDisplay.presetLabel);
      break;
    case ActiveProgramDisplayKind::UserProfile:
      snprintf(buffer, bufferSize, "Profil %u",
               static_cast<unsigned int>(activeProgramDisplay.userProfileNumber));
      break;
    case ActiveProgramDisplayKind::Manual:
      snprintf(buffer, bufferSize, "%s",
               manualStatusLabel(activeProgramDisplay.manualMode));
      break;
    case ActiveProgramDisplayKind::Inactive:
    default:
      snprintf(buffer, bufferSize, "Inactiv");
      break;
  }
  return buffer;
}

/**
 * @brief Starts the current manual editor profile or prompts to replace a run.
 *
 * @param profile Manual profile to run.
 */
void startManualProfile(const dehydrator::ProfileConfig& profile) {
  const ActiveProgramDisplayContext launchDisplay = manualEditorDisplayContext();
  const char* runToken = runTokenForDisplayContext(launchDisplay);
  const char* logType = launchDisplay.kind == ActiveProgramDisplayKind::UserProfile
                            ? "run_profile"
                            : "run_manual";
  if (presetRunController.snapshot().state == dehydrator::RunState::Idle) {
    if (presetRunController.startProfile(profile, runToken)) {
      activeProgramDisplay = launchDisplay;
      activeOutputCommand = presetRunController.outputCommand();
      relayOutputs.apply(activeOutputCommand);
      resetStatusPage();
      logEvent(logType, "started");
      logEvent("run_state", presetRunController.stateToken());
      menuController.returnToStatus();
      currentScreen = BringupScreen::Status;
    } else {
      logEvent(logType, "rejected");
    }
    return;
  }

  pendingPresetSelection = nullptr;
  pendingManualProgramStart = true;
  pendingManualProgramProfile = profile;
  pendingRunDisplay = launchDisplay;
  replaceRunCancelScreen = BringupScreen::ManualProgram;
  confirmReplaceRunController.reset();
  binaryConfirmPurpose = BinaryConfirmPurpose::ReplaceRun;
  logEvent("run_replace", "prompt");
  currentScreen = BringupScreen::ConfirmReplaceRun;
}

/**
 * @brief Completes one manual save request and follows the pending origin flow.
 *
 * @param slotIndex Zero-based user-profile slot index.
 */
void saveManualProfileToSlot(uint8_t slotIndex) {
  if (!userProfileStore.save(slotIndex, manualProgramController.profile())) {
    logEvent("profile_save", "failed");
    return;
  }

  manualProgramController.markSaved(slotIndex);
  refreshUserProfileSlots();
  logEvent("profile_save", "saved");

  if (manualSaveFlowOrigin == ManualSaveFlowOrigin::StartRequest) {
    const dehydrator::ProfileConfig profile = manualProgramController.profile();
    manualSaveFlowOrigin = ManualSaveFlowOrigin::None;
    startManualProfile(profile);
    return;
  }

  if (manualSaveFlowOrigin == ManualSaveFlowOrigin::BackRequest) {
    manualSaveFlowOrigin = ManualSaveFlowOrigin::None;
    menuController.enterMenu();
    currentScreen = BringupScreen::Menu;
    return;
  }

  manualSaveFlowOrigin = ManualSaveFlowOrigin::None;
  currentScreen = BringupScreen::ManualProgram;
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

  activityBlinkOn = !activityBlinkOn;
  if (currentScreen == BringupScreen::Test) {
    dehydrator::LcdTestSnapshot testSnapshot;
    testSnapshot.selectedField = testModeController.selectedField();
    testSnapshot.ntc = latestNtc;
    testSnapshot.tempRh = latestTempRh;
    testSnapshot.command = testModeController.command();
    testView.render(testSnapshot);
    return;
  }

  if (currentScreen == BringupScreen::ManualProgram) {
    dehydrator::LcdManualProgramSnapshot manualProgramSnapshot;
    manualProgramSnapshot.mode = manualProgramController.mode();
    manualProgramSnapshot.selectedField = manualProgramController.selectedField();
    manualProgramSnapshot.selectedIndex = manualProgramController.selectedIndex();
    manualProgramSnapshot.editing = manualProgramController.editing();
    manualProgramSnapshot.targetTempC = manualProgramController.targetTempC();
    manualProgramSnapshot.durationMinutes =
        manualProgramController.durationMinutes();
    manualProgramSnapshot.boostDeltaC = manualProgramController.boostDeltaC();
    manualProgramSnapshot.boostDurationMinutes =
        manualProgramController.boostDurationMinutes();
    manualProgramSnapshot.upperTempC = manualProgramController.upperTempC();
    manualProgramSnapshot.lowerTempC = manualProgramController.lowerTempC();
    manualProgramSnapshot.upperDurationMinutes =
        manualProgramController.upperDurationMinutes();
    manualProgramSnapshot.lowerDurationMinutes =
        manualProgramController.lowerDurationMinutes();
    manualProgramView.render(manualProgramSnapshot);
    return;
  }

  if (currentScreen == BringupScreen::Preset) {
    dehydrator::LcdPresetSnapshot presetSnapshot;
    presetSnapshot.presets = dehydrator::PresetCatalog::items();
    presetSnapshot.presetCount = dehydrator::PresetCatalog::PRESET_COUNT;
    presetSnapshot.selectedIndex = presetSelectController.selectedIndex();
    presetView.render(presetSnapshot);
    return;
  }

  if (currentScreen == BringupScreen::SettingsMenu) {
    const char* labels[dehydrator::SettingsMenuController::ITEM_COUNT] = {};
    dehydrator::SettingsMenuController::fillVisibleItems(labels);

    dehydrator::LcdMenuSnapshot settingsSnapshot;
    settingsSnapshot.title = "Setari";
    settingsSnapshot.items = labels;
    settingsSnapshot.itemCount = dehydrator::SettingsMenuController::ITEM_COUNT;
    settingsSnapshot.selectedIndex = settingsMenuController.selectedIndex();
    menuView.render(settingsSnapshot);
    return;
  }

  if (currentScreen == BringupScreen::ConfirmReplaceRun) {
    dehydrator::LcdConfirmReplaceRunSnapshot confirmSnapshot;
    confirmSnapshot.confirmSelected =
        confirmReplaceRunController.confirmSelected();
    confirmReplaceRunView.render(confirmSnapshot);
    return;
  }

  if (currentScreen == BringupScreen::SavePrompt) {
    dehydrator::LcdSavePromptSnapshot saveSnapshot;
    saveSnapshot.choice = savePromptController.currentChoice();
    savePromptView.render(saveSnapshot);
    return;
  }

  if (currentScreen == BringupScreen::UserProfileSlots) {
    dehydrator::LcdUserProfileSlotSnapshot slotSnapshot;
    slotSnapshot.title =
        userProfileSlotScreenPurpose == UserProfileSlotScreenPurpose::Save
            ? "Salveaza profil"
            : "Programe utilizator";
    slotSnapshot.slots = userProfileSlots;
    slotSnapshot.selectedIndex = userProfileSlotController.selectedIndex();
    userProfileSlotView.render(slotSnapshot);
    return;
  }

  if (currentScreen == BringupScreen::UserProfileDetail) {
    dehydrator::LcdUserProfileDetailSnapshot detailSnapshot;
    detailSnapshot.slotNumber = static_cast<uint8_t>(activeProfileDetailSlot + 1U);
    detailSnapshot.occupied = isOccupiedUserProfileSlot(activeProfileDetailSlot);
    detailSnapshot.profile = userProfileSlots[activeProfileDetailSlot].profile;
    detailSnapshot.action = userProfileActionController.currentAction();
    userProfileDetailView.render(detailSnapshot);
    return;
  }

  if (currentScreen == BringupScreen::BinaryConfirm) {
    dehydrator::LcdBinaryConfirmSnapshot confirmSnapshot;
    confirmSnapshot.confirmSelected =
        confirmReplaceRunController.confirmSelected();
    if (binaryConfirmPurpose == BinaryConfirmPurpose::OverwriteSlot) {
      confirmSnapshot.title = "Suprascriere";
      confirmSnapshot.prompt = "Locatie ocupata?";
    } else if (binaryConfirmPurpose == BinaryConfirmPurpose::StopProgram) {
      confirmSnapshot.title = "Confirmare";
      confirmSnapshot.prompt = "Opresti programul?";
    } else if (binaryConfirmPurpose == BinaryConfirmPurpose::DeleteSlot) {
      confirmSnapshot.title = "Stergere";
      confirmSnapshot.prompt = "Stergi profilul?";
    } else {
      confirmSnapshot.title = "Confirmare";
      confirmSnapshot.prompt = "Continui?";
    }
    binaryConfirmView.render(confirmSnapshot);
    return;
  }

  if (currentScreen == BringupScreen::Menu) {
    syncMainMenuContext();
    const char* labels[dehydrator::MenuController::MAX_ITEM_COUNT] = {};
    menuController.fillVisibleItems(labels);

    dehydrator::LcdMenuSnapshot menuSnapshot;
    menuSnapshot.title = "Meniu";
    menuSnapshot.items = labels;
    menuSnapshot.itemCount = menuController.itemCount();
    menuSnapshot.selectedIndex = menuController.selectedIndex();
    menuView.render(menuSnapshot);
    return;
  }

  clampStatusPage();
  dehydrator::LcdStatusSnapshot snapshot;
  char programLabel[20] = {};
  const dehydrator::RunStateSnapshot runSnapshot = presetRunController.snapshot();
  const dehydrator::ProfileConfig* activeProfile = presetRunController.activeProfile();
  const uint16_t elapsedMinutes =
      static_cast<uint16_t>(runSnapshot.activeElapsedSeconds / 60UL);
  const uint16_t remainingMinutes =
      activeProfile != nullptr && elapsedMinutes < activeProfile->durationMinutes
          ? static_cast<uint16_t>(activeProfile->durationMinutes - elapsedMinutes)
          : 0U;
  snapshot.page = currentStatusPage();
  snapshot.programLabel = formatActiveProgramLabel(programLabel, sizeof(programLabel));
  snapshot.ntcTempC = latestNtc.tempC;
  snapshot.ntcValid = latestNtc.valid;
  snapshot.rhPercent = latestTempRh.rhPercent;
  snapshot.rhValid = latestTempRh.valid;
  snapshot.elapsedMinutes = activeProfile != nullptr ? elapsedMinutes : 0U;
  snapshot.remainingMinutes = remainingMinutes;
  snapshot.profileValid = activeProfile != nullptr;
  if (activeProfile != nullptr) {
    snapshot.profile = *activeProfile;
  }
  snapshot.heaterOn = activeOutputCommand.heaterOn;
  snapshot.fanOn = activeOutputCommand.fanOn;
  const dehydrator::RunState currentRunState = runSnapshot.state;
  snapshot.activityIndicator =
      currentRunState == dehydrator::RunState::Running
          ? dehydrator::StatusActivityIndicator::Running
          : (currentRunState == dehydrator::RunState::Paused
                 ? dehydrator::StatusActivityIndicator::Paused
                 : dehydrator::StatusActivityIndicator::None);
  snapshot.activityIndicatorOn = activityBlinkOn;
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
    if (currentScreen == BringupScreen::Status) {
      navigateStatusPage(delta);
    } else if (currentScreen == BringupScreen::Test) {
      logTestResult(testModeController.onRotate(delta));
    } else if (currentScreen == BringupScreen::ManualProgram) {
      logManualProgramResult(manualProgramController.onRotate(delta));
    } else if (currentScreen == BringupScreen::SavePrompt) {
      savePromptController.onRotate(delta);
    } else if (currentScreen == BringupScreen::UserProfileSlots) {
      userProfileSlotController.onRotate(delta);
    } else if (currentScreen == BringupScreen::UserProfileDetail) {
      userProfileActionController.onRotate(delta);
    } else if (currentScreen == BringupScreen::BinaryConfirm) {
      confirmReplaceRunController.onRotate(delta);
    } else if (currentScreen == BringupScreen::ConfirmReplaceRun) {
      logConfirmReplaceRunResult(confirmReplaceRunController.onRotate(delta));
    } else if (currentScreen == BringupScreen::SettingsMenu) {
      settingsMenuController.onRotate(delta);
    } else if (currentScreen == BringupScreen::Preset) {
      logPresetResult(presetSelectController.onRotate(delta));
    } else if (currentScreen == BringupScreen::Menu) {
      syncMainMenuContext();
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
      return;
    }

    logEvent("input", "button_short");
    if (currentScreen == BringupScreen::Test) {
      const dehydrator::TestUiResult result = testModeController.onShortPress();
      logTestResult(result);
      if (result.exitToMenu) {
        currentScreen = BringupScreen::SettingsMenu;
      }
      return;
    }

    if (currentScreen == BringupScreen::ManualProgram) {
      const dehydrator::ManualProgramUiResult result =
          manualProgramController.onShortPress();
      logManualProgramResult(result);
      if (result.startRequested) {
        if (manualProgramController.dirty()) {
          manualSaveFlowOrigin = ManualSaveFlowOrigin::StartRequest;
          savePromptController.reset();
          currentScreen = BringupScreen::SavePrompt;
        } else {
          startManualProfile(manualProgramController.profile());
        }
      } else if (result.saveRequested) {
        manualSaveFlowOrigin = ManualSaveFlowOrigin::ExplicitSave;
        openUserProfileSlots(UserProfileSlotScreenPurpose::Save,
                             manualProgramController.associatedSlot(),
                             manualProgramController.hasAssociatedSlot());
      } else if (result.exitToMenu) {
        if (manualProgramController.dirty()) {
          manualSaveFlowOrigin = ManualSaveFlowOrigin::BackRequest;
          savePromptController.reset();
          currentScreen = BringupScreen::SavePrompt;
        } else {
          menuController.enterMenu();
          currentScreen = BringupScreen::Menu;
        }
      }
      return;
    }

    if (currentScreen == BringupScreen::SavePrompt) {
      const dehydrator::SavePromptResult result = savePromptController.onShortPress();
      if (result.choice == dehydrator::SavePromptChoice::Yes) {
        openUserProfileSlots(UserProfileSlotScreenPurpose::Save,
                             manualProgramController.associatedSlot(),
                             manualProgramController.hasAssociatedSlot());
      } else if (result.choice == dehydrator::SavePromptChoice::No) {
        if (manualSaveFlowOrigin == ManualSaveFlowOrigin::StartRequest) {
          manualSaveFlowOrigin = ManualSaveFlowOrigin::None;
          startManualProfile(manualProgramController.profile());
        } else if (manualSaveFlowOrigin == ManualSaveFlowOrigin::BackRequest) {
          manualSaveFlowOrigin = ManualSaveFlowOrigin::None;
          manualProgramController.discardChanges();
          menuController.enterMenu();
          currentScreen = BringupScreen::Menu;
        } else {
          manualSaveFlowOrigin = ManualSaveFlowOrigin::None;
          currentScreen = BringupScreen::ManualProgram;
        }
      } else {
        manualSaveFlowOrigin = ManualSaveFlowOrigin::None;
        currentScreen = BringupScreen::ManualProgram;
      }
      return;
    }

    if (currentScreen == BringupScreen::UserProfileSlots) {
      const dehydrator::UserProfileSlotResult result =
          userProfileSlotController.onShortPress();
      if (result.exitRequested) {
        currentScreen = userProfileSlotScreenPurpose ==
                                UserProfileSlotScreenPurpose::Save
                            ? BringupScreen::ManualProgram
                            : BringupScreen::Menu;
        if (currentScreen == BringupScreen::Menu) {
          menuController.enterMenu();
        }
        return;
      }

      if (!result.slotSelected) {
        return;
      }

      pendingProfileSlot = userProfileSlotController.currentSlot();
      if (userProfileSlotScreenPurpose == UserProfileSlotScreenPurpose::Browse) {
        openUserProfileDetail(pendingProfileSlot);
        return;
      }

      if (isOccupiedUserProfileSlot(pendingProfileSlot)) {
        binaryConfirmPurpose = BinaryConfirmPurpose::OverwriteSlot;
        confirmReplaceRunController.reset();
        currentScreen = BringupScreen::BinaryConfirm;
      } else {
        saveManualProfileToSlot(pendingProfileSlot);
      }
      return;
    }

    if (currentScreen == BringupScreen::UserProfileDetail) {
      const dehydrator::UserProfileActionResult result =
          userProfileActionController.onShortPress();
      if (result.startRequested &&
          isOccupiedUserProfileSlot(activeProfileDetailSlot)) {
        const dehydrator::ProfileConfig profile =
            userProfileSlots[activeProfileDetailSlot].profile;
        if (presetRunController.snapshot().state == dehydrator::RunState::Idle) {
          if (presetRunController.startProfile(profile, "user_profile")) {
            activeProgramDisplay.kind = ActiveProgramDisplayKind::UserProfile;
            activeProgramDisplay.userProfileNumber =
                static_cast<uint8_t>(activeProfileDetailSlot + 1U);
            activeOutputCommand = presetRunController.outputCommand();
            relayOutputs.apply(activeOutputCommand);
            resetStatusPage();
            logEvent("run_profile", "started");
            logEvent("run_state", presetRunController.stateToken());
            menuController.returnToStatus();
            currentScreen = BringupScreen::Status;
          }
        } else {
          pendingPresetSelection = nullptr;
          pendingManualProgramStart = true;
          pendingManualProgramProfile = profile;
          pendingRunDisplay.kind = ActiveProgramDisplayKind::UserProfile;
          pendingRunDisplay.userProfileNumber =
              static_cast<uint8_t>(activeProfileDetailSlot + 1U);
          replaceRunCancelScreen = BringupScreen::UserProfileDetail;
          confirmReplaceRunController.reset();
          binaryConfirmPurpose = BinaryConfirmPurpose::ReplaceRun;
          currentScreen = BringupScreen::ConfirmReplaceRun;
        }
      } else if (result.editRequested) {
        if (isOccupiedUserProfileSlot(activeProfileDetailSlot)) {
          manualProgramController.loadProfile(
              userProfileSlots[activeProfileDetailSlot].profile, true,
              activeProfileDetailSlot);
        } else {
          manualProgramController.resetToDefaults();
        }
        currentScreen = BringupScreen::ManualProgram;
      } else if (result.deleteRequested &&
                 isOccupiedUserProfileSlot(activeProfileDetailSlot)) {
        pendingProfileSlot = activeProfileDetailSlot;
        binaryConfirmPurpose = BinaryConfirmPurpose::DeleteSlot;
        confirmReplaceRunController.reset();
        currentScreen = BringupScreen::BinaryConfirm;
      } else if (result.exitRequested) {
        currentScreen = BringupScreen::UserProfileSlots;
      }
      return;
    }

    if (currentScreen == BringupScreen::BinaryConfirm) {
      const dehydrator::ConfirmReplaceRunResult result =
          confirmReplaceRunController.onShortPress();
      if (binaryConfirmPurpose == BinaryConfirmPurpose::OverwriteSlot) {
        if (result.confirmed) {
          saveManualProfileToSlot(pendingProfileSlot);
        } else {
          currentScreen = BringupScreen::UserProfileSlots;
        }
      } else if (binaryConfirmPurpose == BinaryConfirmPurpose::StopProgram) {
        if (result.confirmed) {
          if (presetRunController.stopConfirmed()) {
            clearActiveProgramDisplay();
            activeOutputCommand = presetRunController.outputCommand();
            relayOutputs.apply(activeOutputCommand);
            resetStatusPage();
            logEvent("run_stop", "confirmed");
            logEvent("run_state", presetRunController.stateToken());
          } else {
            logEvent("run_stop", "rejected");
          }
          menuController.returnToStatus();
          currentScreen = BringupScreen::Status;
        } else {
          menuController.enterMenu();
          currentScreen = BringupScreen::Menu;
        }
      } else if (binaryConfirmPurpose == BinaryConfirmPurpose::DeleteSlot) {
        if (result.confirmed) {
          if (userProfileStore.clear(pendingProfileSlot)) {
            refreshUserProfileSlots();
            logEvent("profile_delete", "done");
          }
        }
        openUserProfileDetail(pendingProfileSlot);
      } else {
        currentScreen = BringupScreen::ManualProgram;
      }
      binaryConfirmPurpose = BinaryConfirmPurpose::None;
      return;
    }

    if (currentScreen == BringupScreen::ConfirmReplaceRun) {
      const dehydrator::ConfirmReplaceRunResult result =
          confirmReplaceRunController.onShortPress();
      logConfirmReplaceRunResult(result);
      if (result.confirmed && pendingPresetSelection != nullptr) {
        if (presetRunController.stopConfirmed() &&
            presetRunController.startPreset(*pendingPresetSelection)) {
          activeProgramDisplay.kind = ActiveProgramDisplayKind::Preset;
          activeProgramDisplay.presetLabel = pendingPresetSelection->label;
          activeOutputCommand = presetRunController.outputCommand();
          relayOutputs.apply(activeOutputCommand);
          resetStatusPage();
          logEvent("preset_start", pendingPresetSelection->token);
          logEvent("run_state", presetRunController.stateToken());
          pendingPresetSelection = nullptr;
          pendingManualProgramStart = false;
          pendingManualProgramProfile = {};
          pendingRunDisplay = {};
          replaceRunCancelScreen = BringupScreen::Status;
          currentScreen = BringupScreen::Status;
        } else {
          logEvent("preset_start", "rejected");
          pendingPresetSelection = nullptr;
          pendingManualProgramStart = false;
          pendingManualProgramProfile = {};
          pendingRunDisplay = {};
          replaceRunCancelScreen = BringupScreen::Status;
          currentScreen = BringupScreen::Status;
        }
      } else if (result.confirmed && pendingManualProgramStart) {
        if (presetRunController.stopConfirmed() &&
            presetRunController.startProfile(
                pendingManualProgramProfile,
                runTokenForDisplayContext(pendingRunDisplay))) {
          activeProgramDisplay = pendingRunDisplay;
          activeOutputCommand = presetRunController.outputCommand();
          relayOutputs.apply(activeOutputCommand);
          resetStatusPage();
          logEvent(activeProgramDisplay.kind == ActiveProgramDisplayKind::UserProfile
                       ? "run_profile"
                       : "run_manual",
                   "started");
          logEvent("run_state", presetRunController.stateToken());
          pendingManualProgramStart = false;
          pendingManualProgramProfile = {};
          pendingRunDisplay = {};
          replaceRunCancelScreen = BringupScreen::Status;
          currentScreen = BringupScreen::Status;
        } else {
          logEvent(pendingRunDisplay.kind == ActiveProgramDisplayKind::UserProfile
                       ? "run_profile"
                       : "run_manual",
                   "rejected");
          pendingManualProgramStart = false;
          pendingManualProgramProfile = {};
          pendingRunDisplay = {};
          replaceRunCancelScreen = BringupScreen::Status;
          currentScreen = BringupScreen::Status;
        }
      } else {
        pendingPresetSelection = nullptr;
        pendingManualProgramStart = false;
        pendingManualProgramProfile = {};
        pendingRunDisplay = {};
        currentScreen = replaceRunCancelScreen;
        if (currentScreen == BringupScreen::Menu) {
          menuController.enterMenu();
        }
      }
      return;
    }

    if (currentScreen == BringupScreen::SettingsMenu) {
      const dehydrator::SettingsMenuResult result =
          settingsMenuController.onShortPress();
      if (result.openTest) {
        currentScreen = BringupScreen::Test;
        logEvent("ui", "test_open");
      } else if (result.exitToMainMenu) {
        menuController.enterMenu();
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
        if (preset != nullptr) {
          if (presetRunController.snapshot().state == dehydrator::RunState::Idle) {
            if (presetRunController.startPreset(*preset)) {
              activeProgramDisplay.kind = ActiveProgramDisplayKind::Preset;
              activeProgramDisplay.presetLabel = preset->label;
              activeOutputCommand = presetRunController.outputCommand();
              relayOutputs.apply(activeOutputCommand);
              resetStatusPage();
              logEvent("preset_start", preset->token);
              logEvent("run_state", presetRunController.stateToken());
              menuController.returnToStatus();
              currentScreen = BringupScreen::Status;
            } else {
              logEvent("preset_start", "rejected");
            }
          } else {
            pendingPresetSelection = preset;
            pendingManualProgramStart = false;
            pendingManualProgramProfile = {};
            replaceRunCancelScreen = BringupScreen::Preset;
            confirmReplaceRunController.reset();
            logEvent("run_replace", "prompt");
            currentScreen = BringupScreen::ConfirmReplaceRun;
          }
        }
      } else if (result.exitToMenu) {
        menuController.enterMenu();
        currentScreen = BringupScreen::Menu;
      }
      return;
    }

    syncMainMenuContext();
    const dehydrator::UiResult result = menuController.onShortPress();

    if (currentScreen == BringupScreen::Status &&
        presetRunController.snapshot().state ==
            dehydrator::RunState::FinishedAlarm &&
        result.action == dehydrator::UiAction::OpenMenu) {
      if (presetRunController.acknowledgeFinished()) {
        clearActiveProgramDisplay();
        activeOutputCommand = presetRunController.outputCommand();
        relayOutputs.apply(activeOutputCommand);
        resetStatusPage();
        logEvent("run_ack", "finished");
        menuController.returnToStatus();
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
        strcmp(menuController.currentToken(), "program_manual") == 0) {
      menuController.returnToStatus();
      currentScreen = BringupScreen::ManualProgram;
      logEvent("ui", "manual_open");
    } else if (result.action == dehydrator::UiAction::SelectItem &&
               menuController.currentToken() != nullptr &&
               strcmp(menuController.currentToken(), "programe_utilizator") == 0) {
      menuController.returnToStatus();
      openUserProfileSlots(UserProfileSlotScreenPurpose::Browse);
      logEvent("ui", "user_profiles_open");
    } else if (result.action == dehydrator::UiAction::SelectItem &&
               menuController.currentToken() != nullptr &&
               strcmp(menuController.currentToken(), "programe_presetate") == 0) {
      menuController.returnToStatus();
      currentScreen = BringupScreen::Preset;
      logEvent("ui", "preset_open");
    } else if (result.action == dehydrator::UiAction::SelectItem &&
               menuController.currentToken() != nullptr &&
               strcmp(menuController.currentToken(), "setari") == 0) {
      settingsMenuController.reset();
      currentScreen = BringupScreen::SettingsMenu;
      logEvent("ui", "settings_open");
    } else if (result.action == dehydrator::UiAction::SelectItem &&
               menuController.currentToken() != nullptr &&
               strcmp(menuController.currentToken(), "pauza_program") == 0) {
      if (presetRunController.pause()) {
        activeOutputCommand = presetRunController.outputCommand();
        relayOutputs.apply(activeOutputCommand);
        resetStatusPage();
        logEvent("run_pause", "confirmed");
        logEvent("run_state", presetRunController.stateToken());
        menuController.returnToStatus();
        currentScreen = BringupScreen::Status;
      } else {
        logEvent("run_pause", "rejected");
      }
    } else if (result.action == dehydrator::UiAction::SelectItem &&
               menuController.currentToken() != nullptr &&
               strcmp(menuController.currentToken(), "reluare_program") == 0) {
      if (presetRunController.resume()) {
        activeOutputCommand = presetRunController.outputCommand();
        relayOutputs.apply(activeOutputCommand);
        resetStatusPage();
        logEvent("run_resume", "confirmed");
        logEvent("run_state", presetRunController.stateToken());
        menuController.returnToStatus();
        currentScreen = BringupScreen::Status;
      } else {
        logEvent("run_resume", "rejected");
      }
    } else if (result.action == dehydrator::UiAction::SelectItem &&
               menuController.currentToken() != nullptr &&
               strcmp(menuController.currentToken(), "oprire_program") == 0) {
      binaryConfirmPurpose = BinaryConfirmPurpose::StopProgram;
      confirmReplaceRunController.reset();
      currentScreen = BringupScreen::BinaryConfirm;
    } else if (result.action == dehydrator::UiAction::CloseMenu) {
      menuController.returnToStatus();
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

  char line[dehydrator::config::LOGGING.lineSize] = {};
  if (dehydrator::LogFormatter::formatBringupState(
          line, sizeof(line), nowMs, ledOn, latestNtc.valid,
          latestNtc.tempC, latestNtc.adcCount, latestTempRh.valid,
          latestTempRh.tempC, latestTempRh.rhPercent,
          presetRunController.stateToken(), presetRunController.activeRunToken(),
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
  writeBootSplash();

  if (tempRhDriver.begin()) {
    logEvent("sensor", "temp_rh_ready");
  } else {
    logEvent("sensor", "temp_rh_missing");
  }

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.createChar(dehydrator::LcdStatusView::PLAY_CHAR, playGlyph);
  lcd.createChar(dehydrator::LcdStatusView::PAUSE_CHAR, pauseGlyph);
  relayOutputs.begin();
  lastEncoderPosition = rotaryEncoder.read();
  encoderStepFilter.reset(lastEncoderPosition);

  latestNtc = ntcReader.read();
  latestTempRh = tempRhReader.read();
  refreshUserProfileSlots();

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
  logSelfCheckVerdict("config", startupInput.configValid);
  logSelfCheckVerdict("outputs", startupInput.outputsSafe);
  logSelfCheckVerdict("ntc", startupInput.primarySensorValid);
  logSelfCheckVerdict("button", startupInput.buttonSafe);
  logSelfCheckStatus("watchdog",
                     startupInput.watchdogResetDuringRun ? "during_run"
                                                         : "clear");
  logSelfCheckStatus("resume", startupInput.interruptedRunAvailable
                                   ? "available"
                                   : "none");
  logEvent("run_state", controlStateMachine.stateToken());
  logEvent("self_check", startupResult.passed ? "passed" : "failed");
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

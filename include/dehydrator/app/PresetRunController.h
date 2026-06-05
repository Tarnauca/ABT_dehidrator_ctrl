#pragma once

#include <stdint.h>

#include "dehydrator/config/RuntimeConfig.h"
#include "dehydrator/domain/ProfileEngine.h"
#include "dehydrator/domain/RunState.h"
#include "dehydrator/domain/RunStateMachine.h"
#include "dehydrator/domain/TemperatureControl.h"
#include "dehydrator/interfaces/OutputController.h"
#include "dehydrator/presets/PresetCatalog.h"

namespace dehydrator {

/**
 * @brief Pure coordinator that turns a selected preset into a live run shell.
 *
 * This class bridges the existing pure modules:
 * - `RunStateMachine` for lifecycle ownership,
 * - `ProfileEngine` for target generation,
 * - `TemperatureControl` for relay-safe heater decisions.
 *
 * It deliberately stays allocation-free and hardware-independent so native
 * tests can cover the first preset-to-run vertical slice before bench upload.
 */
class PresetRunController {
 public:
  /**
   * @brief Creates a preset-run controller with the provided control config.
   *
   * @param controlConfig Hysteresis and relay timing configuration.
   */
  explicit constexpr PresetRunController(
      const config::ControlConfig& controlConfig = config::CONTROL)
      : controlConfig_(controlConfig) {}

  /**
   * @brief Starts a run from one built-in preset.
   *
   * @param preset Preset definition chosen by the user.
   * @return true when the run was accepted from idle state.
   */
  bool startPreset(const PresetDefinition& preset) {
    if (!startProfile(preset.profile, preset.token)) {
      return false;
    }

    activePreset_ = &preset;
    return true;
  }

  /**
   * @brief Starts a run from a non-preset profile, such as manual mode.
   *
   * @param profile Profile configuration to run.
   * @param runToken Stable ASCII token for logs.
   * @return true when the run was accepted from idle state.
   */
  bool startProfile(const ProfileConfig& profile, const char* runToken) {
    if (!stateMachine_.start(profile)) {
      return false;
    }

    activePreset_ = nullptr;
    activeProfile_ = profile;
    activeProfileValid_ = true;
    activeRunToken_ = runToken;
    lastTargetTempC_ = initialTargetTempC(profile);
    currentCommand_ = {};
    temperatureControl_.reset(false);
    updateCommand(0U, false, 0);
    return true;
  }

  /**
   * @brief Advances the active run using the latest primary thermistor reading.
   *
   * @param deltaSeconds Scheduler-provided elapsed time.
   * @param ntcValid Whether the primary thermistor reading may be used for control.
   * @param ntcTempC Latest primary thermistor temperature in Celsius.
   */
  void update(uint16_t deltaSeconds, bool ntcValid, int16_t ntcTempC) {
    stateMachine_.update(deltaSeconds);
    updateCommand(deltaSeconds, ntcValid, ntcTempC);
  }

  /**
   * @brief Acknowledges the finish alarm and returns to idle.
   *
   * @return true when the finish alarm was acknowledged.
   */
  bool acknowledgeFinished() {
    if (!stateMachine_.acknowledgeFinished()) {
      return false;
    }

    clearRunContext();
    return true;
  }

  /**
   * @brief Acknowledges a hard fault and returns to idle.
   *
   * @return true when the fault was acknowledged.
   */
  bool acknowledgeFault() {
    if (!stateMachine_.acknowledgeFault()) {
      return false;
    }

    clearRunContext();
    return true;
  }

  /**
   * @brief Applies a confirmed user stop and returns to idle immediately.
   *
   * @return true when an active lifecycle state was stopped.
   */
  bool stopConfirmed() {
    if (!stateMachine_.stopConfirmed()) {
      return false;
    }

    clearRunContext();
    return true;
  }

  /**
   * @brief Resumes one previously paused run.
   *
   * @return true when the lifecycle returned to `Running`.
   */
  bool resume() {
    if (!stateMachine_.resume()) {
      return false;
    }

    updateCommand(0U, false, 0);
    return true;
  }

  /**
   * @brief Returns the current logical device output command.
   *
   * @return Latest heater/fan/alarm command.
   */
  constexpr OutputCommand outputCommand() const { return currentCommand_; }

  /**
   * @brief Returns the current lifecycle snapshot.
   *
   * @return Current run timers and state flags.
   */
  RunStateSnapshot snapshot() const { return stateMachine_.snapshot(); }

  /**
   * @brief Returns the active preset, if any.
   *
   * @return Active preset pointer or null when idle.
   */
  constexpr const PresetDefinition* activePreset() const { return activePreset_; }

  /**
   * @brief Returns the current active run token, preset-based or manual.
   *
   * @return Stable ASCII token or null when idle.
   */
  constexpr const char* activeRunToken() const { return activeRunToken_; }

  /**
   * @brief Returns the last profile target temperature used for control.
   *
   * @return Integer target temperature in Celsius.
   */
  constexpr int16_t lastTargetTempC() const { return lastTargetTempC_; }

  /**
   * @brief Returns a stable English token for logs and diagnostics.
   *
   * @return Stable lower-case state token.
   */
  const char* stateToken() const {
    switch (stateMachine_.snapshot().state) {
      case RunState::Running:
        return "running";
      case RunState::Paused:
        return "paused";
      case RunState::FinishCooldown:
        return "finish_cooldown";
      case RunState::FinishedAlarm:
        return "finished_alarm";
      case RunState::Fault:
        return "fault";
      case RunState::Boot:
        return "boot";
      case RunState::SelfCheck:
        return "self_check";
      case RunState::ResumeOffer:
        return "resume_offer";
      case RunState::Stopping:
        return "stopping";
      case RunState::Idle:
      default:
        return "idle";
    }
  }

  /**
   * @brief Returns the Romanian LCD state label for the current run state.
   *
   * @return Compact Romanian state text suitable for the status screen.
   */
  const char* stateLabelRo() const {
    switch (stateMachine_.snapshot().state) {
      case RunState::Running:
        return "RULARE";
      case RunState::Paused:
        return "PAUZA";
      case RunState::FinishCooldown:
        return "RACIRE";
      case RunState::FinishedAlarm:
        return "FINALIZAT";
      case RunState::Fault:
        return "EROARE";
      case RunState::Boot:
        return "PORNIRE";
      case RunState::SelfCheck:
        return "VERIFICARE";
      case RunState::ResumeOffer:
        return "RELUARE";
      case RunState::Stopping:
        return "OPRIRE";
      case RunState::Idle:
      default:
        return "INACTIV";
    }
  }

 private:
  static int16_t initialTargetTempC(const ProfileConfig& profile) {
    return (profile.mode == ProfileMode::Fluctuating ||
            profile.mode == ProfileMode::Boost)
               ? profile.highTempC
               : profile.targetTempC;
  }

  void clearRunContext() {
    activePreset_ = nullptr;
    activeProfile_ = {};
    activeProfileValid_ = false;
    activeRunToken_ = nullptr;
    currentCommand_ = {};
    lastTargetTempC_ = 0;
    temperatureControl_.reset(false);
  }

  void updateCommand(uint16_t deltaSeconds, bool ntcValid, int16_t ntcTempC) {
    const RunStateSnapshot snapshot = stateMachine_.snapshot();
    const RunOutputPolicy runPolicy = stateMachine_.outputPolicy();

    currentCommand_.fanOn = runPolicy.fanOn;
    currentCommand_.heaterOn = false;
    currentCommand_.buzzerOn = runPolicy.finishAlarmOn || runPolicy.faultAlarmOn;
    currentCommand_.backlightOn = true;

    if (snapshot.state != RunState::Running || !activeProfileValid_) {
      if (snapshot.state == RunState::Idle) {
        activePreset_ = nullptr;
        activeProfileValid_ = false;
      }
      temperatureControl_.reset(false);
      return;
    }

    const ProfileTarget target =
        ProfileEngine::evaluate(activeProfile_, snapshot.activeElapsedSeconds);
    lastTargetTempC_ = target.targetTempC;

    if (!ntcValid) {
      temperatureControl_.reset(false);
      return;
    }

    TemperatureControlInput controlInput;
    controlInput.currentTempC = ntcTempC;
    controlInput.targetTempC = target.targetTempC;
    controlInput.runPolicy = runPolicy;
    controlInput.deltaSeconds = deltaSeconds;
    const TemperatureControlOutput controlOutput =
        temperatureControl_.update(controlConfig_, controlInput);
    currentCommand_.heaterOn = controlOutput.heaterOn;
    currentCommand_ = sanitizeOutputCommand(currentCommand_);
  }

  const config::ControlConfig& controlConfig_;
  RunStateMachine stateMachine_;
  TemperatureControl temperatureControl_;
  const PresetDefinition* activePreset_ = nullptr;
  ProfileConfig activeProfile_;
  bool activeProfileValid_ = false;
  const char* activeRunToken_ = nullptr;
  OutputCommand currentCommand_;
  int16_t lastTargetTempC_ = 0;
};

}  // namespace dehydrator

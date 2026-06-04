#pragma once

#include <stdint.h>

#include "dehydrator/domain/RunState.h"

namespace dehydrator {

/**
 * @brief Startup inputs used by the boot/self-check state machine.
 *
 * The controller keeps this structure intentionally small and allocation-free
 * so the startup sequence can be tested without Arduino dependencies.
 */
struct ControlStartupInput {
  /** True when the runtime configuration passed validation. */
  bool configValid = true;
  /** True when the relays/backlight/buzzer are already in a safe OFF state. */
  bool outputsSafe = true;
  /** True when the primary NTC sensor is present and usable. */
  bool primarySensorValid = true;
  /** True when the encoder pushbutton is not latched active at boot. */
  bool buttonSafe = true;
  /** True when boot/reset analysis detected a watchdog reset during a run. */
  bool watchdogResetDuringRun = false;
  /** True when a resume snapshot exists in persistent storage. */
  bool interruptedRunAvailable = false;
};

/**
 * @brief Result of one startup self-check pass.
 */
struct ControlStartupResult {
  /** True when the startup self-check passed without a hard fault. */
  bool passed = true;
  /** True when the startup sequence latched a hard fault. */
  bool hardFault = false;
  /** Stable token describing the reason when `hardFault` is true. */
  const char* faultToken = nullptr;
};

/**
 * @brief Snapshot of the control startup state.
 */
struct ControlStateSnapshot {
  /** Current high-level startup/control state. */
  RunState state = RunState::Boot;
  /** True when startup self-check has been completed successfully. */
  bool startupChecked = false;
  /** True when a resume offer is currently available. */
  bool resumeOfferAvailable = false;
  /** True when a hard fault is latched. */
  bool hardFault = false;
};

/**
 * @brief Pure startup controller for boot, self-check, and resume-offer flow.
 *
 * This controller is intentionally smaller than the run lifecycle state
 * machine. It owns the boot-time self-check phase that verifies configuration,
 * sensor plausibility, button safety, and output safe state before the rest of
 * the application accepts a run.
 */
class ControlStateMachine {
 public:
  /**
   * @brief Creates a controller in the boot state.
   */
  ControlStateMachine() = default;

  /**
   * @brief Marks the beginning of the startup sequence.
   */
  void enterBoot() {
    state_ = RunState::Boot;
    startupChecked_ = false;
    resumeOfferAvailable_ = false;
    hardFault_ = false;
    faultToken_ = nullptr;
  }

  /**
   * @brief Advances from boot into the self-check phase.
   */
  void enterSelfCheck() { state_ = RunState::SelfCheck; }

  /**
   * @brief Runs the startup self-check and updates the current state.
   *
   * Mandatory checks are:
   * - configuration validity
   * - output safe state
   * - primary NTC presence/validity
   * - encoder/button not stuck active
   * - no watchdog reset during active run
   *
   * A detected hard fault latches the controller into `Fault`. A successful
   * check transitions to `ResumeOffer` when a persisted interrupted run is
   * available, otherwise to `Idle`.
   *
   * @param input Startup conditions gathered by the application.
   * @return Summary of the check outcome.
   */
  ControlStartupResult completeSelfCheck(const ControlStartupInput& input) {
    enterSelfCheck();
    startupChecked_ = true;

    if (input.watchdogResetDuringRun) {
      latchFault("watchdog_reset_during_run");
      return result(false);
    }

    if (!input.configValid) {
      latchFault("config_invalid");
      return result(false);
    }

    if (!input.outputsSafe) {
      latchFault("outputs_not_safe");
      return result(false);
    }

    if (!input.primarySensorValid) {
      latchFault("ntc_invalid");
      return result(false);
    }

    if (!input.buttonSafe) {
      latchFault("button_stuck");
      return result(false);
    }

    resumeOfferAvailable_ = input.interruptedRunAvailable;
    state_ = resumeOfferAvailable_ ? RunState::ResumeOffer : RunState::Idle;
    faultToken_ = nullptr;
    hardFault_ = false;
    return result(true);
  }

  /**
   * @brief Accepts the resume-offer branch and returns to idle.
   *
   * This keeps the startup controller future-proof without forcing the current
   * code to restore a run snapshot yet.
   *
   * @return true when a resume offer was dismissed.
   */
  bool dismissResumeOffer() {
    if (state_ != RunState::ResumeOffer) {
      return false;
    }

    resumeOfferAvailable_ = false;
    state_ = RunState::Idle;
    return true;
  }

  /**
   * @brief Acknowledges a startup hard fault and returns to idle.
   *
   * @return true when the hard fault was cleared.
   */
  bool acknowledgeFault() {
    if (!hardFault_) {
      return false;
    }

    state_ = RunState::Idle;
    hardFault_ = false;
    faultToken_ = nullptr;
    return true;
  }

  /**
   * @brief Returns a snapshot of the control startup state.
   *
   * @return Current control startup state and flags.
   */
  ControlStateSnapshot snapshot() const {
    ControlStateSnapshot snapshot;
    snapshot.state = state_;
    snapshot.startupChecked = startupChecked_;
    snapshot.resumeOfferAvailable = resumeOfferAvailable_;
    snapshot.hardFault = hardFault_;
    return snapshot;
  }

  /**
   * @brief Returns a stable English token for logs and diagnostics.
   *
   * @return Stable lower-case startup state token.
   */
  const char* stateToken() const {
    switch (state_) {
      case RunState::Boot:
        return "boot";
      case RunState::SelfCheck:
        return "self_check";
      case RunState::ResumeOffer:
        return "resume_offer";
      case RunState::Fault:
        return "fault";
      case RunState::Idle:
      default:
        return "idle";
    }
  }

  /**
   * @brief Returns the Romanian LCD label for the current startup state.
   *
   * @return Compact Romanian state text suitable for the status screen.
   */
  const char* stateLabelRo() const {
    switch (state_) {
      case RunState::Boot:
        return "PORNIRE";
      case RunState::SelfCheck:
        return "VERIFICARE";
      case RunState::ResumeOffer:
        return "RELUARE";
      case RunState::Fault:
        return "EROARE";
      case RunState::Idle:
      default:
        return "INACTIV";
    }
  }

  /**
   * @brief Returns the fault token for the latched startup error.
   *
   * @return Null when no fault is latched.
   */
  const char* faultToken() const { return faultToken_; }

 private:
  void latchFault(const char* token) {
    state_ = RunState::Fault;
    hardFault_ = true;
    faultToken_ = token;
    resumeOfferAvailable_ = false;
  }

  ControlStartupResult result(bool passed) const {
    ControlStartupResult startupResult;
    startupResult.passed = passed;
    startupResult.hardFault = hardFault_;
    startupResult.faultToken = faultToken_;
    return startupResult;
  }

  RunState state_ = RunState::Boot;
  bool startupChecked_ = false;
  bool resumeOfferAvailable_ = false;
  bool hardFault_ = false;
  const char* faultToken_ = nullptr;
};

}  // namespace dehydrator

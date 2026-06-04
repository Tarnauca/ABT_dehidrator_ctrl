#pragma once

#include <stdint.h>

#include "dehydrator/domain/ProfileEngine.h"
#include "dehydrator/domain/RunState.h"

namespace dehydrator {

/**
 * @brief Output policy requested by the run state machine.
 *
 * This policy describes lifecycle-level output permissions only. Temperature
 * control and safety logic may further restrict heater operation.
 */
struct RunOutputPolicy {
  /** True when the fan must be commanded ON by the application coordinator. */
  bool fanOn = false;
  /** True when heater control is allowed to request heater ON. */
  bool heaterControlAllowed = false;
  /** True while the normal-finish alarm should be active. */
  bool finishAlarmOn = false;
  /** True while the hard-fault alarm should be active. */
  bool faultAlarmOn = false;
};

/**
 * @brief Snapshot of the run state machine for UI, logging, and tests.
 */
struct RunStateSnapshot {
  /** Current high-level runtime state. */
  RunState state = RunState::Idle;
  /** Active profile elapsed time, excluding paused and cooldown time. */
  uint32_t activeElapsedSeconds = 0;
  /** Finish cooldown elapsed time after normal profile completion. */
  uint16_t cooldownElapsedSeconds = 0;
  /** True while the active run can be resumed by the user. */
  bool resumeAllowed = false;
  /** True when the current profile configuration is valid. */
  bool profileValid = false;
};

/**
 * @brief Pure lifecycle state machine for a drying run.
 *
 * `RunStateMachine` owns pause/resume, finish cooldown, normal stop, and hard
 * fault lifecycle transitions. It does not read clocks directly; callers pass
 * elapsed seconds from the cooperative scheduler so native tests can exercise
 * timing without Arduino APIs.
 */
class RunStateMachine {
 public:
  /**
   * @brief Fixed fan cooldown after normal program finish, in seconds.
   */
  static constexpr uint16_t FINISH_COOLDOWN_SECONDS = 3U * 60U;

  /**
   * @brief Creates an idle state machine with no active profile.
   */
  RunStateMachine() = default;

  /**
   * @brief Starts a drying profile if the profile configuration is valid.
   *
   * Starting is accepted only from `Idle`. A valid start clears old elapsed
   * time and enables pause/resume for the active run.
   *
   * @param profile Profile configuration to run.
   * @return true when the run was started.
   */
  bool start(const ProfileConfig& profile) {
    if (state_ != RunState::Idle || !ProfileEngine::isValid(profile)) {
      return false;
    }

    profile_ = profile;
    state_ = RunState::Running;
    activeElapsedSeconds_ = 0;
    cooldownElapsedSeconds_ = 0;
    resumeAllowed_ = true;
    profileValid_ = true;
    return true;
  }

  /**
   * @brief Advances lifecycle timers by a scheduler-provided time delta.
   *
   * Active profile elapsed time advances only while running. Finish cooldown
   * time advances only in `FinishCooldown`.
   *
   * @param deltaSeconds Elapsed scheduler time since the previous update.
   */
  void update(uint16_t deltaSeconds) {
    if (state_ == RunState::Running) {
      activeElapsedSeconds_ += deltaSeconds;

      if (ProfileEngine::evaluate(profile_, activeElapsedSeconds_).complete) {
        state_ = RunState::FinishCooldown;
        cooldownElapsedSeconds_ = 0;
        resumeAllowed_ = false;
      }
      return;
    }

    if (state_ == RunState::FinishCooldown) {
      const uint32_t nextCooldown =
          static_cast<uint32_t>(cooldownElapsedSeconds_) + deltaSeconds;
      cooldownElapsedSeconds_ =
          nextCooldown >= FINISH_COOLDOWN_SECONDS
              ? FINISH_COOLDOWN_SECONDS
              : static_cast<uint16_t>(nextCooldown);

      if (cooldownElapsedSeconds_ >= FINISH_COOLDOWN_SECONDS) {
        state_ = RunState::FinishedAlarm;
      }
    }
  }

  /**
   * @brief Pauses an active run and keeps it resumable.
   *
   * @return true when the state changed to `Paused`.
   */
  bool pause() {
    if (state_ != RunState::Running) {
      return false;
    }

    state_ = RunState::Paused;
    resumeAllowed_ = true;
    return true;
  }

  /**
   * @brief Resumes a paused run from the same active elapsed time.
   *
   * @return true when the state changed to `Running`.
   */
  bool resume() {
    if (state_ != RunState::Paused || !resumeAllowed_) {
      return false;
    }

    state_ = RunState::Running;
    return true;
  }

  /**
   * @brief Applies a confirmed user stop/cancel.
   *
   * Confirmed stop immediately disables resume and returns to idle. No finish
   * cooldown or alarm is started for user stop.
   *
   * @return true when an active lifecycle state was stopped.
   */
  bool stopConfirmed() {
    if (state_ != RunState::Running && state_ != RunState::Paused &&
        state_ != RunState::FinishCooldown && state_ != RunState::FinishedAlarm) {
      return false;
    }

    resetToIdle();
    return true;
  }

  /**
   * @brief Enters hard-fault state from any state.
   *
   * Faults immediately disable resume. The run context may still be logged by
   * another module, but this state machine will not allow resume from fault.
   */
  void fault() {
    state_ = RunState::Fault;
    resumeAllowed_ = false;
  }

  /**
   * @brief Acknowledges an active hard fault and returns to idle.
   *
   * @return true when the fault was acknowledged.
   */
  bool acknowledgeFault() {
    if (state_ != RunState::Fault) {
      return false;
    }

    resetToIdle();
    return true;
  }

  /**
   * @brief Acknowledges the normal finish alarm and returns to idle.
   *
   * @return true when the finish alarm was acknowledged.
   */
  bool acknowledgeFinished() {
    if (state_ != RunState::FinishedAlarm) {
      return false;
    }

    resetToIdle();
    return true;
  }

  /**
   * @brief Returns a lifecycle-level output policy for the current state.
   *
   * @return Output policy before lower-level temperature control is applied.
   */
  RunOutputPolicy outputPolicy() const {
    RunOutputPolicy policy;

    if (state_ == RunState::Running) {
      policy.fanOn = true;
      policy.heaterControlAllowed = true;
    } else if (state_ == RunState::FinishCooldown) {
      policy.fanOn = true;
    } else if (state_ == RunState::FinishedAlarm) {
      policy.finishAlarmOn = true;
    } else if (state_ == RunState::Fault) {
      policy.faultAlarmOn = true;
    }

    return policy;
  }

  /**
   * @brief Returns the current state machine snapshot.
   *
   * @return State, timers, and resumability flags.
   */
  RunStateSnapshot snapshot() const {
    RunStateSnapshot snapshot;
    snapshot.state = state_;
    snapshot.activeElapsedSeconds = activeElapsedSeconds_;
    snapshot.cooldownElapsedSeconds = cooldownElapsedSeconds_;
    snapshot.resumeAllowed = resumeAllowed_;
    snapshot.profileValid = profileValid_;
    return snapshot;
  }

 private:
  void resetToIdle() {
    state_ = RunState::Idle;
    activeElapsedSeconds_ = 0;
    cooldownElapsedSeconds_ = 0;
    resumeAllowed_ = false;
    profileValid_ = false;
  }

  RunState state_ = RunState::Idle;
  ProfileConfig profile_;
  uint32_t activeElapsedSeconds_ = 0;
  uint16_t cooldownElapsedSeconds_ = 0;
  bool resumeAllowed_ = false;
  bool profileValid_ = false;
};

}  // namespace dehydrator

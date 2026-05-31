#pragma once

namespace dehydrator {

/**
 * @brief High-level runtime states owned by the control state machine.
 *
 * These states describe the controller's operating mode independently from
 * the low-level relay, buzzer, backlight, LCD, and serial hardware adapters.
 */
enum class RunState {
  /** Outputs are commanded to a safe state before initialization continues. */
  Boot,
  /** Startup checks are running before the controller can accept a run. */
  SelfCheck,
  /** No active run is present and heater/fan are expected to be off. */
  Idle,
  /** A valid interrupted run exists and the user may resume or discard it. */
  ResumeOffer,
  /** A drying program is active. Fan is expected on; heater is controlled. */
  Running,
  /** The active run is suspended and heater/fan are expected to be off. */
  Paused,
  /** A confirmed user stop is being applied and resume is disabled. */
  Stopping,
  /** Normal finish cooldown is active with heater off and fan on. */
  FinishCooldown,
  /** Finish cooldown is complete and the finish alarm is active. */
  FinishedAlarm,
  /** A hard fault is active and acknowledgement is required. */
  Fault,
};

/**
 * @brief Returns whether the high-level state expects heater and fan off.
 *
 * This helper describes heater/fan safety expectations only. Alarm outputs
 * such as the buzzer or LCD backlight may still be active in states like
 * `FinishedAlarm` or `Fault`.
 *
 * @param state High-level runtime state to evaluate.
 * @return true when heater and fan should normally be commanded off.
 */
constexpr bool isHeaterFanOffState(RunState state) {
  return state == RunState::Boot || state == RunState::Idle ||
         state == RunState::ResumeOffer || state == RunState::Paused ||
         state == RunState::Stopping || state == RunState::FinishedAlarm ||
         state == RunState::Fault;
}

}  // namespace dehydrator

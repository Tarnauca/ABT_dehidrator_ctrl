# ADR-005: Control And Safety Baseline

## Status

Accepted.

## Context

The dehydrator controls heat and airflow. The device must operate safely with relay-based on/off heater and fan control.

## Decision

Use simple hysteresis control with relay minimum on/off timing.

Safety baseline:

- PT50 is the primary control sensor.
- AHT-like sensor is secondary telemetry and warning-only if unavailable.
- User setpoint maximum is 75 C.
- Heater is forced OFF above 75 C.
- Hard over-temperature fault occurs at 80 C.
- Heater must never be ON unless fan is ON.
- Hard faults turn heater and fan OFF immediately and require acknowledgement.
- Finish runs fan cooldown for 3 minutes, then alarms.
- Pause and user stop turn heater and fan OFF immediately.
- Resume is allowed after pause or confirmed interrupted run, but not after hard fault.

## Consequences

- Control is simple and explainable.
- Mechanical relay wear is reduced by minimum on/off timing.
- Product-minded independent thermal protection is still recommended.
- Some thresholds may need tuning after bench testing.

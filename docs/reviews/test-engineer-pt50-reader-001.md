# Test Engineer Review 001: PT50 Reader And Calibration

Date: 2026-06-02

Scope:
- `include/dehydrator/domain/Pt50SensorModel.h`
- `include/dehydrator/sensors/Pt50Reader.h`
- `include/dehydrator/interfaces/AnalogInput.h`
- `include/dehydrator/config/RuntimeConfig.h`
- `test/test_pt50_sensor/test_pt50_sensor.cpp`
- `test/test_config/test_config.cpp`
- `docs/backlog.md`
- `docs/hardware.md`
- `docs/test-plan.md`
- `docs/logbook.md`

## Summary

The PT50 conversion and interface-based reader structure was good, but the
first implementation narrowed the calculated temperature to `int16_t` before
checking plausible range. Near-rail ADC values could therefore wrap into a
valid-looking temperature.

## Findings

### High: near-rail ADC values could wrap into valid-looking temperature

The first implementation converted the calibrated temperature to `int16_t`
before validity checking. With the default fixed-high divider, near-maximum ADC
counts can represent extremely high, invalid temperatures that overflow
`int16_t` and wrap into the configured plausible range.

Resolution:
- Changed the model to keep calibrated Celsius in a wide integer type during
  plausibility checks.
- Assign `Pt50Reading::tempC` only after the wide value is confirmed plausible
  and within `int16_t` range.
- Added near-rail ADC tests to prevent regression.

### Medium: near-rail ADC tests were missing

The first tests covered `0` and `adcMaxCount`, but not counts close enough to
the rails to expose overflow or implausible resistance.

Resolution:
- Added a default-orientation near-max ADC invalid test.
- Added opposite-orientation near-zero ADC invalid test.

### Low: opposite divider orientation coverage was nominal only

The first tests only covered the opposite divider orientation near 0 C.

Resolution:
- Added a high-temperature direction test for `Pt50HighFixedLow`.

### Low: backlog wording could overclaim concrete Arduino adapter completion

The branch adds a pure reader and `AnalogInput` interface, but not a concrete
Arduino `analogRead()` adapter.

Resolution:
- Split the backlog into completed interface-based model/reader work and a
  pending concrete Arduino PT50 adapter after wiring is confirmed.
- Hardware docs now state the concrete adapter remains pending.

## Verification

After applying review findings:
- `platformio test -e native`: 106 tests passed
- `platformio run -e megaatmega2560`: success

## Verdict

Ready to commit after review findings were fixed and verification passed.

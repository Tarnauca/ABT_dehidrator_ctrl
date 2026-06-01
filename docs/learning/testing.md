# Testing Learning Notes

## Native Tests

Native tests run on the development machine rather than the Mega2560. They are
best for pure C++ logic that does not include Arduino-specific headers.

Good native-test targets:

- state transitions
- timing helpers
- log formatting
- command sanitization
- fault detection
- profile calculations
- persistence validation helpers

## Fakes

Fakes let tests simulate hardware behavior without a board connected.

Useful fakes:

- `FakeClock`
- `FakeSensorReader`
- `FakeOutputController`
- `FakePersistentStore`
- `FakeLogSink`

## Bench Tests

Bench tests run on the real board or connected hardware. Use them for behavior
that depends on actual pins, relays, sensors, LCDs, buzzers, encoders, or serial
ports.

Before connecting heater power, bench tests should verify low-risk outputs and
sensor behavior using safe conditions.

## Tests As Documentation

Tests are executable documentation. A test such as “heater is forced off when
fan is off” makes a safety rule concrete and repeatable.

When a requirement changes, update the relevant tests and documentation in the
same branch.

# Architecture Decision Learning Notes

## Why Group Hardware Pins Into A Struct?

The hardware pins were grouped into `HardwarePins` instead of leaving each pin
as an unrelated top-level constant.

This gives several benefits:

- Pin assignments are clearly separated from other hardware assumptions such as
  relay polarity and LCD I2C address.
- Future hardware adapter constructors can receive coherent configuration
  groups instead of long lists of unrelated parameters.
- Wiring changes have one obvious place to inspect.
- The code scales better as more signals are added: primary thermistor analog input, heater
  relay, fan relay, LCD backlight, buzzer, encoder A/B/button, and future
  diagnostic pins.

Example:

```cpp
struct HardwareConfig {
  HardwarePins pins;
  ActiveLevel heaterRelayActiveLevel;
  ActiveLevel fanRelayActiveLevel;
  uint8_t lcdI2cAddress;
};
```

This is not the only valid design. Flat constants are simpler at the very
beginning:

```cpp
constexpr uint8_t HEATER_RELAY_PIN = 255;
constexpr uint8_t FAN_RELAY_PIN = 255;
```

But this project has enough hardware surfaces that grouping the related pins is
useful without becoming a heavy abstraction.

## Logging Sink Versus Future Protocol Adapter

The current logging design sends the same structured log line to multiple
`LogSink` destinations. This is log mirroring:

```text
LogFormatter -> LogDispatcher -> USB Serial + Serial1
```

That matches the current requirement: secondary serial mirrors the terminal
output during bring-up.

If `Serial1` later becomes Modbus RTU or another gateway protocol, it should not
remain a `LogSink`. Modbus RTU has different framing, timing, addressing, CRC,
and request/response behavior. It should be a separate protocol adapter.

The logging abstraction is therefore intentionally limited:

- Good: structured logs, tests, mirrored bring-up output.
- Not intended: future telemetry protocol abstraction.

## Interfaces At Hardware Boundaries

Interfaces are useful where the firmware crosses from testable logic into
hardware-specific behavior.

Good interface candidates:

- clock/time
- outputs
- sensors
- display
- persistent store
- log sinks

Avoid creating abstractions only to make the code look architectural. Use them
when they reduce coupling, enable tests, or isolate replaceable libraries.

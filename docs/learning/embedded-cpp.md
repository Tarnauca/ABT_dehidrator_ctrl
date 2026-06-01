# Embedded C++ Learning Notes

## Doxygen Comments In Public Headers

Public headers should document public types, functions, parameters, return
values, units, safety assumptions, timing assumptions, and side effects.

Names matter, but names alone are not enough for safety-relevant code.

Example:

```cpp
/**
 * @brief Returns whether the high-level state expects heater and fan off.
 *
 * @param state High-level runtime state to evaluate.
 * @return true when heater and fan should normally be commanded off.
 */
constexpr bool isHeaterFanOffState(RunState state);
```

## Avoid Dynamic Allocation

Application code should avoid dynamic allocation on the Mega2560.

Avoid:

- `new`
- `delete`
- `malloc`
- `free`
- Arduino `String`
- dynamically growing containers

Prefer:

- fixed-size buffers
- string literals
- `snprintf`
- static/global/stack objects with clear lifetime

## Arduino `String`

Arduino `String` uses heap allocation internally. On AVR this can fragment RAM
over long runtimes, especially with repeated concatenation or resizing.

Use fixed buffers and explicit formatting instead.

## Non-Blocking Timing

Avoid long `delay()` calls in normal runtime logic. Use `millis()`-based timing
through small helpers such as `PeriodicTask`.

This keeps safety checks, UI updates, heartbeat, serial logs, and input handling
responsive.

## Arduino Macro Collisions

Arduino headers define some short macro names. One example encountered in this
project was `SERIAL`.

Using a config constant named `SERIAL` compiled in native tests but failed in
the Mega2560 build because Arduino defines `SERIAL` as a macro.

Lesson: prefer more specific names for public constants, such as
`SERIAL_PORTS`.

# Hardware

Status: draft baseline. Pin assignments are TBD until wiring is finalized.

## Target

- Board: Arduino Mega2560.
- Framework: Arduino through PlatformIO.

## Interface Table

| Signal | Purpose | Mega2560 Pin | Active Level | Notes |
| --- | --- | --- | --- | --- |
| PT50 analog | Primary temperature control sensor | TBD analog | N/A | Voltage divider into ADC, supplied from VCC |
| AHT-like sensor SDA/SCL | Secondary temp/RH telemetry | TBD I2C | N/A | Warning only if unavailable |
| Heater relay | Heater control | TBD | Configurable | Mechanical relay, on/off only |
| Fan relay | Fan control | TBD | Configurable | Mechanical relay, on/off only |
| LCD I2C | 4x20 display | TBD I2C | N/A | I2C-to-parallel backpack |
| Encoder A/B | Navigation/change values | TBD | TBD | Debounced in firmware |
| Encoder button | Select/back/cancel | TBD | TBD | Short/long press; stuck detection |
| Piezo buzzer | Finish/fault alarm | TBD | TBD | No routine button beep |
| Backlight FET | LCD backlight blink/control | TBD | TBD | Dedicated MCU output |
| USB Serial | Debug log | USB | N/A | 115200 8N1 |
| Secondary Serial | Mirrored telemetry log | TBD HW serial | N/A | 115200 8N1, output-only, possible IrDA |

## Sensor Notes

PT50 is the primary control sensor. It is currently expected to be measured through a voltage divider into an analog input using MCU VCC as supply/reference.

The firmware contains a ratiometric PT50 conversion model with configurable divider orientation, fixed resistor value, PT50 nominal resistance, alpha, offset, scale, and plausible temperature range. Default values are placeholders and must be reviewed against the real divider before heater testing:

- fixed resistor: 100 ohm,
- PT50 nominal resistance: 50 ohm at 0 C,
- alpha: 3850 ppm/C,
- ADC range: 0..1023,
- default divider orientation: fixed resistor high side, PT50 low side,
- plausible converted PT50 range: -20 C..120 C.

The current branch adds a project-owned `AnalogInput` interface and a PT50 reader that can consume raw ADC counts through that interface. A concrete Arduino `analogRead()` adapter is still pending until wiring is confirmed.

Product-minded recommendation: consider a precision reference, calibration procedure, ratiometric review, or RTD interface IC for improved measurement accuracy.

The AHT-like sensor provides secondary temperature and RH. Its failure is a warning only and must not stop drying unless a future feature depends on RH.

## Output Notes

Relay polarity must be configurable. Application logic should command logical `heater ON/OFF` and `fan ON/OFF`; only the hardware adapter should translate logical output into pin level.

The firmware contains a relay output adapter with configurable active-high or active-low relay polarity. It defensively sanitizes unsafe commands so heater cannot remain ON when fan is OFF, writes fan ON before heater ON, and writes heater OFF before fan OFF. Startup configuration preloads inactive relay levels before enabling output mode through the project `DigitalOutput` interface. Placeholder relay pins are ignored until real wiring is assigned, and heater ON is blocked if the fan relay pin is unassigned.

The firmware also contains a buzzer/backlight alarm output adapter with configurable active-high or active-low polarity. It preloads inactive output levels during startup and ignores placeholder pins until real wiring is assigned.

Product-minded recommendation: use independent hardware thermal protection in the heater power path.

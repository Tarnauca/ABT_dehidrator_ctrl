# Hardware

Status: draft baseline. Heater/fan pin assignments are TBD until wiring is finalized. UI pins use temporary bench defaults.

## Target

- Board: Arduino Mega2560.
- Framework: Arduino through PlatformIO.

## Interface Table

| Signal | Purpose | Mega2560 Pin | Active Level | Notes |
| --- | --- | --- | --- | --- |
| PT50 analog | Primary temperature control sensor | A0 placeholder | N/A | Voltage divider into ADC, supplied from VCC |
| AHT-like sensor SDA/SCL | Secondary temp/RH telemetry | Mega I2C SDA/SCL | N/A | Warning only if unavailable |
| Heater relay | Heater control | TBD | Configurable | Mechanical relay, on/off only |
| Fan relay | Fan control | TBD | Configurable | Mechanical relay, on/off only |
| LCD I2C | 4x20 display | Mega I2C SDA/SCL | N/A | `LiquidCrystal_I2C`, default address `0x27` |
| Encoder A/B | Navigation/change values | D2/D3 temporary | Pull-up/input library dependent | `Encoder` library |
| Encoder button | Select/confirm | D4 temporary | Active low with `INPUT_PULLUP` | `Bounce2`; short press is active behavior, long press currently reserved |
| Piezo buzzer | Finish/fault alarm | D8 temporary | Configurable, default active high | No routine button beep |
| Backlight FET | LCD backlight blink/control | D7 temporary | Configurable, default active high | Dedicated MCU output |
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

The firmware contains a project-owned `AnalogInput` interface, a PT50 reader
that consumes raw ADC counts through that interface, and a concrete Arduino
`analogRead()` adapter for hardware bring-up. The default analog channel is A0
placeholder. Real divider wiring and calibration still need bench confirmation
before the PT50 value is used for heater decisions.

Product-minded recommendation: consider a precision reference, calibration procedure, ratiometric review, or RTD interface IC for improved measurement accuracy.

The AHT-like sensor provides secondary temperature and RH. Its failure is a warning only and must not stop drying unless a future feature depends on RH.

The firmware contains an interface-based AHT reader that accepts centi-Celsius
and centi-percent RH samples, applies optional calibration offsets, and rejects
implausible temperature/RH values.

The current concrete Arduino adapter uses the proven `Adafruit_AHTX0` library
on the shared I2C bus and converts its floating-point readings into the fixed
centi-units used by the project. If the sensor is missing or incompatible,
initialization fails, RH stays unavailable on the LCD, and the serial log emits
`EVENT type=sensor detail=aht_missing`.

## Output Notes

Relay polarity must be configurable. Application logic should command logical `heater ON/OFF` and `fan ON/OFF`; only the hardware adapter should translate logical output into pin level.

The firmware contains a relay output adapter with configurable active-high or active-low relay polarity. It defensively sanitizes unsafe commands so heater cannot remain ON when fan is OFF, writes fan ON before heater ON, and writes heater OFF before fan OFF. Startup configuration preloads inactive relay levels before enabling output mode through the project `DigitalOutput` interface. Placeholder relay pins are ignored until real wiring is assigned, and heater ON is blocked if the fan relay pin is unassigned.

The firmware also contains a buzzer/backlight alarm output adapter with configurable active-high or active-low polarity. It preloads inactive output levels during startup and ignores placeholder pins until real wiring is assigned.

Product-minded recommendation: use independent hardware thermal protection in the heater power path.

## UI Bring-Up Notes

The firmware now includes a first hardware bring-up screen for the 4x20 LCD.
It uses `LiquidCrystal_I2C` for the HD44780 I2C backpack, a project-owned
`CharacterDisplay` interface, and a testable `LcdStatusView` renderer. The
current screen is intentionally minimal:

```text
Stare: INACTIV
T:--C    RH:--%
H:OFF     F:OFF
                   <heartbeat>
```

The heartbeat is a custom LCD character in the bottom-right cell. The renderer
redraws fixed-width lines so shorter values do not leave stale characters.
When the AHT sensor is present and initializes correctly, `RH:--%` is replaced
by the live humidity percentage.

Encoder rotation is read through the proven `Encoder` library and the
pushbutton is debounced through `Bounce2`. In this slice, input events are only
logged:

- `EVENT type=input detail=encoder_cw`
- `EVENT type=input detail=encoder_ccw`
- `EVENT type=input detail=button_short`
- `EVENT type=input detail=button_long`

The default UI pin assignments are temporary bench values and should be edited
in `include/dehydrator/config/HardwareConfig.h` to match the real wiring.

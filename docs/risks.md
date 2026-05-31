# Risk Log

Status: draft baseline. Update this file when hardware assumptions, safety behavior, or control strategy changes.

| ID | Risk | Severity | Likelihood | Mitigation | Status |
| --- | --- | --- | --- | --- | --- |
| RISK-001 | Over-temperature/fire risk | High | Medium | 75 C setpoint limit, heater OFF above 75 C, hard fault at 80 C, independent thermal cutoff recommended | Open |
| RISK-002 | PT50 failure causes unsafe control | High | Medium | PT50 invalid/out-of-range is hard fault; startup plausibility check | Open |
| RISK-003 | Heater relay stuck ON | High | Low/Medium | After 2 min post-heater-off grace period, detect PT50 rise of 3 C over 5 min while heater command OFF; hard fault; independent cutoff recommended | Open |
| RISK-004 | Temperature not rising while heater ON | Medium | Medium | Hard fault if less than 2 C rise within 5 min of accumulated heater ON command time | Open |
| RISK-005 | Fan failure | High | Unknown | Not detectable with current hardware; product-minded recommendation to add feedback if needed | Accepted for current scope |
| RISK-006 | EEPROM wear | Medium | Medium | Write only on meaningful changes/checkpoints; minimum 15 min run snapshot interval | Open |
| RISK-007 | EEPROM corruption | Low/Medium | Medium | Version/checksum; load defaults and warn | Open |
| RISK-008 | Watchdog reset during run | High | Low/Medium | Outputs OFF on boot; record reset cause; require acknowledgement/no auto-resume | Open |
| RISK-009 | Long blocking code delays safety checks | High | Medium | Cooperative scheduler; avoid blocking delays over 100 ms in runtime logic | Open |
| RISK-010 | Dynamic allocation fragments SRAM | Medium | Medium | No dynamic allocation in application code; avoid Arduino `String` | Open |
| RISK-011 | LCD text does not fit 4x20 | Low | Medium | UI documented before implementation; compact Romanian ASCII labels | Open |
| RISK-012 | Inaccurate PT50 due to VCC reference/voltage divider | Medium | Medium | Calibration constants, filtering, debug visibility; consider better reference/RTD interface | Open |
| RISK-013 | Relay chatter/wear | Medium | Medium | Hysteresis plus 10 s minimum ON/OFF times | Open |
| RISK-014 | Secondary telemetry misinterpreted as control channel | Medium | Low | Output-only requirement; no command parser in baseline | Open |
| RISK-015 | EEPROM wear from reset-cause writes during reset loop | Medium | Low/Medium | Record reset cause carefully; avoid repeated boot writes when reset cause/event has not meaningfully changed | Open |

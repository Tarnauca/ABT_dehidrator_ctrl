# Project Discovery Log

This log preserves the discovery conversation that established the initial project baseline. It is intentionally more detailed than `docs/project-context.md` so the reasoning process remains available after a devcontainer rebuild or Codex session loss.

## Initial Goal

The user wants to develop a food dehydrator controller and learn agentic software development. The goals include proper requirements definition, better architecture, documentation, coding, testing, and safety review.

## Q&A And Decisions

1. Target platform started as Arduino Uno/Nano, then changed to Arduino Mega2560 because squeezing the design into ATmega328 is unnecessary overhead. Uno/Nano compatibility is dropped from scope.
2. Toolchain is PlatformIO.
3. Existing environment is a working devcontainer in WSL. PlatformIO can build/upload a basic blink project. Serial port is attached to the devcontainer. SSH agent is available from WSL inside the container, and Git commits can be made from the container.
4. Initial hardware:
   - PT50 as primary temperature sensor.
   - AHT21-like temperature/RH sensor as secondary sensor.
   - Heater and fan controlled by existing relays, on/off only.
   - 4x20 LCD over typical I2C-to-parallel interface.
   - Encoder with pushbutton.
   - Piezo buzzer.
   - MCU EEPROM only for storage.
5. Project is hobby use but should consider product use where recommendations are helpful. It is not a proof of concept; the device must work safely.
6. Draft requirements are not yet fully formulated. The user will add them later.
7. Agents should support advising/reviewing, documentation, firmware code, builds/tests, development environment review, architecture, requirements, and safety.
8. Process formality should be medium, with professional software development practices but without excessive ceremony.
9. Primary UI is LCD plus encoder. USB serial debug is mandatory. A future gateway-like device over a second serial interface is considered.
10. Remote monitoring/control is not a current hard requirement. External interface should be reporting only, not control.
11. Top agentic-development priorities:
    - proper requirements definition,
    - better architecture,
    - documentation,
    - coding,
    - testing and unit testing,
    - safety aspects.
12. Operating modes:
    - manual on/off,
    - fixed temperature and duration,
    - fixed average temperature with fluctuations,
    - future experimental humidity-based stop.
13. Pause/resume and presets are required.
14. Finish alarm uses buzzer and blinking LCD backlight.
15. Maximum user setpoint is 75 C. Maximum duration is 99 hours.
16. Faults stop the device.
17. Fan behavior:
    - fan is always on during a run,
    - heater is controlled separately,
    - no heater operation without fan ON,
    - finish cooldown runs fan for 3 minutes after heater is OFF,
    - pause turns heater and fan OFF immediately,
    - user stop turns heater and fan OFF immediately.
18. Pause behavior:
    - heater OFF,
    - fan OFF immediately,
    - timer/profile suspended,
    - resume continues same profile from paused point.
19. PT50 is the primary control sensor. AHT temperature/RH is secondary telemetry and plausibility checking.
20. PT50/AHT temperature mismatch is warning only, not fault.
21. Hard faults:
    - PT50 missing/invalid/out of range,
    - measured PT50 at/above 80 C,
    - temperature not rising while heater ON,
    - encoder/button stuck,
    - suspected heater stuck ON,
    - watchdog reset during active run.
22. EEPROM/config corruption is not a hard fault; use defaults and warn.
23. Fan failure cannot currently be detected.
24. After a hard fault:
    - heater OFF immediately,
    - fan OFF immediately,
    - show fault,
    - buzzer/backlight alarm,
    - require acknowledgement.
25. After power loss/reset:
    - boot with outputs OFF,
    - if interrupted run exists, offer resume only after user confirmation.
26. Presets are built-in only. User may later provide a similar product manual PDF for preset values.
27. Presets should include structured profile details, such as `Apple: fluctuating mode, average 57 C, cycle 50-65 C, 10 h`.
28. Fluctuating mode initial algorithm should be simple:
    - low temperature,
    - high temperature,
    - configurable/default cycle period,
    - first default could be 30 minutes high and 30 minutes low,
    - heater still uses hysteresis around current target.
29. Heater control uses simple hysteresis, not PID or time-proportional control initially.
30. Temperature units:
    - Celsius only,
    - integer display,
    - user input in 1 C steps.
31. UI language:
    - LCD/user-facing text Romanian only,
    - serial logs/code/comments/docs English.
32. Serial logs should be human-readable English-ish but easy for testing tools to parse.
33. Documentation structure accepted:
    - requirements,
    - architecture,
    - risks,
    - test plan,
    - agents,
    - ADRs,
    - Romanian UI document.
34. Reusable agent definitions should be Markdown files in the repo.
35. Agent invocation style is hybrid:
    - Codex recommends/invokes at natural checkpoints,
    - user can manually invoke agents.
36. Agent definitions should include operational content:
    - role,
    - responsibilities,
    - inputs,
    - outputs,
    - constraints,
    - checklist,
    - example invocation,
    - expected final response format.
37. Agent permission model:
    - most review agents are read-only by default,
    - firmware/test/doc agents edit only when explicitly assigned bounded ownership.
38. Workflow should be branch/commit based. User wants to learn manual Git after Codex performs the first baseline branch/commit/push.
39. Conversation/session persistence should be handled by repo docs, not chat memory. `project-context.md` and `project-discovery-log.md` should preserve decisions and reasoning.
40. Include a written backlog.
41. Use ADRs for important decisions.
42. Use a lightweight risk log.
43. Documentation updates are triggered manually by the user, by Codex coordinator when an impact is noticed, and at milestones.
44. Documentation impact check is required before branch merge and when changes affect requirements, architecture, safety, tests, UI, persistence, hardware, or external interfaces.
45. Test documentation should include strategy and concrete test cases.
46. Firmware must be testable from the beginning:
    - separate pure logic from Arduino hardware calls,
    - state machine/profile/fault logic should be unit-testable.
47. PlatformIO should eventually support real Mega2560 firmware and native unit tests for pure logic.
48. Dev environment is working and should only be inspected initially. Changes to `.devcontainer`, `platformio.ini`, upload settings, serial settings, dependency config, or CI scripts require explicit approval.
49. External libraries are allowed when useful, but must be wrapped behind project-owned interfaces and documented.
50. Use clear C++ without over-engineering.
51. Add helpful comments for complex safety/control/business logic.
52. Add a lightweight coding standards document.
53. Requirement IDs should be stable and not reused.
54. Requirements should be grouped into functional, safety, hardware, UI, logging/diagnostics, persistence, testing/verification, and future/deferred.
55. LCD is 4x20, not 4x32.
56. Buzzer is only for finish/fault alarms, not routine button feedback.
57. LCD backlight is controlled through a FET on a separate MCU pin.
58. Relay polarity should be configurable.
59. PT50 is voltage-divider based into analog input, supplied from MCU VCC with no dedicated reference.
60. Product-minded recommendation: consider better reference, calibration procedure, or RTD interface IC for more accurate PT50 measurement.
61. Calibration is needed:
    - PT50 offset/scale,
    - optional AHT offsets,
    - visible in debug,
    - UI calibration can wait.
62. Duration values:
    - support up to 99 h 0 min,
    - editable/structured forms may use `HH:MM`,
    - compact LCD summaries may use `Xh Ym`.
63. Humidity is reporting only in current scope.
64. AHT failure is warning only; drying can continue without RH.
65. RH should be shown on LCD if AHT sensor is present/functional.
66. Manual mode still enforces all safety limits.
67. Startup self-checks should include outputs OFF first, PT50 plausibility, AHT availability, config validation/defaulting, stuck input detection, interrupted run detection, and explicit confirmation before resume/start.
68. Serial lifecycle events should be logged:
    - boot,
    - self-check,
    - run start,
    - pause/resume,
    - output changes,
    - warnings,
    - faults,
    - finish,
    - acknowledgement.
69. Serial periodic state log interval is 5 seconds.
70. LCD should update changed fields rather than redraw constantly.
71. Heartbeat symbol should appear in the bottom-right corner and always continue while firmware main loop is alive.
72. UI/menu flow should be documented before implementation.
73. Testable architecture:
    - pure C++ control logic,
    - fake hardware for tests,
    - board-independent unit tests for control/profile/fault behavior.
74. No dynamic allocation in application code:
    - avoid `new`, `delete`, `malloc`, `free`,
    - avoid Arduino `String`,
    - prefer fixed buffers and `snprintf`.
75. LCD text strings are allowed; the rule is against dynamic heap-allocated strings, not against text.
76. Avoid blocking delays longer than 100 ms in normal runtime logic. Use `millis()` and a simple cooperative scheduler/event loop.
77. Centralize hardware pins and configuration.
78. Configuration should be separated into categories:
    - hardware,
    - safety,
    - control timing,
    - UI timing,
    - logging,
    - calibration,
    - presets.
79. EEPROM writes must be minimized:
    - never in fast loops,
    - write only when needed,
    - use version and checksum/validation,
    - use defaults on invalid config,
    - avoid over-engineered wear leveling initially.
80. Interrupted-run state should be stored in EEPROM at minimal checkpoints, no sooner than every 15 minutes during a run except major events.
81. Resume state clears automatically on normal finish.
82. On hard fault, retain last run/fault context for diagnostics but do not allow resume.
83. Fault codes should be defined from the beginning.
84. LCD should show short Romanian fault names while serial logs use stable English fault codes.
85. Warnings are shown/logged but do not require acknowledgement.
86. Encoder behavior:
    - rotate to navigate/change value,
    - short press select/confirm,
    - menu-like screens should provide `Inapoi` as the last entry for one-level-up navigation,
    - long press is currently reserved and not required for navigation.
87. No dedicated emergency-stop shortcut is required.
88. Normal stop/cancel during run requires confirmation.
89. Normal stop/cancel has no cooldown.
90. Normal program finish has time-based 3-minute fan cooldown, not temperature-based.
91. Event logging:
    - every event, parameter change, and output change is logged,
    - temperature status is only periodic.
92. Timing defaults:
    - PT50/control 1 s,
    - AHT 2 s,
    - serial status 5 s.
93. PT50 readings should use simple averaging/filtering and reject impossible/out-of-range values.
94. Raw ADC is verbose/debug only.
95. Serial command interface is deferred; baseline is logs only.
96. Relay protection:
    - heater min ON/OFF 10 s,
    - fan min ON/OFF 10 s,
    - hard fault and explicit stop can force OFF immediately.
97. Temperature safety layers:
    - setpoint max 75 C,
    - heater forced OFF above 75 C,
    - hard fault at 80 C.
98. No-temperature-rise fault:
    - 2 C rise expected within 5 min while heater ON.
99. Heater stuck ON fault:
    - PT50 rises 3 C over 5 min while heater command is OFF.
100. Pushbutton stuck threshold:
    - active continuously for 30 s is hard fault.
101. Watchdog should be enabled after basic firmware is stable.
102. Reset cause should be logged at boot and recorded in EEPROM carefully.
103. Brown-out/power loss during run offers resume after confirmation; watchdog reset is stricter and must not auto-resume.
104. Secondary serial telemetry mirrors primary logs, output-only, enabled by default.
105. Both serial outputs default to 115200 8N1, no flow control.
106. Add a hardware interface table with TBD pins.
107. Add `docs/hardware.md`.
108. Product-minded recommendations should be clearly labeled.
109. Do not include food safety guidance in baseline docs.
110. UI docs should include draft Romanian text examples but mark them draft.
111. LCD Romanian text should be ASCII-only by default; heartbeat custom character has priority.
112. Add instructions for future Codex sessions to recover after session loss.
113. README should remain the project entry point and preserve existing setup knowledge. Detailed WSL/devcontainer/SSH-agent/serial instructions may live in `docs/dev-environment.md`.
114. Dev environment audit is mandatory before editing environment/tooling files.

## First Baseline Work Agreement

The user allowed Codex to perform the first branch/commit/push to establish this documentation baseline. After that, the user wants to perform Git operations manually for a while, with Codex providing guidance and recommended commands.

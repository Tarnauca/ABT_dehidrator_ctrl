# Romanian LCD UI Draft

Status: draft. LCD labels are Romanian, ASCII-only by default, and must fit a 4x20 character display.

## Rules

- LCD size: 4 lines x 20 characters.
- User-facing text: Romanian.
- Use ASCII-only Romanian initially because HD44780 diacritics are uncertain.
- Reserve bottom-right cell for the heartbeat symbol.
- Update changed fields instead of full-screen redraws where practical.
- Buzzer is used only for finish and fault alarms.
- Backlight blinking is controlled through a dedicated FET output pin.

## Draft Status Screen

Initial implemented bring-up screen:

```text
Stare: INACTIV
T:--°C   RH:--%
H:OFF     F:OFF
                   <3
```

The heartbeat occupies the bottom-right character as a custom LCD glyph. Sensor values are shown as `--` until real readings are connected to the status snapshot.

Current implemented lifecycle labels on the status screen include:

- `INACTIV`
- `RULARE`
- `RACIRE`
- `FINALIZAT`

When `FINALIZAT` is shown, a short press acknowledges the finished run and
returns the controller to idle instead of opening the menu.

## Draft Fault Screen

```text
EROARE
Senzor NTC
Iesiri oprite
Apasa pt. confirm.
```

## Draft Finish Screen

```text
Program finalizat
Ventilare terminata

Apasa pt. OK
```

## Draft Menu Items

- `Pornire preset`
- `Mod manual`
- `Testare`
- `Setari`
- `Reluare program`
- `Oprire`
- `Inapoi`

## Implemented Menu Shell

Current bring-up behavior:

- Short press from status opens the menu.
- First line shows the current section title.
- Encoder rotation moves the highlighted item on the second, third, and fourth lines.
- Short press on a menu item logs the selection.
- The last selectable entry is always `Inapoi`.
- Selecting `Inapoi` returns exactly one level up in the UI hierarchy.

Current menu layout:

```text
Meniu
>Pornire preset
 Mod manual
 Testare
```

The menu does not wrap around at the ends. The selected item stays on the
second line, with the remaining visible items listed below it.

## Implemented Preset Selection Shell

Current bring-up behavior:

- Select `Pornire preset` from the menu to open the preset screen.
- Encoder rotation moves between built-in starter presets.
- Short press confirms the selected preset and starts the associated run.
- If another preset run is already active, confirming a new preset opens a
  `Confirmare` screen with `Nu` / `Da`.
- `Nu` returns to preset selection without changing the active run.
- `Da` stops the current run and starts the newly selected preset.
- The last selectable row is `Inapoi`, which returns to the menu.
- If the user exits with `Inapoi`, the next entry into the preset screen starts
  again from the first real preset, not from `Inapoi`.

Current preset layout:

```text
Pornire preset
>Mere
Mod fluctuat
50-65°C 10h 0m
```

The preset list currently contains starter values that can be replaced later
when the product manual or bench calibration values are available.

After a preset starts successfully, the firmware returns to the status screen
and the displayed state changes from `INACTIV` to the active run lifecycle
label.

Replacement confirmation layout:

```text
Confirmare
Inlocuire program?
>Nu    Da
```

## Implemented Manual Program Shell

Current bring-up behavior:

- Select `Mod manual` from the menu to open the manual program screen.
- Encoder rotation navigates between `Temp`, `Dur`, `F:Nu/Da`, `Start`, and `Inapoi`.
- Short press on `Temp`, `Dur`, or `F:Nu/Da` enters or leaves edit mode.
- While a field is in edit mode, encoder rotation changes its value.
- Short press on `Start` starts the configured manual program.
- If another run is already active, starting a manual program uses the same
  `Confirmare` screen as preset replacement.
- `Inapoi` is always the last selectable entry and returns to the menu.
- If the user exits with `Inapoi`, the next entry into manual mode starts again
  from `Temp`, not from `Inapoi`.

Current manual layout:

```text
Mod manual
>Temp:57°C
 Dur:8h 0m
 F:Nu Start Inapoi
```

## Implemented Test Shell

Current bring-up behavior:

- Select `Testare` from the menu to open the direct output test screen.
- Encoder rotation switches between `Fan`, `Heat`, and `Inapoi`.
- Short press toggles the selected output.
- `Inapoi` is always the last selectable entry and returns to the menu.
- If the user exits with `Inapoi`, the next entry into test mode starts again
  from `Fan`, not from `Inapoi`.
- Turning `Heat` ON also forces `Fan` ON.
- Turning `Fan` OFF also forces `Heat` OFF.

Current test layout:

```text
Testare
>Fan: OFF
 Heat: OFF
 Inapoi
```

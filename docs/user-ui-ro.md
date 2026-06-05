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

- `Oprire program` - shown only when a running or stoppable program exists
- `Reluare program` - shown only when a resumable program exists
- `Programe presetate`
- `Programe utilizator`
- `Program manual`
- `Setari`
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

Example menu layout without active/resumable program:

```text
Meniu
>Programe presetate
 Programe utilizator
 Program manual
```

Example menu layout while a resumable run exists:

```text
Meniu
>Oprire program
 Reluare program
 Programe presetate
```

The menu does not wrap around at the ends. The selected item stays on the
second line, with the remaining visible items listed below it.

## Implemented Preset Selection Shell

Current bring-up behavior:

- Select `Programe presetate` from the menu to open the preset screen.
- Encoder rotation moves between built-in starter presets.
- Short press confirms the selected preset and starts the associated run.
- If another preset run is already active, confirming a new preset opens a
  `Confirmare` screen with the question `Pornesti programul nou?` and
  `Nu` / `Da`.
- `Nu` returns to preset selection without changing the active run.
- `Da` stops the current run and starts the newly selected preset.
- The last selectable row is `Inapoi`, which returns to the menu.
- If the user exits with `Inapoi`, the next entry into the preset screen starts
  again from the first real preset, not from `Inapoi`.
- Long press currently has no assigned action.

Current preset layout:

```text
Programe presetate
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
Pornesti programul
nou?
>Nu    Da
```

## Implemented Manual Program Shell

Current bring-up behavior:

- Select `Program manual` from the menu to open the manual program screen.
- The first selectable entry is `Mod`, with values `Constant`, `Boost`, and
  `Fluctuant`.
- The visible field list changes dynamically based on the selected mode.
- Encoder rotation navigates through the current field list.
- Short press on editable fields enters or leaves edit mode.
- While a field is in edit mode, encoder rotation changes its value.
- Short press on `Start` starts the configured manual program.
- `Salveaza` appears before `Inapoi`.
- If another run is already active, starting a manual program uses the same
  `Confirmare` screen with `Pornesti programul nou?` as preset replacement.
- If `Start` or `Inapoi` is selected while the current manual profile has
  unsaved changes, the controller opens a `Da / Nu / Renunta` save prompt.
- Choosing `Da` opens the 10-slot save list and then resumes the original
  action automatically.
- Choosing `Nu` skips saving. For `Inapoi`, unsaved changes are discarded.
- Choosing `Renunta` returns to the editor.
- `Inapoi` is always the last selectable entry and returns to the menu.
- If the user exits with `Inapoi`, the next entry into manual mode starts again
  from `Mod`, not from `Inapoi`.
- Long press currently has no assigned action.

Manual mode variants:

- `Constant`: `Temp`, `Dur`, `Start`, `Salveaza`, `Inapoi`.
- `Boost`: `Temp`, `Dur`, `Boost`, `DurBoost`, `Start`, `Salveaza`,
  `Inapoi`.
- `Fluctuant`: `Temp`, `Dur`, `Tsup`, `Tinf`, `Dur Tsup`, `Dur Tinf`,
  `Start`, `Salveaza`, `Inapoi`.

Manual edit constraints:

- `Boost` is a temperature delta above the base temperature, from 0 C to 20 C,
  in 5 C steps.
- `Boost` target temperature is blocked if it would exceed 75 C.
- `DurBoost` is edited in 5 min steps and cannot exceed 50% of total duration.
- `Tsup` and `Tinf` are absolute temperatures, not offsets.
- `Tsup` and `Tinf` are blocked if they would move more than 10 C away from
  the reference temperature or outside 0...75 C.
- `Dur Tsup` and `Dur Tinf` are edited from 5 min to 20 min in 1 min steps.

Current manual layout:

```text
Program manual
>Mod:Constant
 Temp:57°C
 Dur:8h 0m
```

Example `Boost` scrolled layout:

```text
Program manual
>Boost:+10°C
 DurBoost:30m
 Start
```

Example `Fluctuant` scrolled layout:

```text
Program manual
>Tsup:62°C
 Tinf:52°C
 Dur Tsup:10m
```

## Implemented Settings Menu

Current behavior:

- Select `Setari` from the main menu to open the settings submenu.
- `Setari` currently exposes only `Testare` and `Inapoi`.
- `Inapoi` returns to the main menu.
- Long press currently has no assigned action.

Current settings layout:

```text
Setari
>Testare
 Inapoi

```

## Implemented Test Shell

Current bring-up behavior:

- Select `Testare` from `Setari` to open the direct output test screen.
- Encoder rotation switches between `Fan`, `Heat`, and `Inapoi`.
- Short press toggles the selected output.
- `Inapoi` is always the last selectable entry and returns to the menu.
- If the user exits with `Inapoi`, the next entry into test mode starts again
  from `Fan`, not from `Inapoi`.
- Turning `Heat` ON also forces `Fan` ON.
- Turning `Fan` OFF also forces `Heat` OFF.
- Long press currently has no assigned action.

Current test layout:

```text
Testare
>Fan: OFF
 Heat: OFF
 Inapoi
```

## Implemented Save Prompt

Current behavior:

- `Da` continues to the save-slot list.
- `Nu` continues without saving or discards on back.
- `Renunta` returns to the editor.

Current save prompt layout:

```text
Salveaza profil?
Alege:
>Nu
Roteste si apasa
```

## Implemented User Profile Slots

Current behavior:

- `Programe utilizator` opens the 10-slot list.
- The same slot list is reused by `Salveaza`.
- All 10 slots are always visible through scrolling.
- Vacant slots show `Profil N (nedef.)`.
- Occupied slots show only `Profil N`, without a mode suffix on the same line.
- The last selectable entry is always `Inapoi`.

Example slot list:

```text
Programe utilizator
>Profil 1 (nedef.)
 Profil 2
 Profil 3 (nedef.)
```

## Implemented User Profile Detail

Current behavior:

- Occupied slots expose `Pornire`, `Editeaza`, `Sterge`, `Inapoi`.
- Vacant slots expose `Editeaza`, `Inapoi`.
- `Sterge` asks for confirmation.
- Saving over an occupied slot asks for confirmation.
- `Editeaza` loads the saved manual profile back into `Program manual`.

Example occupied detail layout:

```text
Profil 2
Boost
55/65°C 6h 0m
>Pornire
```

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
T:--C    RH:--%
H:OFF     F:OFF
                   <3
```

The heartbeat occupies the bottom-right character as a custom LCD glyph. Sensor values are shown as `--` until real readings are connected to the status snapshot.

## Draft Fault Screen

```text
EROARE
Senzor PT50
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
- `Setari`
- `Reluare program`
- `Oprire`
- `Confirmi oprirea?`

## Implemented Menu Shell

Current bring-up behavior:

- Short press from status opens the menu.
- Encoder rotation moves the highlighted item.
- Short press on a menu item logs the selection.
- Long press returns to the status screen.

Current menu layout:

```text
>Pornire preset
 Mod manual
 Setari
Apas=OK Tine=Inap
```

The last line still keeps the bottom-right heartbeat glyph.

The menu does not wrap around at the ends. The selected item stays on the
first visible line, with the remaining visible items listed below it.

## Implemented Manual Mode Shell

Current bring-up behavior:

- Select `Mod manual` from the menu to open the manual screen.
- Encoder rotation switches between `Fan` and `Heat`.
- Short press toggles the selected output.
- Long press returns to the menu.
- Turning `Heat` ON also forces `Fan` ON.
- Turning `Fan` OFF also forces `Heat` OFF.

Current manual layout:

```text
Mod manual
>Fan: OFF
 Heat: OFF
Apas=Sch Tine=Inap
```

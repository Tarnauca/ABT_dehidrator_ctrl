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

Example only:

```text
Mod: Fix    T:57C
RH:42%      H:OFF
Fan:ON   R:08:15
Stare: Ruleaza     <3
```

The heartbeat occupies the bottom-right character. Exact symbol depends on custom character support.

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

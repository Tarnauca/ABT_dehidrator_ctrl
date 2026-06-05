# User Profile Persistence Review 001

Status: resolved in branch `feat/manual-program-modes`.

## Scope

- EEPROM-backed storage for 10 user-defined manual profiles
- Manual-editor save/start/back prompt behavior
- User-profile browse/start/edit/delete UI flow

## Review Passes

### Requirements / Product Review

Findings:

1. Saving and resume persistence must remain separate concepts.
2. Overwrite and delete need explicit confirmation because they destroy user
   state.
3. Unsaved `Inapoi` must genuinely discard, not merely hide, edits.

Resolution:

- Added separate user-profile persistence requirements and architecture notes.
- Added overwrite/delete confirmation screens.
- Added baseline/discard handling inside `ManualProgramController`.

### Architecture Review

Findings:

1. EEPROM layout must not depend on compiler struct padding.
2. UI flow state needed to remain readable as save/overwrite/delete branches
   multiplied.

Resolution:

- `UserProfileStore` now uses explicit byte-wise encoding/decoding.
- `main.cpp` uses small purpose enums for save flow and confirmation context.

### Test Review

Findings:

1. Persistence logic needed dedicated native coverage.
2. Unsaved prompt and user-profile menu controllers needed pure tests before
   bench wiring.

Resolution:

- Added `test_user_profile_store`.
- Added `test_save_prompt_controller`.
- Added `test_user_profile_controllers`.
- Extended manual editor and LCD tests for `Salveaza`, dirty state, and
  discard behavior.

## Outcome

No blocking findings remain for this branch after the current native and Mega
build verification pass.

# Alt Spam Helper

Target: Geometry Dash Android 2.2081, Geode 5.11.2, Android 64-bit.

## Current prototype

- Geode file-setting picker for macro selection.
- Re-analyses the selected path when the setting changes.
- Reads selected files as text and looks for a sustained alternating sequence of JSON-style `"button": 1` and `"button": 2` events.
- Logs whether that simple pattern was detected.
- Keeps the two-finger touch detector prototype.

## Important limitations

This is an early importer/detection experiment, not a finished macro player.

- The current parser only recognises JSON-like event data with a `button` field. It does not yet parse binary GDR, ToastyReplay, MegaHack, xdBot, TasBot, or every other format.
- Alternating button events are only a rough signal; they do not prove that the macro is alternate wave spam.
- The in-game Alt Spam controls are not conditionally displayed yet.
- Macro playback, playback synchronisation, and automated input are not implemented.
- Android file picker behaviour and the build must be tested on-device.

Do not describe this prototype as a working macro player or distribute it as a tested release until CI builds successfully and it has been tested in Geometry Dash Android 2.2081.

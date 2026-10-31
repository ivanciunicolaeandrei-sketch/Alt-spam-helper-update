# Alt Spam Helper

Android64 Geode prototype targeting Geometry Dash 2.2081 and Geode 5.11.2.

## Current prototype
- Select a macro through the Geode **Import macro** file setting.
- The file is re-analysed whenever a new file is selected.
- The first parser recognises JSON-like event fields named `button` or `btn` with values `1` and `2`.
- It reports a possible sustained alternating pattern in the Geode log.

## Important limitations
- This is an importer/parser prototype, not a finished gameplay helper.
- It does not play macros, inject taps, or automate inputs.
- It does not decode binary GDR or other proprietary replay formats yet.
- Detection is a simple pattern heuristic and can produce false positives or miss valid patterns.
- The current analysis result is visible in the Geode log, not in a custom in-game panel.
- The Tap rate and Helper Enabled settings are placeholders and currently have no gameplay effect.

A successful Android64 build is required before installing this mod. Do not treat the source code as a ready-to-install .geode package.

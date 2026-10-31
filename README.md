# Alt Spam Helper

Target: Geometry Dash Android 2.2081, Geode 5.11.2, Android 64-bit.

## Intended macro workflow

1. The player imports a macro.
2. The mod automatically loads the imported macro; there is no separate manual load step.
3. The mod analyses the loaded macro for alternate wave-spam input patterns.
4. If alternate wave spam is detected, the Alt Spam controls become available.
5. If it is not detected, the Alt Spam controls stay hidden.

The detection must be based on the macro's actual input data, not merely its filename or extension. Re-importing a macro should replace the previous loaded macro and refresh the detection result.

## Current prototype status

- Geode project and Android64 cloud-build workflow.
- Invisible two-finger touch detector prototype.
- Basic settings metadata.

## Not implemented yet

- Macro file importer and automatic loading.
- Alternate wave-spam pattern detection.
- Conditional visibility of Alt Spam controls.
- Macro playback and level-position synchronization.
- Pause-menu controls.

The macro format must be identified and tested with a real sample before implementing a reliable parser. The project must compile and be tested before a downloadable .geode package can be called working.

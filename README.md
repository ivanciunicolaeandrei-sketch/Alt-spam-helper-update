# Alt Spam Helper

Target: Geometry Dash Android 2.2081, Geode 5.11.2, Android 64-bit.

## Intended macro workflow

1. The player imports a macro.
2. The mod attempts to identify the macro format from its contents and metadata, rather than relying only on the file extension.
3. A matching format parser reads the macro. The design should support multiple formats through separate parser adapters.
4. A successfully parsed macro loads automatically.
5. The mod analyses its input events for alternate wave-spam patterns.
6. If alternate wave spam is detected, the Alt Spam controls become available. Otherwise those controls remain hidden.
7. Importing another macro replaces the current one and refreshes the detection result.

## Format compatibility

The goal is broad, multi-format compatibility. It is not technically honest to promise every format: undocumented, encrypted, corrupted, or proprietary formats may not be readable without a specification or sample. Unknown formats should show a clear unsupported-format message rather than being silently accepted.

Detection must use the actual parsed input events, not just the filename or extension.

## Current prototype status

- Geode project and Android64 cloud-build workflow.
- Invisible two-finger touch detector prototype.
- Basic settings metadata.

## Not implemented yet

- File picker and macro importer.
- Format identification and parser adapters.
- Automatic macro loading.
- Alternate wave-spam pattern detection.
- Conditional visibility of Alt Spam controls.
- Macro playback and level-position synchronization.
- Pause-menu controls.

The macro formats need real sample files and testing before reliable parser support can be claimed. The project must compile and be tested before a downloadable .geode package can be called working.

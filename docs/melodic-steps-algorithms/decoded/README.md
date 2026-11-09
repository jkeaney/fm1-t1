# Decoded items: Melodic Steps

| Item | What it is |
| --- | --- |
| `patcher.json` | the main patcher, decoded from the `.amxd` container |
| `embedded_patchers/` | the eight patchers embedded in the container: the five lane faders, the shared lane engine (`fader_logic_new`), one step cell and the edit buttons. ⚠ names are inferred from each patcher's contents and the device's dependency list |
| `presets_decoded.json` | all 7 presets: the 6 stored parameters and the full blob (80 fader levels, Root Note, Scales, Swing, Triplets, MIDI) |
| `presets_xml/` | the 7 `.adv` presets, gunzipped to their original XML (the blob is hex-encoded JSON) |

The decoded preset tables for review are in [../07-presets.md](../07-presets.md). The device has no embedded JavaScript.

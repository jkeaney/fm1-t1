# Decoded items: Rhythmic Steps

| Item | What it is |
| --- | --- |
| `patcher.json` | the main patcher, decoded from the `.amxd` container |
| `embedded_patchers/` | the six embedded helper patchers. `…Lane_(note_maker_and_layers).json` (about 200 KB) holds the per-lane logic (`PerTypeSequencer`, `MakeNote`, `Roll`, `MpeNoteGen`) ⚠ other labels are guesses |
| `presets_decoded.json` | all 19 presets: every parameter (value and default) and the 20 step arrays (4 lanes × Chance/Velo/Roll/Slide/Alt) |
| `presets_xml/` | the 19 `.adv` presets, gunzipped to their original XML (the step blob inside is hex-encoded JSON) |

The decoded preset tables for review are in [../07-presets.md](../07-presets.md).

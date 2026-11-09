# Decoded items: Sting 2.08

Extracted from `sting 2.amxd` for study. Nothing here is edited except trimming container bytes off the ends of the JS files.

| Item | What it is |
| --- | --- |
| `patcher.json` | the main patcher, decoded from the `.amxd` container (JSON) |
| `js/` | the six embedded JavaScript helpers (`sting_seq.js` grid UI, `sting_ui25.js` eye animation UI, `stng_awr.js` scale snap, `listremove.js` pair removal, `step_r.js` / `step_r_edit.js` Push LED helpers) |
| `embedded_patchers/` | the five patchers embedded in the container |

**Caveats**
- The container stores files back to back without a reliable index in the part I parsed, so the JS boundaries and the order/names of the embedded patchers were found by pattern and byte ranges.
  The JS files were trimmed at the first container marker; each should be complete, but check any file before relying on it.
- The embedded patcher names are my best guess from the device's dependency list. ⚠

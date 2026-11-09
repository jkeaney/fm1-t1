# Decoded items: MDD Snake V3.2.3

| Item | What it is |
| --- | --- |
| `patcher.json` | the main patcher, decoded from the `.amxd` container (JSON, about 4.7 MB; mostly UI and colour subpatchers) |
| `embedded_patchers/M4L.Chooser.maxpat.json` | the embedded parameter chooser patcher |
| `js/M4L.chooser.js` | the embedded Cycling '74 parameter-mapping helper (not part of the musical logic) |

The 14 snake shape tables are inside `patcher.json` under the `Snake Core` subpatcher (message boxes of 16 numbers); the scale tables are `table` objects with saved data.

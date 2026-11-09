# Decoded items: Step Arp

| Item | What it is |
| --- | --- |
| `patcher.json` | the main patcher, decoded from the `.amxd` container |
| `embedded_patchers/` | the four patchers embedded in the container. The first (82 boxes) is the shared 16-slider lane widget used by Velocity, Length, Chance, Ratchet, Transpose and Map/CC. ⚠ The labels on the other three are guesses from their contents |

The device's small UI helper script (`VisibleBang.js`) is also embedded but is not part of the musical logic and is not extracted.

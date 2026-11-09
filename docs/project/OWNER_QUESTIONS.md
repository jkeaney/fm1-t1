# Open questions for the owner

Collected from design tasks; resolve at the matching human-review gate (P1-010 / P2-001..P2-004).

| # | From | Question | Needed by |
| --- | --- | --- | --- |
| 1 | P1-003 sting | Is ACID without slide (TB-3 portamento) acceptable for v1? | P1-010 |
| 2 | P1-003 sting | How does the shared scale-quantise (SCL QNT) setting relate to ACID's own scale snap? | P2-001 |
| 3 | P1-003 sting | VISION rule 3 says a channel's only setting is its MIDI channel, but the algorithm docs assume shared GATE / DIV / SWING. Are those global, per-algorithm, or dropped? | P2-001 / P2-004 |

## Decisions (owner, 2026-11-09)
| # | Decision |
| --- | --- |
| 1 | P0-005 flash: owner will flash in the morning (2026-11-10). Task restored to todo, human-run. |
| 2 | ACID gets slide as a sixth dice layer in v1 (task P1-011). |
| 3 | Scale is ONE control, used by every algorithm; global by default (not per-channel). |
| 4 | GATE, DIV, SWING (and scale) are GLOBAL performance settings on the panel, not per channel. A channel still has only its MIDI channel and algorithm. |
| 5 | Branch t1-hardware-bringup pushed to origin (jkeaney/fm1-t1). |

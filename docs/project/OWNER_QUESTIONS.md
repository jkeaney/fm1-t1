# Open questions for the owner

Collected from design tasks; resolve at the matching human-review gate (P1-010 / P2-001..P2-004).

| # | From | Question | Needed by |
| --- | --- | --- | --- |
| 1 | P1-003 sting | Is ACID without slide (TB-3 portamento) acceptable for v1? | P1-010 |
| 2 | P1-003 sting | How does the shared scale-quantise (SCL QNT) setting relate to ACID's own scale snap? | P2-001 |
| 3 | P1-003 sting | VISION rule 3 says a channel's only setting is its MIDI channel, but the algorithm docs assume shared GATE / DIV / SWING. Are those global, per-algorithm, or dropped? | P2-001 / P2-004 |
| 4 | P1-004 snake | The global scale control: what is its default value, and is "off" (chromatic) a setting? SNAKE's random pitches sound arbitrary on chromatic; recommend a minor-pentatonic default or a non-chromatic first scale. | P2-001 |
| 5 | P1-004 snake | Is a uniformly random pitch lane acceptable as a bass? Fallbacks: lower default RANGE (12) or scale-weighted pitches. | P1-010 |
| 6 | P1-004 snake | Should the dropped Custom modulation lanes return post-v1 as a CC lane? | P1-010 |
| 7 | P1-011 slide | SLIDE has no amount control (fixed ~16% of steps, only dice changes it) because the 4 knobs are taken. Acceptable, or give it a shifted-knob/button gesture to turn slide off? Recommend acceptable for v1; dice until none. | P1-010 |
| 8 | P1-011 slide | Slide is sent as overlapping notes (no CC). Confirm on the TB-3 that this glides (hardware test); if not, we try CC65. | first hardware test |
| 9 | P1-011 slide | A dice or knob change now takes effect one step later (next-step commit, up to ~125 ms at 120 BPM). Acceptable? | P1-010 |
| 10 | P1-005 ml185 | STAGE loops can be 8..64 pulses but the 16 white keys reach only pulses 0..15. Acceptable (long loops come from the generator), or should step entry get a page/shift to reach pulses 16..63? | P1-010 / P2-003 |
| 11 | P1-005 ml185 | STAGE long holds (mode I-) ignore the global GATE so a long note stays long when GATE is shortened. Acceptable? (A table edit if not.) | P1-010 |
| 12 | P1-005 ml185 | Are STAGE's defaults (TOTAL 16, DENS 6, RATCH 2, RANGE 12, base C2) good on a TB-3, and is a rhythm-first 8-pitch riff generator (no accent or slide lane) enough? | first hardware test |
| 13 | P1-006 steparp | ARP is recommended LATER (its rhythm half duplicates SNAKE/STAGE; only CHORD, ORDER, OCT are new). If v1 needs a harmonic algorithm, promote it as a fourth algorithm on the shared rank-order/ratchet primitives, or offer CHORD/ORDER as a pitch option for other algorithms instead? | P1-010 |
| 14 | P1-006 steparp | Is a 2..4-note chord pool enough for a TB-3 bass, or does it want a root-on-downbeat bias? | first hardware test (only if promoted) |
| 15 | P1-006 steparp | Chord snap on sparse scales turns major into minor in minor pentatonic. Acceptable? (Relates to #2 and #4.) | P2-001 |
| 16 | P1-006 steparp | Chord-tone snap direction: at or below (as inferred in the source) or nearest? | P2-001 |

## Decisions (owner, 2026-11-09)
| # | Decision |
| --- | --- |
| 1 | P0-005 flash: owner will flash in the morning (2026-11-10). Task restored to todo, human-run. |
| 2 | ACID gets slide as a sixth dice layer in v1 (task P1-011). |
| 3 | Scale is ONE control, used by every algorithm; global by default (not per-channel). |
| 4 | GATE, DIV, SWING (and scale) are GLOBAL performance settings on the panel, not per channel. A channel still has only its MIDI channel and algorithm. |
| 5 | Branch t1-hardware-bringup pushed to origin (jkeaney/fm1-t1). |

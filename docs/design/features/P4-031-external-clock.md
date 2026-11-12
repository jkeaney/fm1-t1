<!-- SPDX-License-Identifier: GPL-3.0-only -->
# P4-031 External clock: use case

## Gate A (before the build)
- **Who/when:** a producer syncing the FM-1 to a DAW or a clock box during a live set.
- **Steps:** (1) connect the host or TRS clock; (2) press PLAY on the host; (3) the header shows EXT over the BPM and the pattern starts on the host's start; (4) change tempo on the host, the BPM follows; (5) stop the host: the FM-1 stops, notes off; (6) clock disappears while playing: header shows WAIT then the red lost mark; the FM-1 keeps its last tempo.
- **Panel:** no new control; PLAY on the panel acts as clkx_play/clkx_stop.
- **Screen:** header tempo slot per SCREEN_SPEC_V2 (EXT / WAIT / lost); no new element.
- **Live-performance check:** no menu visit is needed; the state is readable at a glance.

## Gate B (after the build)
Not yet done.

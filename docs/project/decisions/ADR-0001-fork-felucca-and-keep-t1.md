# ADR-0001: Build on Felucca 1.6 + the T-1 fork, then reduce

- **Status:** accepted (2026-11-09)
- **Context:** Felucca 1.6 already solves the hard platform work on the FM-1: boot, the LCD and fonts, input scanning,
  USB MIDI and audio, flash storage, the OTA loader and the installer, plus a host-test harness with about 4,400
  checks. The tensor-programming fork adds a T-1-style algorithmic sequencer, now working on hardware. The vision
  needs a sequencer-first device, and most of Felucca's synth and groovebox features are out of scope.
- **Decision:** keep this codebase as the platform. Map it in P1, decide keep/adapt/remove in P2 (SYSTEM_MAP), and
  remove in P3 with the build and tests green after each removal. Don't rewrite from scratch, and don't keep
  patching features on top before the maps exist.
- **Consequences:** the unity-build include order and freestanding constraints stay (see CLAUDE.md). The update
  loader and installer stay untouched, which keeps recovery safe. Removing code also means removing or updating its
  host tests in the same task.

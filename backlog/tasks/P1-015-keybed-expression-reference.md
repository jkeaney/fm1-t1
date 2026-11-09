---
id: P1-015
title: Identify the open-source keybed-expression (MPE-like) reference and what the FM-1 keybed can sense
phase: P1
type: spike
status: done
priority: 0
executor: sonnet
depends_on: [P0-001]
gate: none
---
## Goal
Owner requirement R4: perform a key and get expression (ratchet, slide, mute ...) using the keybed, following the open-source project that unlocked MPE-like capability from the FM-1 keybed. The project is not identified yet. Find it, learn how it does it, and say what the hardware allows.

## Read first
- docs/project/OWNER_QUESTIONS.md (requirements R1-R4, ninth round)
- docs/design/CONTROL_INVENTORY.md and firmware/src/panel.c (how keys are scanned today), hal/ headers for the key matrix
- Candidate community/official projects named in press: FM1 Open (M-VAVE official open firmware, GitHub, SDK, V14), KARMA, SLOOP, Felucca (this repo's base), X0X, FM-106; plus Elektronauts thread 'M-vave fm-1' and the reverse-engineering write-ups (e.g. the Korean zeli.app firmware reverse-engineering story). Search the web for MPE, aftertouch, key pressure, velocity, 'per-key', 'expression' with these names; read the actual repo sources (git clone into a NEW empty directory under /tmp, never run code from it; read-only).

## Deliverable
- docs/design/KEYBED_EXPRESSION_REFERENCE.md: (1) which project(s) do this, with URLs, licence (GPL compatible?), commit/file references, and a verdict on how sure you are; if no project does, say so plainly and list the closest; (2) how it works: what the hardware senses (velocity by two-contact timing? pressure? per-key analog? a held-time gesture?), the scan method, resolution and latency, and the exact MIDI it emits (per-note channel/MPE zone, pitch bend, pressure, CC74); (3) what the FM-1 keybed can and cannot sense, evidence for each claim tagged [V] verified in source, [I] inferred, or unconfirmed; (4) candidate performative gestures this enables for a sequencer step/key (ratchet, slide, mute, accent, probability, pitch bend), each with the input needed, feasibility (certain/likely/unlikely) and cost in CPU/RAM if known; (5) a recommendation for which gestures the design should rely on and a fallback if the hardware can only do velocity/hold-time; (6) owner questions.

## Acceptance criteria
- [x] Every claim about hardware capability cites a source file/URL and is tagged [V]/[I]/unconfirmed
- [x] The answer to 'which project unlocked MPE-like expression from the keybed' is stated, or 'not found' with the list of repos/pages checked
- [x] At least 4 candidate gestures are assessed for feasibility
- [x] Nothing was cloned into or executed from the repo directory; no code run from downloaded sources

## Verification
```sh
test -f docs/design/KEYBED_EXPRESSION_REFERENCE.md && grep -c '\[V\]\|\[I\]' docs/design/KEYBED_EXPRESSION_REFERENCE.md
```

## Out of scope
- Writing firmware; flashing.

## Log

### 2026-11-10 executor (sonnet)
- Searched web (synthtopia, matrixsynth, zeli.app, awesome-fm-1 list); cloned read-only into /tmp/p1015_*: fm1_open, FM-1-RE, fm1-fomni, awesome-fm-1, fm1-velocity, fm1-monk, scintilla-fm1, mvave-fm1-mpe. Nothing run, nothing cloned in the repo.
- Found: Velo (charlesvestal/fm1-velocity) reads velocity + poly aftertouch via ADC; no true MPE exists. Details in docs/design/KEYBED_EXPRESSION_REFERENCE.md.
- Verification: `test -f docs/design/KEYBED_EXPRESSION_REFERENCE.md && grep -c '\[V\]\|\[I\]' ...` printed 16.
- Limits: Elektronauts thread and FM1 Open app sources not read in depth; Scintilla sensing code not read.

### 2026-11-10 review (sonnet)
Verdict: PASS (gate none), with one material gap fixed in place by the reviewer.
- Re-cloned charlesvestal/fm1-velocity (260b978, 2026-11-07) into a fresh /tmp dir, read-only. Verified: GPL-3.0 LICENSE; README "working; parked 2026-10-14"; notes/01-lab.md table (PA7=ch0 notes 9..19, PA8=ch1 notes 0..8 + OCT, PA10 LED line for top row, notes 20..26 no ADC); ~2.2 ms sampling per key; divider 7 / delay 2; chord 59/76/127/104 (lab.md:92); quartiles 40/59/74 (README); ui.c aftertouch() sends 0xA0 or 0xD0, no pitch bend or CC74 anywhere.
- This repo: firmware/hal/fm1_input.h has no analog hook (digital scan, note_t0, press_*); fm1_adc.h is a polled blocking reader for PB6 MASTER (ch4) / PB1 battery (ch3), used by master.c and main.c. Confirmed. The "hal/" path in the doc means firmware/hal/.
- monch1962/mvave-fm1-mpe README: MPE input from an external controller. Confirmed.
- GAP (fixed): executor missed CTL-1 (charlesvestal/fm1-ctl, 2a9f92b, MPL-2.0), the shipped successor with a Touch page and practical caveats (edge presses read soft, top 7 keys no aftertouch, 5 ms note delay, LEDs disturb neighbouring key readings). Added to section 1 and the budget note. Also added ADR-0003 numbers (46496 B RAM headroom, +2257 B core) which the recommendation lacked; Q53/54 reworded in OWNER_QUESTIONS.md. Conclusion (digital gestures v1, force later) still stands and is strengthened.
- Minor, left: "~60 B x 27" per-key state is an unmeasured estimate (tagged [I]); Elektronauts thread and FM1 Open sources unread, disclosed.
- Touched only P1-015 files (doc, task, OWNER_QUESTIONS rows 53-54, BACKLOG index).

---
target: UI proposal v3.2 (FM-1 240x240 screen)
total_score: 27
p0_count: 0
p1_count: 3
timestamp: 2026-11-12T05-44-07Z
slug: s-design-ui-review-proposals-v3-2-proposal-v3-2-py
---
# Critique: UI proposal v3.2 (FM-1 240x240 screen, 30 mm wide)

Sources: docs/design/ui_review/critique_v3_2/ASSESSMENT_A_v3_2.md (independent design review, 27/40) and EVIDENCE_B_v3_2.md (measurements). Detector scanned zero files (non-scan).

Score 27/40 (Acceptable), up from 24 on v3. Density verdict: does not regress to too much information / too small (no AF_S on any glance screen; 28 strings in 11 roles while playing; map A has 22 AF_S strings).

## Priority issues
- [P1] Lane bars show velocity, not pitch: dicing NOTES/SHAPE changes nothing visible; soft notes look like rests; velocity 1-3 bars identical.
- [P1] Channel setup (CHN key) never drawn: cue says CHN K2 = MIDI CHANNEL while the knob card still shows SLIDE; a MIDI channel steal sets another channel to OFF without naming it.
- [P1] Risky moments mislabelled or unflagged: preset toast says SAVE: KEEP HERE but SAVE overwrites the pending slot; CLOCK LOST only in a transient cue (header identical to internal clock); dice hint drawn in 14 of 36 toasts; selected+muted+MIDI OFF channel accepts REC with no warning.
- [P2] Map accuracy and layout: MAP B omits PRV1..4 holds and the RIG hold; MAP A labels OCT+ ROW+ and HOME CANCEL/MENU; buttons arranged 4x3 vs the panel 6x2; map A printed names AF_S in MID grey; MENU timing (1.5 s + countdown) conflicts with button_modes.tsv (release after 0.7 s) - spec must be updated.
- [P2] Playing column: accent cap and bar both BG; ratchets 5-8 pixel-identical to 4; slide chain stripe; EXT lost header identical to internal.
- [P2] One 4-column grid shared by channels, steps and knobs reads as related columns; vertical gaps 1,1,1,5,2,2,2 px; muted numeral contrast under 4.5:1 on 6 of 9 palettes; 24 of 79 non-text pairs under 3:1 on GREY.
- [P3] Mock bugs: lane_muted shows CH1 muted while cue says CH2; cue_mute_held shows tap text; state_first_run lane full of edited steps; 5 cue strings in no render; cues missing for GLO hold, HOME hold, CHN hold, STEP tap, EDIT hold, both-OCT hold.

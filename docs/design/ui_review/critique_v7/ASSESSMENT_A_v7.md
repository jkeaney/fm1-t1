# Assessment A: v7 main screen (PERF)

Reviewer: independent design director. Inputs: PRODUCT.md, HANDOFF 01/02, panel photo, the four role tables, the seven
v7 renders (1x and 4x), and the drawing code (read only to settle what a mark means). No designer notes were read.

Viewing math used throughout: 8 px/mm, 35 cm, `arcmin = mm / 350 * 3438` (1 px = 0.125 mm = about 1.2 arcmin).
Pixel budget of the screen as drawn: header 0-20, knob tiles 24-76, row marker 80-85, music row 90-124,
buttons 132-182, keybed footer 188-240.

**Score: 20 / 40 (Nielsen). Verdict: not ready to lock.** The skeleton is right (the screen is a top-to-bottom map of
the panel, and every control has a place on it). Execution fails on the three things the product says matter most
at a glance: what is muted, where the playhead is, and what each dice button does. The workflow test also fails
at step 1, because the MIDI channel is not on the screen.

---

## 1. AI-slop / generic-look verdict

This is not generic slop. Its structure comes from the hardware: four knob tiles, a 2x6 button block, and a keybed
drawn the way the real keys sit (3-2-3-2-1 black keys). Nobody would get that from a template. But a lot of the
surface is stock. The house, floppy disk and gear are default UI icons, and the gear in particular says
"settings/menu" on a button whose job is a performance page (SET: BPM, swing, gate, div). The full-width white
toast slab and the filled "progress card" knob tiles read like a phone app, which is one of the anti-references. The
brown meter fill (orange at about 40%, (106,60,16)) looks muddy rather than instrument-like. Four same-tint
yellow-framed dice also read at a glance as "icon soup": you can tell they are a group, but not which one is which.
In short it is bespoke in layout and generic in its marks, and the marks are where it fails.

## 2. Nielsen's 10 heuristics (0-4)

| # | Heuristic | Score | Evidence |
|---|---|---|---|
| 1 | Visibility of system status | 2 | Transport, BPM, preset, the selected channel and the knob values are clear, and the toast and turning popup confirm actions. But the MIDI channel, the pattern length, mute state in the header and undo/redo state are all missing, and the playhead is a 2.1:1 grey column. |
| 2 | Match with the real world | 3 | The screen is stacked in panel order, the footer looks like the keybed, the accent mark `>` and ▶ ■ ● are real conventions. Against that: bar height reads as velocity (the producer's convention) when it means pitch, the gear means "settings", and the house means "home page" on a device whose HOME tap is a no-op on PERF. |
| 3 | User control and freedom | 2 | Undo has a visible icon on EDIT, and the toast says `EDIT=UNDO`. Redo/undo state, RESET (EDIT hold), undo of a preset load (SAVE hold) and cancelling a pending preset are not visible. |
| 4 | Consistency and standards | 1 | The ● glyph means three things: header "stopped" (grey; the code literally draws the `rec` icon), REC button (white, then red), REC footer (orange). The header shows state (▶ = playing) while the PLAY tile shows the action (■ = press to stop). Knob meters change style (full fill becomes a thin bar) while any knob turns. The row marker (□■□) and `PAGE 1/2` are two position markers that look alike and mean different things. |
| 5 | Error prevention | 2 | The pending preset `Pnn?` frame exists in code but is not in any scene. There is no warning that a channel is sending on an unseen or OFF MIDI channel. The destructive holds (RESET, UNDO L) are unmarked. |
| 6 | Recognition rather than recall | 1 | Icon-only dice with 9x9 px inner glyphs (SHAPE `~`, ARTIC "tray", HITS "···"), `S F R` single letters, a 2 px four-dot CHN glyph, an unlabelled 5 px row marker, and four undocumented hold roles all require a manual or memory. |
| 7 | Flexibility and efficiency | 3 | Every performance action is one gesture with no page change, and the keys show their live mode. Missing: no shortcut cue for OCT row peek (the OCT buttons are not on screen at all). |
| 8 | Aesthetic and minimalist design | 2 | Five bands at similar weight. The biggest, brightest type (AF_L knob values, 22% of the height) is mostly static reference, while live state (the playhead and the 2-3 px bars) gets the weakest marks. That inverts principle 3. |
| 9 | Recognise, diagnose, recover from errors | 2 | The dice toast names the result and the recovery (`EDIT=UNDO`) and the diced steps are hatched, which is good. No error states are shown at all (MIDI clash, past-the-end step, MIDI OFF). |
| 10 | Help and documentation | 2 | The footer caption (`KEYS = STEPS  HOLD = EDIT`) is good inline documentation, but nothing tells a first-timer that holding HOME shows the full map, which is the whole basis of the "map on demand" architecture. |
| | **Total** | **20/40** | |

## 3. Cognitive load checklist

| # | Item | Result | Note |
|---|---|---|---|
| 1 | The screen splits into few chunks (at most about 6) | PASS | 5 bands, each matching a panel region. |
| 2 | Related items are grouped by proximity and mirror the controls | PASS | Knobs 4 across, buttons 2x6, keys as keys. |
| 3 | Live state dominates the hierarchy | FAIL | Knob values (AF_L, white on brown) outrank the playhead (2.1:1 grey) and mutes (1 px hollow outlines). |
| 4 | Each mark has one meaning | FAIL | ● x3, grey = muted bar / playhead / empty-step dash / unused key, and the small squares (row marker vs mute key) look alike. |
| 5 | Channel identity has a code beyond colour | FAIL | In the music row a channel is identified only by colour; its x-position inside a step moves with how many channels play there. |
| 6 | Controls are recognised, not recalled | FAIL | Dice layers, S/F/R, CHN, the row marker, holds. |
| 7 | Transient feedback appears where the eye already is | PASS (partial) | The turning popup sits on the knob, and the dice toast plus hatching sits on the row, but the toast takes the header away. |
| 8 | Nothing the user must read mid-performance is under ~10 arcmin | FAIL | Bars 2.5 arcmin wide, mute hollow 1.2 arcmin walls, inner dice glyph strokes 1.2 arcmin, row marker 6 arcmin. |

**Density verdict:** the amount of content is right (the owner wants everything documented), but the pixels are
allocated wrongly. The knob band (52 px) shows four numbers that change only when touched. The music row (34 px)
carries up to 64 live events and the playhead. The button block (50 px) spends 24 px of height per tile on icons whose
distinguishing part is 1 px strokes. Moving about 10-12 px from the knob tiles to the music row, and using the button
tiles for words rather than pictures, would fix more than any restyle would.

## 4. Workflow test from the screen alone

| Step | Can the user tell what to press? | Can they see it worked? | Result |
|---|---|---|---|
| 1a Select channel 1 | Partly. The `1 2 3 4` chips imply a channel selector, but nothing ties them to the SELECT encoder (the header does not echo the left block's layout). | Yes, the filled chip, the coloured algo name and the brown meters all change colour. | PARTIAL |
| 1b Set its MIDI channel | No. The MIDI channel is not on the PERF screen in any scene. The only route is the CHN black key, which is drawn as four 2 px dots (2.5 arcmin) with no word. | No. There is nothing to verify against. | **FAIL** |
| 2 Pick an algorithm | Partly. `ACID` with its glyph sits in the header, but no scene shows ALGO being turned (the ALGO_PICK cue). The user must guess that the left-hand ALGORITHM encoder drives it. | The name changes in the channel colour. | PARTIAL |
| 3a Dial in with knobs | Yes. The tiles are named and carry values, and the popup names `K2 SLIDE 4 ▸ 6`. | Yes. | PASS |
| 3b Dial in with dice | No for 4 of 6 buttons: ALL (pips) and NOTES (♪) are guessable, but ACCNT (`>`), SHAPE (`~`), ARTIC and HITS are not at 1x. | Yes. A good toast (`NOTES DICED`), the inverted tile and hatched steps. Press-to-learn is safe because dice is undoable. | PARTIAL |
| 4 PLAY | Yes, ▶ sits on the PLAY position. | ▶ in the header, ■ on the tile, and the playhead moves (weakly). The stopped state shows a grey ● that reads as "record". | PASS (with a flaw) |
| Morph | Yes. | Yes, but the popup covers K3, so a two-handed K2+K3 morph loses the K3 readout. | PASS |
| Dice | As 3b. | Yes. | PARTIAL |
| Mute | Partly. The first four black keys carry coloured squares, but the word "mute" appears nowhere. | Weak. The muted channel becomes 2 px grey hollow bars (a 2 px hollow rect renders solid) and a 6 px hollow square; the header chip is unchanged. | PARTIAL (glance FAIL) |
| Switch channel | As 1a. | Yes. | PARTIAL |
| Undo | Yes, the undo arrow on EDIT, reinforced by the toast. | Only through the music row changing back. No redo state, and the icon never flips. | PARTIAL |
| Change preset | Partly. `P01` is shown, but nothing says PRESETS turns it; the pending `P02?` state is not in any scene, and SAVE-hold "undo load" is invisible. | Unverified, because no scene shows it. | PARTIAL |

Net: 2 PASS, 1 FAIL, the rest PARTIAL. The acceptance test ("from the screen alone") does not pass.

## 5. Element-by-element verdicts

**Header (y 0-20).** Good: transport + BPM at the left, preset at the right, AF_M at 13.7 arcmin, the selected chip
filled (a shape cue, not only colour). Bad:
- No MIDI channel (see P0-1).
- No mute state on the chips: CH3 muted looks the same as CH2/CH4 in `v7_playing`.
- The stopped icon is the REC glyph in grey (`v7_two_channels_stopped`, top-left).
- The `+` (next channel) has no visible control that adds a channel.
- With channels 1, 2 and 4 in use, a hole is left where 3 would be and the `+` disappears (`v7_turning_k2`). The
  positional logic is defensible, but it reads like a rendering bug.
- The algo glyph before `ACID` is about 10 px and decorative.

**Knob tiles (y 24-76).** The "tile is its own meter" idea is sound and the AF_L values are the most legible thing on
the screen. Execution:
- The fill edge cuts through the label whenever the value is about 70-90% (`v7_playing`, DENS 12/16: the brown edge
  crosses the lower third of `DENS`).
- The meter has no scale, so the fill only says "about three quarters".
- The tiles take the top visual rank although they are static.
- The brown is muddy, and with four channels the meter colour changes on every channel switch, which is good as a
  selected-channel cue but makes the band flash.

**Row marker (□■□ at y 80).** It fails. It is three 5 px squares (6 arcmin) with no word, sits between two bands, and
reads as a page indicator for the music row beneath it. It also always shows three rows, although ROW 3 exists only
when the channel has a RIG device. The OCT-/OCT+ buttons it belongs to are not drawn anywhere.

**Music row (y 90-124).** The idea works for one or two channels (`v7_one_channel` is the best image of the set):
accents are white caps, rests are dashes, and there are beat ticks every 4 steps. It collapses at four channels:
- The bars are `(13-(k-1))//k` = 2 px wide (2.5 arcmin).
- A channel's slot inside a step moves with how many channels play there, so colour is the only identity
  (principle 4 broken).
- A muted channel's hollow outline at 2 px width is drawn solid grey.
- Height is normalised per channel to that channel's own min..max, so a bass line moving a semitone looks like a lead
  jumping two octaves, and heights cannot be compared across channels.
- The playhead is a (65,68,65) column at 2.1:1 on black that looks like one more grey bar (step 6 in
  `v7_playing`).

**Button icons, one by one** (13 px icons in 38x24 tiles; the inner glyph is 9x9 px = 11 arcmin outer extent with
1 px = 1.2 arcmin strokes):
- FX / ALL, die with pips: OK. It is a literal die, so "re-roll everything" lands.
- SCL / NOTES, die + ♪: OK. It is the most recognisable.
- ENV / ACCNT, die + `>`: musicians will get it on a 4x zoom, but at 1x the `>` is two diagonal 1 px strokes.
  Marginal.
- LFO / SHAPE, die + `~`: misleading. A sine on the LFO button says "LFO" (the printed word), not "register jumps,
  slot lengths, drift".
- ARP / ARTIC, die + bar + dot (tenuto/staccato?): clever on paper, unreadable at 1x, and it looks like a tray or a
  minus.
- SEQ / HITS, die + `···` with a bar: unreadable. It looks like a keyboard or a menu.
- EDIT / UNDO, ↶: good, but no redo state and no cue for hold = RESET.
- GLO / SET, gear: wrong metaphor (it opens a performance page; MENU lives on HOME hold). Hold = HARMONY is
  invisible.
- HOME, house: fine as a word substitute, but useless on PERF, where a tap does nothing. Its important role (hold =
  map/help/menu) is unmarked.
- SAVE, floppy: fine. Hold = undo load is invisible.
- PLAY, ▶/■ as the action: fine on its own, but inconsistent with the header showing state.
- REC, ●: fine, but armed is shown by colour alone (white to red).
- The dice-group tint (49,40,16) against the grey tiles is a good group cue.

**Keybed footer (y 188-240).** It is a good idea and the best-executed piece after the knob values: it shows the 16
steps of the selected channel and the black-key roles in place, and its REC transformation is the clearest mode change
on the screen. Faults:
- The off-step keys are (32,32,32) on black (1.29:1), so the keyboard silhouette nearly vanishes when steps are off.
- The playhead's white double outline replaces the step's orange fill (step 6 is on in the row but hollow on the key).
- The black-key glyphs `S F R` (AF_S, single letters), the 2 px four-colour CHN dots and `|◀` (restart, which reads as
  "previous track") are cryptic.
- `PAGE 1/2` is in AF_S grey in the far corner, 70 px from the row it describes.

**Toast (dice).** The content is right (`♪ NOTES DICED   EDIT=UNDO`), the hatch on the changed steps is excellent, and
the inverted tile confirms the button. But the white slab is the loudest object ever drawn on this screen and it hides
transport, BPM, channels and preset. During a burst of dicing the header is effectively gone.

**Turning popup.** It is clear and well built: the name with its knob number, old ▸ new, a scale with ticks, and the
bar. It is placed wrongly. It covers the neighbouring tile (K3 here), and it switches the other tiles to a different
meter style (a thin bottom bar), so the whole band jumps on the first detent. Because the tile is already a meter,
the popup could live inside the tile.

**REC state.** The footer becomes a light piano with `● ALL KEYS PLAY NOTES`, which is good and unmissable. But the
header has no REC marker, the REC tile changes by colour only, the footer dot is the channel colour (orange) and not
red, and the playhead outline remains on the white keys, where it now reads like a held key.

## 6. Viewing distance at true 1x (35 cm)

| Mark | Size | Arcmin | Verdict |
|---|---|---|---|
| Knob values (AF_L) | 21 px | 25.5 | Excellent |
| Header text, knob labels, chips (AF_M) | 11 px | 13.7 | OK |
| Footer captions, `S F R`, `PAGE 1/2` (AF_S) | 9 px | 10.8 | Borderline. PAGE is grey and in a corner. |
| Dice inner glyph | 9 px extent, 1 px strokes | 11 / 1.2 | Shape not resolvable; the four "unknown" dice blur together |
| Row marker squares | 5 px | 6.1 | Unreadable as a state |
| Mute square on the black key | 6 px, 1 px wall | 7.4 / 1.2 | Filled vs hollow barely distinguishable |
| CHN glyph dots | 2 px | 2.5 | A speck |
| Music bars, 4 channels on a step | 2 px wide | 2.5 | Colours merge into one muddy block |
| Accent cap | 2 px tall, 1 px gap | 2.5 | Visible only on a full-width bar |
| Beat ticks under the row | 1x2 px | 1.2-2.5 | Invisible |
| Playhead column | 15x34 px at 2.1:1 | (contrast-limited) | Visible only on close attention |

The 1x render `v7_playing.png` confirms it: the knob numbers and footer orange are what read, and the row is a
coloured texture.

## 7. Personas (one red flag each)

- **First-timer:** four yellow dice that look alike. They can learn by pressing (the toast tells them), but "knows what
  each control does without a manual" is not met, and nothing tells them that holding HOME shows a map.
- **Expert live performer:** in a four-channel set, the answers to "is CH3 muted, and where am I in the bar?" are 2 px
  grey bars and a 2.1:1 column, and the popup hides K3 during a two-handed morph.
- **Colour-blind (deuteranopia; simulated):** CH1 orange becomes (207,185,41) and CH3 green becomes (219,199,91), a
  luminance ratio of 1.16, effectively the same colour. CH4 pink becomes neutral grey (169,170,177), next to the grey of
  muted bars and the playhead. In the music row, colour is the only channel code.
- **Dana (one TB-3 on MIDI ch 2):** bars move and the playhead runs while the TB-3 stays silent, and the screen never
  says which MIDI channel CH1 sends on. Nothing anywhere on PERF says "MIDI", and there is no out-activity mark.

## 8. Priority issues

**P0 (blocks use)**

- **P0-1, MIDI channel absent from PERF.** (All images, header.) Workflow step 1 fails, and Dana cannot diagnose
  silence. *Fix:* add the selected channel's MIDI out to the header in AF_M, for example `▶120 [1]2 3 4  ACID →2
  P01`. Reclaim the width by dropping the decorative algo glyph (about 13 px) and tightening the chips. Show `→OFF` in
  T_REC when the channel is OFF after a clash. Rename the CHN black-key glyph to a letter that means channel/MIDI and
  caption it (see P1-6).
- **P0-2, mute state not readable at a glance.** (`v7_playing`: header chip 3, music-row grey bars, black key 3.)
  Principle 1 names "what is muted" as glance-critical, yet the screen shows it with 1 px walls. *Fix:* (a) the muted
  channel's header chip is drawn struck through or as a dim filled block with an `x`, so the shape changes; (b) in the
  music row a muted channel keeps its slot but draws a 1 px baseline dash in grey instead of a bar (so it cannot be
  mistaken for a narrow bar); (c) the black mute key is drawn fully filled grey with a diagonal, not as a 6 px
  outline.

**P1 (must fix before lock)**

- **P1-1, the music row loses channel identity and scale at 3-4 channels.** (`v7_playing`, `v7_dice`, row.) *Fix:*
  give each channel in use a fixed slot per step (3 px minimum; with 4 channels: 4 x 3 px = 12 px, leaving a 3 px gutter in the 15 px step pitch; separate adjacent
  slots by colour plus the dimming below, not by a gap),
  so a rest leaves a gap and position encodes channel. Draw the selected channel's bars full-saturation and the others
  at a dimmed value. Put height on a shared pitch scale (for example, the selected channel's range applied to all, or
  absolute within 3 octaves), not per-channel min..max. Keep the owner's "split into bars" idea, but make it stable.
- **P1-2, the playhead is the weakest live mark.** (`v7_playing` row step 6; footer key 6.) *Fix:* a white 2 px
  bracket or tick above and below the playhead step in the row, instead of a 2.1:1 grey slab. On the keybed, draw a
  white bar under the key instead of an outline, so the on/off fill stays visible.
- **P1-3, dice layers cannot be recognised at 1x.** (`v7_icons_4x`; buttons FX SCL ENV LFO ARP SEQ.) *Fix:* keep the
  die frame as the group mark (a small 7 px die at the tile's top-left, or the tint alone) and write the role word in
  AF_S in the tile: `ALL NOTES ACCNT SHAPE ARTIC HITS` (they fit 38 px under the 6-character rule; check ACCNT/SHAPE
  in `widths.tsv`). Keep icons for the universal ones (undo, save, play/stop, rec). If the owner insists on icon-only,
  at least replace `~`, the tray and `···`, and make the 4x review at 1x.
- **P1-4, three meanings of ●, plus state vs action.** (Header `v7_two_channels_stopped`, REC tile, REC footer.)
  *Fix:* header stopped = grey ■; REC armed adds a red `REC` word or a red frame in the header; the footer REC dot is
  red, not the channel colour. Decide whether the PLAY tile shows state or action, and make the header the opposite
  only if it is labelled.
- **P1-5, the turning popup covers K3 and restyles the other tiles.** (`v7_turning_k2`.) *Fix:* render the turning
  state inside the turned tile: a channel-colour frame, `4▸6` replacing the value, and the meter staying in the same
  style. If the old/new values need more room, borrow the music-row space below the tile, not the neighbouring knob.
- **P1-6, hold roles and cryptic black-key glyphs are undocumented.** (Buttons EDIT GLO HOME SAVE; footer black keys.)
  *Fix:* a 3x3 px corner notch on every tile that has a hold role, explained by the HOME-hold map. Replace the footer
  caption line with a group caption aligned under the black-key groups, for example `MUTE  CH  SOL FIL ROL  PAGE
  TOP` in AF_S, and move `KEYS = STEPS` onto the key fronts' context (it is self-evident once on-steps light up).
- **P1-7, the knob label collides with the meter fill.** (`v7_playing`, DENS tile.) *Fix:* cap the fill at the value
  band (never above y+20) or move the meter to a 4 px bar along the tile's bottom edge in every state (which also
  fixes the style jump in P1-5).

**P2**

- P2-1, the row marker is unlabelled and looks like a page marker. *Fix:* write `R1` (AF_S) at the right end of the
  knob band, show `R3` only when RIG exists, and tie it to OCT in the HOME map.
- P2-2, `PAGE 1/2` is far from the row it describes and there is no pattern length. *Fix:* move `1/2` and the length
  (`16`) into the row-marker gap above the music row.
- P2-3, the toast hides the whole header. *Fix:* cover only the chips and algo region (x 48-195) and keep transport
  and preset visible; or use an inverted band over the music row's top 12 px.
- P2-4, the off-step keys are nearly invisible (1.29:1). *Fix:* raise T_RAISE on the keybed to about (56,56,56).
- P2-5, the header `+` has no control, and a gap appears when channels are non-contiguous. *Fix:* drop `+` from the
  glance screen, or caption it in the HOME map; draw unused channel positions as 1 px dim dots so the gap looks
  intentional.
- P2-6, no undo/redo state. *Fix:* flip the arrow (↷) when the next tap redoes.

**P3**

- P3-1, the gear on GLO suggests settings or menu. Use a metronome, or `SET` in AF_S.
- P3-2, the brown meter tone is muddy. Use a darker, cleaner channel tint, or a neutral grey fill with a channel-colour
  top edge.
- P3-3, the algo glyph in the header carries no information beyond the name.

## 9. What works

- The vertical order mirrors the panel (encoders, then knobs, buttons and keys), and the 4 / 6 / 16+11 columns match
  the hardware.
- The AF_L knob values are the best type on the screen.
- `v7_one_channel` shows the music row's potential: clean, rhythmic and readable.
- The dice feedback chain: the inverted tile, a toast naming the layer and the undo, and the hatched changed steps.
  This is the best interaction on the screen.
- The keybed footer that changes with the mode, especially the REC piano and its plain-English caption.
- The dice-group tint separates "perform" buttons from "transport/system" buttons.

## 10. Questions for the owner

1. May the MIDI channel of the selected channel live in the header permanently (it costs the algo glyph), or must it
   stay on the CHN page? If the latter, how is Dana meant to see it?
2. Will you accept a short role word on the six dice tiles (keeping the die as the group mark), given that the inner
   glyphs measure 1 px strokes at 35 cm?
3. In the music row, should "height = pitch" be comparable across channels, and is it acceptable that a channel keeps
   a fixed slot in every step (leaving gaps where it rests) so that position, not only colour, says which channel it is?

# 1. Architecture and data flow

## What runs where **[V]**

| Layer | Source | Runs on the FM-1? | Notes |
| --- | --- | --- | --- |
| Algorithms (Euclid, pace, harmony, LFSR, p-locks) | `torso_t1_algo.c` | **yes** | pure functions, integer only |
| Per-tick pipeline (step counters, pulse → events, dispatcher, FM voice allocator) | `torso_t1_pipeline.c` | **yes** | the engine the device really uses |
| Clock, transport, track-1 and USB-MIDI glue, default pattern | `t1_glue.c` | **yes** | included into `felucca.c` after `seq.c` |
| State: [TEMP], [GLOBAL] | `torso_t1_state.c` | **yes** | flash functions in the same file are **not** called |
| UI model: pages, encoders | `torso_t1_ui.c`, `ui_t1.c` | **yes** | screen and controls while T1 mode is on |
| Timer ISR, micro-queue dispatcher, second voice allocator, task table, timing stats | `torso_t1.c` | **compiled in, never called** | host-test code only |
| Flash save/load (`t1_flash_*`) | `torso_t1_state.c` | **compiled in, never called** | "Nothing here touches the flash" (`t1_glue.c`) |

The device build includes all of these into one translation unit (`felucca.c` lines 78 to 96, with `T1_DEVICE` defined). A search of the firmware shows no caller of `t1_timer_192ppqn_isr`, `t1_micro_queue_dispatch`,
`t1_fm_voice_alloc`, `t1_timer_set_bpm`, `t1_midi_worker_step` or any `t1_flash_*` function outside their own file. So the design document's
"FreeRTOS tasks, a 192 PPQN hardware timer ISR and TinyUSB queues" do not exist on the device; they are a host-side model.

## Clock **[V]**

The T-1 is clocked from the **audio ISR**, not a hardware timer:

```
each audio block of n samples (CTL = 32 samples ≈ 0.73 ms at 44.1 kHz):
    t1_acc += n * 192                        # PPQN_RATE × samples
    while t1_acc >= beat_samples():          # one beat = beat_samples() samples
        t1_acc -= beat_samples()
        t1_pipeline_tick(t1_tick++)          # one 192-PPQN tick
    flush USB-MIDI
```
- `beat_samples()` follows the FM-1's own tempo (BPM knob = SELECT in T1 mode) **or an external MIDI clock** as `seq.c` follows it. External clock loss for more than 500 ms stops playback.
- Tick 0 fires on the first block after PLAY (`t1_acc` is pre-loaded). Every tick in a block runs back to back at the start of that block, so **timing resolution is one audio block (~0.7 ms)**; at 300 BPM a tick is 1.04 ms, so up to two ticks can fall in one block.
- While T1 mode is on, Felucca's four pattern tracks are not ticked (`seq_tick` is skipped) and the keys are silent.

## Data flow per tick **[V]**

```
 t1_pipeline_tick(tick):
   for each of 6 tracks (skip muted):
       cycle = tracks[t].cycles[active_cycle]
       step counter:  advance when the current step's length (division, swing) has elapsed
       on a new step:
           pulse?  = Euclidean mask[step]                         → 03
           chance  = probability (+ p-lock) vs LFSR roll          → 06
           pitch   = 60 + offset + phrase + style (+ random) → quantise → chord → voicing   → 05
           velocity= base (+ p-lock, random, accent)              → 06
           repeats = 1..16 hits, pace-warped timing, velocity slope, gate                → 04
           push NOTE_ON / NOTE_OFF (and a CC) into the micro-queue, time-ordered         → 08
   dispatch every queued event due at or before this tick:
       track 0  → voice allocator → synth_note_on/off  → Felucca track 1
       tracks 1-5 → MIDI bytes → ring buffers → USB-MIDI (UART ring is drained and discarded)
```

## Output routing **[V]**

| T-1 track | Destination on the FM-1 | Channel |
| --- | --- | --- |
| 1 (index 0) | Felucca **track 1**: its engine and preset (`trk_note_on/off`) | none |
| 2 to 6 (index 1 to 5) | **USB-MIDI OUT** only | `midi_channel` (default index 1 to 5 → MIDI channels 2 to 6) |

The FM-1 has no MIDI OUT jack, so the "TRS-MIDI UART" in the design is not wired: `t1_pipeline_flush_midi(0, t1_usb_out)` passes a null UART callback, and the UART ring is simply emptied.
The device boots with a **default pattern** (`t1_default_pattern`): track 1 silent, track 2 a generative bassline on channel 2. As first reviewed it played A Locrian over MIDI 22 to 115 (findings F2 and F4); after the fixes it plays A minor pentatonic over about MIDI 21 to 57.

## Memory **[V]**

| Item | Size |
| --- | --- |
| `CycleParams` | 128 B (static-asserted) |
| `Track` | 776 B (6 cycles + 8 B) |
| `Pattern` | 4,672 B (6 tracks + 16 B) |
| `SequencerRAM` (active + snapshot) | 9,344 B + 4 B |
| Micro-event queue | 256 events (12 B each ≈ 3 KB) |
| MIDI ring buffers | 2 × 256 B |

On the device these live in the spare pool section (`.pool`), not the 72 KB main RAM.

# TIMING_SPEC: tight MIDI timing from any clock source, with the source always visible (P2-010)

Status: **proposed**, for the owner's human-review gate. Nothing here is approved until the owner writes it in the
P2-010 log. Design only: no firmware is changed by this task. It amends ADR-0003 section 7 (addendum at the end of
that ADR) and SCREEN_SPEC section 6 (rewritten to the owner rulings of 2026-11-10). Locked UX
([SEQUENCER_UX.md](SEQUENCER_UX.md), P1-017) is not changed: SET K1 stays inert while an external clock drives.

Trigger: notes recorded into Ableton from the FM-1 looked loose (owner, 2026-11-10). Owner requirement: "make sure
our code has all the tools to ensure timing is accurate, no matter where the clock is set, and that it's really
obvious to the user the clock and where it's coming from."

Citations are `file:line` in `firmware/src/` unless a path is given. **[V]** verified by reading the code (or by the
probe in section 1.1), **[I]** inferred, **⚠** unconfirmed until it is measured on the device (the gate in section 7).

## 0. Summary for the owner (read this first)

1. **Why it sounds loose, today.** The sequencer decides *when* a note happens to within one control block
   (0.73 ms), but the audio interrupt renders four blocks back to back (one 2.90 ms half buffer) and the USB
   transmitter only runs after that interrupt has finished, on a 0.5 ms poll. Every note of a half therefore leaves
   in the same burst. A note's position on the wire is "end of its half buffer" instead of "its own time": about
   **3.4 ms peak-to-peak jitter on the FM-1 side**, plus the USB frame (1 ms) and the host. ADR-0003 7.1 claimed
   "at most one block (0.73 ms)"; that claim is wrong and is corrected here (section 8).
2. **A second, slower fault: the internal tempo is 400 ppm fast.** Tempo is counted in samples assuming 44100 Hz
   (`fx.c:221`), but the I2S clock runs at 24 MHz / 544 = 44117.6 Hz (`audio.c:9`). That is **+0.04 %**: 24 ms
   gained per minute at any BPM, 0.24 s over a ten-minute take. A DAW recording a free-running FM-1 drifts out of
   grid even if every note were perfect. ⚠ The codec rate is read from a comment; the test build measures it.
3. **The fix is three mechanisms, none of which changes the musical design:** (M1) place each tick at its exact
   sample inside the block; (M2/M3) stamp every outgoing event with the TIMER4 (24 MHz) time it is *due* and let the
   10 kHz TIMER5 interrupt release it on time, instead of flushing per block; (M10/M11) correct the tempo constant and
   follow an external clock through a phase-locked loop on 41.7 ns timestamps instead of 1 ms ones.
4. **Targets** (section 2): FM-1-side jitter at most **0.25 ms** peak-to-peak (from about 3.4 ms), jitter at the
   host at most **1.5 ms** peak-to-peak (dominated by the 1 ms USB frame), tempo error at most **100 ppm** (from
   400 ppm), clock out exactly 24 pulses per quarter with the same accuracy, zero long-term drift when following.
5. **Clock source** (section 4): AUTO as the owner ruled. Internal by default; a heard host clock becomes the
   source; silence returns to internal at the user's own tempo, which is never overwritten.
6. **Visibility** (section 5): a blinking dot in the header on every screen. Amber solid = internal, cyan with a
   centre hole = external, hollow blinking = waiting for the host to start.
7. **Proof** (section 6, 7): a timing-test firmware image plays a fixed pattern; a tool measures jitter from
   timestamped MIDI; the hardware gate has numeric pass thresholds.
8. **Cost** (section 9): about 0.6 KB of `.bss` over ADR-0003's out ring (table in section 9), nothing in the normal build for the test
   mode, four dev tasks.

## 1. Facts, measured or derived

### 1.1 The clock grid (the probe)

A probe compiled against the generated tables (`build/gen/felucca_tables.h`, from `tools/gen_tables.py:13-14`) prints:

```
FS=44100 CTL=32 HALF_FRAMES=128 blocks/half=4
block nominal 0.7256 ms (725.6 us); actual-rate 0.7253 ms
half nominal 2.9025 ms; actual 2.9013 ms
DAC rate 44117.647 Hz, ratio 1.000400, error 400.2 ppm   (24 MHz / DAC_TICKS 544, audio.c:9)
bpm  40 beat_samples  66150 tick 7.812 ms = 344.5 samples, ticks/block 0.093, pulse(24ppq) 62.500 ms
bpm 120 beat_samples  22050 tick 2.604 ms = 114.8 samples, ticks/block 0.279, pulse(24ppq) 20.833 ms
bpm 240 beat_samples  11025 tick 1.302 ms =  57.4 samples, ticks/block 0.557, pulse(24ppq) 10.417 ms
drift at 120 BPM: 24.01 ms per min
```

| Quantity | Value | Source |
| --- | --- | --- |
| Control block `CTL` | **32 samples = 0.7256 ms** at 44100 Hz (0.7253 ms at the real DAC rate) | `tools/gen_tables.py:14`; `fx.c:929` calls `events_block(n)` once per block |
| I2S half buffer | **128 frames = 2.9025 ms = 4 blocks**; one audio interrupt renders the 4 blocks back to back | `core.h:15`, `audio.c:116-120` |
| Real sample rate | 24 MHz / 544 = **44117.65 Hz**, +400 ppm against the `FS` used for tempo ⚠ | `audio.c:9`; `fx.c:221` uses `FS` |
| 192-PPQN tick | 2.604 ms at 120 BPM (114.8 samples); **at most one tick per block** from 40 to 330 BPM (0.557 ticks per block at 240) | probe; `t1_glue.c:133-137` |
| 24-PPQN clock pulse | 20.833 ms at 120 BPM = 8 ticks | probe |
| `fm1_ms` (the clock used for input stamps) | 1 ms steps, counted in the TIMER5 interrupt from the 24 MHz TIMER4 | `main.c:24-35`, `usb.c:187` |
| `fm1_ticks()` | TIMER4 count, **24 ticks per microsecond** (41.7 ns), 32 bit (wraps every 179 s: compare differences only) | `hal/fm1_time.h:18,32` |
| TIMER5 | 10 kHz (100 us); `usb_poll` every 5th call = **2 kHz** | `main.c:21,34`, `usb.c:1230` |
| USB | full speed (a 1 kHz SOF is counted), bulk endpoints of 64 bytes ⚠ the host's bulk scheduling is not under our control | `usb.c:1235-1238`, MIDI endpoints `usb.c:281-283` |
| Audio overload bound | shed a voice when a half takes over 85 % of 2.90 ms (2.47 ms) twice running | `audio.c:51,58-60` |

### 1.2 How a note travels today (T-1 path; the new `sq_block` keeps the same shape, ADR-0003 7.2)

1. **Tick to block.** `events_block` runs at the start of every 32-sample block (`seq.c:1314`) and, in T-1 mode,
   calls `t1_block(seq_n)` (`seq.c:1427`). `t1_block` adds `n * 192` to `t1_acc` and runs every tick that has become
   due (`t1_glue.c:133-137`). The tick runs at the *start of the first block that contains it*: **0 to 0.73 ms late,
   0.73 ms peak-to-peak**. The remainder stays in `t1_acc`, so there is no cumulative error [V].
2. **Tick to ring.** `t1_pipeline_tick` schedules into a time-sorted queue and the dispatcher emits everything with
   `tick <= current_tick` (`torso_t1_pipeline.c:398-399`); it compares whole ticks only, the `sub_tick` field is
   never read [V] (ratchet sub-tick positions in `torso_t1_algo.c:204` are lost). A negative micro-delay is clamped
   to "now" (`torso_t1_pipeline.c:308`), so no event can ever be placed *earlier* than its tick [V]. Note-off is
   ordered before note-on at an equal tick (`torso_t1.c:274-277`) [V].
3. **Flush at the end of every block.** `t1_pipeline_flush_midi` converts the three-byte messages into USB-MIDI
   packets and calls `midi_out_event` for each (`t1_glue.c:138`, `torso_t1.c:469-492`). `midi_out_event` writes the
   64-entry ring `midi_out_q[MQ]` and drops when no host is configured or the ring is full (`usb.c:169,194-200`).
   **No packet carries a time.** USB-MIDI 1.0 has no timestamps; the packet means "now" when the host receives it.
4. **Four blocks, one burst.** The four `events_block` calls of a half run inside one audio interrupt
   (`audio.c:116-120`), so the ring fills in a burst whatever the block each note came from.
5. **The transmitter cannot run while the audio interrupt runs.** TIMER5 nests into the audio interrupt but, while
   `in_audio` is set, only services USB audio and returns (`main.c:42-48`); `usb_poll` and therefore `ep1_tx`
   (`usb.c:1308`) run only between audio interrupts, on the next 2 kHz poll (`main.c:50-52`) [V by reading].
6. **`ep1_tx`** copies up to 16 events (64 bytes) into the endpoint and arms it if the last packet has gone
   (`usb.c:916-960`). The host collects it at its next bulk poll of that endpoint.

### 1.3 The jitter budget today (worst case and spread; every hop)

| Hop | What it adds | Range | Peak-to-peak | Basis |
| --- | --- | --- | --- | --- |
| H1 tick placed at its block start | lateness inside the block | 0 .. 0.73 ms | 0.73 ms | 1.2 item 1 [V] |
| H2 block to wire: all events of a half leave after the interrupt ends | the position of the block inside the half: a note from block 0 waits three more blocks than one from block 3 | 0 .. 2.18 ms (3 blocks) | 2.18 ms | 1.2 items 4-5 [V by reading] |
| H1 + H2 combined | an event's lateness is "end of its half" minus "its own time", uniform over the half | 0 .. 2.90 ms | **2.90 ms** | the half buffer |
| H3 `usb_poll` period | waiting for the next 2 kHz poll after the interrupt ends | 0 .. 0.50 ms | 0.50 ms | `main.c:21,34` |
| **FM-1 side total** | | 0 .. 3.40 ms | **3.4 ms** | |
| H4 USB full-speed frame | the host takes the packet at its next bulk poll | 0 .. 1 ms | 1 ms ⚠ | `usb.c:1235` [I] |
| H5 host stack | CoreMIDI timestamping and the DAW's own latency | not ours | not measured ⚠ | |
| **Visible in the DAW** | | | **about 4.4 ms plus host noise** | |

If the audio interrupt is longer than a block of work, the spread H2 shrinks (the events have gone out later in
real time), but the batching, not the load, sets the spread: the events still leave together at the end. At 120 BPM
a 1/16 step is 125 ms, so 4.4 ms is 3.5 % of a step. That is the "loose" feel when notes are recorded against a grid.

Mean added latency (tick to wire) is about 1.45 ms (half a half buffer) + 0.25 ms (poll) + 0.5 ms (USB) = 2.2 ms,
and is not a problem: a constant delay is inaudible to a performer and compensable in a DAW.

### 1.4 External clock in today (INT is above)

| Step | What happens | Source |
| --- | --- | --- |
| Arrival | `midi_enqueue` stamps each packet with `fm1_ms` (1 ms resolution) **at the moment the 2 kHz poll reads it**, so the stamp is 0..0.5 ms late and quantised to 1 ms | `usb.c:176-190` |
| Realtime accepted | only `0xF8 0xFA 0xFB 0xFC` are forwarded as clock; **`0xF2` Song Position Pointer is dropped** [V] | `usb.c:821-824` |
| Which source | the ring carries `source` 1 = USB, 2 = TRS; only the one selected in `song.g[G_CLOCK]` is used | `seq.c:1394-1403` |
| Pulse | each `0xF8` is processed at the start of the next block | `seq.c:1391-1403`, `midi_clock.c:33` |
| Tempo | measured only every 6th pulse from a 1 ms-resolution `dt` (62..375 ms accepted), written straight to `song.g[G_BPM]` (the user's tempo is overwritten) and to `midi_beat_samples`; a 250 ms span at 120 BPM measured to 1 ms is **0.4 % per update** (±0.5 BPM) | `midi_clock.c:36-67` |
| Position | `pos += beat_samples()/24` per pulse, with the remainder carried: the position is **pulse-counted, so it cannot drift against the host** [V] | `midi_clock.c:68-76` |
| Between pulses | linear interpolation of elapsed `fm1_ms` against the last single interval (8..80 ms, no averaging) | `midi_clock.c:70-72,86-104` |
| Start | `0xFA` sets `pos = 0` and calls `seq_start()` at once; `t1_on_start` primes `t1_acc = beat_samples()` so **tick 0 runs in the first block, at the Start message and not at the first `0xF8`**; the first `0xF8` after Start only sets `have_pulse` | `midi_clock.c:13-18,68-77`, `t1_glue.c:117-118` ⚠ measure the offset |
| Loss | while playing, no pulse for 500 ms stops the transport; **while stopped nothing watches** | `seq.c:1406-1416` |
| Clock out | none: no `0xF8/0xFA/0xFB/0xFC` is ever sent [V: grep of `midi_out_event` callers] | `seq.c:736,753`, `t1_glue.c:88,94`, `ui_input.c:122` |

So the follower is already count-locked (good), and its weaknesses are 1 ms stamps, a six-pulse tempo update, no
filtering of the single interval used for interpolation, a late Start, no Song Position, and the overwritten tempo.

## 2. Target and budget

Measured at the host input (CoreMIDI) from the FM-1's USB port, over at least 5 minutes (at least 600 notes), on the
timing-test image (section 6), with the constant latency removed (a straight-line fit of arrival against the ideal
grid). "FM-1 side" figures come from the device's own counters (section 6.3): release time minus due time.

| # | Quantity | Target (pass) | Hard limit (fail) | Today | Why this number |
| --- | --- | --- | --- | --- | --- |
| T1 | Release lateness inside the FM-1 (due time to the endpoint being armed), peak-to-peak | **0.25 ms** | 0.40 ms | 3.4 ms | one 100 us release tick + one USB endpoint refill + interrupt jitter; 1 sample (22.7 us) for the stamp |
| T2 | Note-on and note-off jitter at the host, peak-to-peak | **1.5 ms** | 2.5 ms | about 4.4 ms | T1 + the 1 ms full-speed USB frame (uniform, so 0.29 ms RMS) + 0.2 ms host noise; the host stack is the floor and cannot be removed |
| T3 | Same, RMS | **0.4 ms** | 0.7 ms | about 1.0 ms | uniform 1 ms frame is 0.29 ms RMS; margin for T1 and the host |
| T4 | Constant latency, grid tick to host | at most **6 ms** and constant (T2 holds around it) | 10 ms | about 2.2 ms mean | the stamp leads by the 2.90 ms half buffer so MIDI and the preview synth line up (M2) |
| T5 | Internal tempo error against the 24 MHz crystal, 5 min | **100 ppm** (0.006 BPM at 60 BPM) | 200 ppm | **400 ppm** | the correction of M10; 100 ppm is 6 ms per minute, below T2 per beat and never audible; a free-running oscillator pair differs by about this much anyway |
| T6 | Clock out: pulses per quarter | exactly **24**, over 10 min no miscount (0 lost, 0 extra) | any miscount | not implemented | a follower counts pulses; one lost pulse is a permanent offset |
| T7 | Clock out: pulse spacing jitter at the host, peak-to-peak | **1.5 ms** (same as T2) | 2.5 ms | n/a | same path |
| T8 | Clock out: mean pulse spacing error | **100 ppm** | 200 ppm | n/a | same as T5 |
| T9 | Following: position drift against the host | **0 steps** over 10 min, any tempo | 1 step | 0 (count-locked [V]) | the invariant of 1.4 must survive the PLL |
| T10 | Following: note-on jitter measured from the host's clock pulse (loop-back), peak-to-peak | **3.0 ms** | 4.5 ms | not measured | two USB hops (host to FM-1 stamped at 0.5 ms resolution, FM-1 to host) + the PLL residual |
| T11 | Start: first note-on after a host `0xFA` | within **1.5 ms** of its steady-state latency measured from the first `0xF8` after Start | 1 pulse | up to one pulse early ⚠ | step 0 must be the first pulse (1.4, Start row) |
| T12 | Ordering | **0** note-on before its note-off for the same channel and note at an equal tick, **0** stuck notes after Stop, over 10 min | any | holds (`torso_t1.c:274`) | |
| T13 | Tempo follow lag | tempo of a host ramp of 1 BPM per beat tracked within **0.3 BPM**, measured on the period the loop advances its phase by (`peff` = P + e/4, the quantity that places ticks; M11.2). The loop's P alone lags this ramp by about 0.65 BPM and the shown integer BPM may lag up to 1.5 BPM; that display lag is **accepted** (it is a readout, not timing) | 1 BPM | quantised to 0.5 BPM steps every 6 pulses | PLL gains of M11 |

The numbers are chosen so that nothing in T1 is limited by the firmware: the floor is the USB frame and the host.
If the owner wants less than T2 the only lever is a faster bulk poll on the host side, which a USB device cannot
request; USB-MIDI 2.0 timestamps are out of scope.

## 3. Mechanisms

A new module-level object, the **wall-clock release ring** (`txr`), sits between `sq_emit` (ADR-0003 8.1) and the
endpoint. Everything below is firmware design for the dev tasks of section 9; none of it is implemented by this task.

### M1. Exact tick position inside the block

`sq_block` (ADR-0003 7.2) already accumulates `sq_acc += n * 192` against `beat_samples()`. Compute, for each tick that
fires in the block, the sample it belongs to: with `acc0` the accumulator before the add and `bs` the beat length,

```
s = ceil((bs - acc0) / 192)      /* 0 .. n-1, the sample inside this block where the tick lands */
```

and for a further tick in the same block (possible only above about 330 BPM) add `ceil(bs / 192)`. `s` is exact to one
sample (22.7 us). No change to the 192-PPQN grid, to host tests of tick order, or to the accumulator.

### M2. Wall-clock stamp ("due") on every outgoing packet

Every packet leaving `sq_emit` (notes, CC123, clock, Start/Stop/Continue/SPP) carries `due`, in TIMER4 ticks:

```
due = tref_block + s * 544 + LEAD         tref_block = tref + b * CTL * 544   (b = 0..3 the block within the half)
LEAD = HALF_FRAMES * 544 = 69632 ticks = 2.90 ms
```

- `tref` is the half buffer's reference time. `audio.c:103` already reads `t0 = fm1_ticks()` at the start of each
  half and uses it for the key-latency stats (`audio.c:118`, `kb_out_tick`). Instead of using raw `t0` (which jitters
  with interrupt entry), keep `tref += 69632` each half and nudge it by 1/16 of `(t0 - tref)`, resynchronising
  outright if the error exceeds 1 ms. Interrupt-entry jitter is then filtered out and a real rate difference between
  the I2S clock and TIMER4 is tracked.
- `LEAD` is the time the audio of this half is *played*: MIDI then lines up with the preview synth the user hears
  (the preview is the same events, ADR-0003 8.1), and it is long enough that every stamp is in the future when it is
  pushed even at the 85 % load bound (render at most 2.47 ms < 2.90 ms). It is a constant in the flex table
  (section 10); a smaller `LEAD` lowers T4 and is limited only by the render time of a half.
- External clock: the same `due` is built from the PLL phase instead of `tref` (M11): `due = pll_tick_time(tick) + LEAD`.

### M3. Timed release: TIMER5 sends at the due time, not the audio interrupt

`txr[128]` of `{uint32 pkt; uint32 due;}` (1024 B) replaces the 64-entry `midi_out_q` for packets the sequencer sends
(ADR-0003 8.3 had a 128-entry out ring of 4-byte packets; this is +512 B).

- Producer: `midi_out_event_at(pkt, due)` (the audio interrupt). It inserts at the tail; because stamps are produced in
  non-decreasing order the ring stays sorted, and equal stamps keep insertion order (M4).
  **Accepted limit (P2-032):** "due" is a signed 32-bit difference, so a head packet older than 2^31 ticks (89 s at
  24 ticks/us) reads as "not due" until the counter wraps. A host that stalls for more than 89 s with a non-empty ring
  wedges the head for up to another 89 s. Accepted: the ring is cleared on a bus reset (`usb.c`, `mo_r = mo_w`), but not on suspend, so the
  case is a host that stays configured but stops polling for over 89 s. A fix, if wanted, is to clear the ring when
  `usb.config` drops outside a bus reset (firmware task, not done here).
- Consumer: `midi_tx_release()`, called by TIMER5 **every tick (10 kHz), including when it nests inside the audio
  interrupt** (extending the nested branch of `main.c:42-48`). It looks only at the head: if
  `(int32_t)(fm1_ticks() - head.due) >= 0` and the endpoint is free it moves every consecutive due packet (up to 16)
  into `ep1tx` and arms it (the body of `ep1_tx`, `usb.c:932-960`). Idle cost is one compare.
- Why nested: the audio interrupt occupies a share of every half equal to the CPU load, and stamps fall inside it with
  that probability; if the release waited for the interrupt to end, those packets would be late by up to the render
  time (up to 2.47 ms at the 85 % bound). ⚠ Risk R1: the SIE index register `S_INDEX` is shared; `usb.c:5` states
  that all SIE access lives in the TIMER5 handler "so INDEX is never shared". The release function runs in that same
  handler (nested under audio, not under itself), so the rule holds, but the dev task must prove it with a stress
  test at 85 % load (section 6.4) and must keep the fallback: if nested access proves unsafe, release only between
  audio interrupts and the T1 target becomes the render time (0.25 ms target fails; T2 stays under its hard limit
  at loads below 50 %).
- Granularity: 100 us. A due time is met within 0.1 ms plus the interrupt latency.

### M4. Ordering (what the wire sees at one tick)

Insertion order at a tick: **clock pulse (`0xF8`) first**, then Start/Continue/Stop, then all NOTE_OFFs, then all
NOTE_ONs, then CCs (ADR-0003 7.3 and 7.4). **Exception at tick 0 of a start (M7):** `0xFA` first, then the first
`0xF8`, because MIDI requires Start to precede the first clock; the steady-state order above applies from the next
tick. Equal stamps keep that order through `txr` and through the 16-event packet.
Same channel and note: the off precedes the on at the same tick (already true in the T-1 queue, `torso_t1.c:274-277`);
the new core must keep it, and its host test asserts it for every tick of a 4-bar run (T12).

### M5. Ring sizing and priority

- Capacity: at most 24 note events per tick plus one clock event (ADR-0003 8.2); the stamp leads by 2.90 ms, which
  holds at most 3 ticks at 240 BPM (tick 1.30 ms): 75 events worst case, 50 at 120 BPM. **128 entries** (the same
  number as ADR-0003 8.3) hold it with margin; the drain is 16 events per 1 ms frame (16000 per second) against a
  worst clock rate of 96 per second.
- Priority when short on room (ADR-0003 8.3 reserve rule, kept): with fewer than 8 free, new NOTE_ONs are dropped and
  counted; NOTE_OFFs and clock are kept while any slot is free. At a completely full ring (`midi_out_free() == 0`,
  `usb.c midi_out_event_at`) everything is dropped, OFFs and clock included; the reserve makes that unreachable in
  normal use (a host that has stopped reading is the only way), and a bus reset clears the ring. Clock is `CIN 0xF`, one byte, so a pulse costs one slot.
- TIMER5 stays above the audio interrupt in priority (it already nests into it, `main.c:42`); no priority change.

### M6. Flush frequency

The per-block flush (`t1_glue.c:138`) is removed: the per-tick `sq_emit` calls `midi_out_event_at` directly. Nothing is
batched except what the 16-per-packet endpoint transfer batches by itself, and only for packets that are *already due*.

### M7. Clock out at exact sample positions

- A pulse is due at ticks 0, 8, 16, ... of the sequencer grid. It goes through M1/M2/M3 like a note, so its spacing
  has the same accuracy (T7, T8) and its count is exact (T6): one `0xF8` per 8 ticks, produced by the tick loop itself.
- `0xFA` is due at tick 0 of a start from step 0 and is followed by the first `0xF8` at the same stamp (so at tick 0 the
  order is `0xFA`, then `0xF8`: the M4 exception; steady state is `0xF8` first). `0xFC` on stop
  is due at once, before the note-offs of M4's `sq_all_off`. **`0xF2` Song Position Pointer (new)** is sent before
  `0xFB` on a continue, value = `tick / 48` (16th notes; one SPP unit is 6 pulses = 48 ticks), so a DAW follower
  locates. The pulse and the transport bytes are `CIN 0xF` (one byte) except SPP (`CIN 3`, three bytes).
- Sent only while the FM-1 is the tempo master **and playing** (section 4); a stopped FM-1 sends nothing, so a host
  that loops our clock back can never feed it to us while we are idle, and no clock reaches the follower unless there
  is a transport to follow.

### M8. Start and Stop alignment

- Internal PLAY: `transport_req` is read at the start of a block (`seq.c:1326-1334`), `sq_acc` is primed so tick 0
  runs in the first block, `s = 0` (`t1_glue.c:117-118`); the first note is due at `tref + LEAD`.
- External Start: see M12. External Stop (`0xFC`): `seq_stop()` as today (`midi_clock.c:28-30`); `sq_all_off()` queues
  the note-offs *due now* (not stamped with LEAD), so a held note is released at once.

### M9. Swing, delay and other offsets are intentional only

Every musical offset is an integer number of ticks **computed from the grid, never from the current time**: swing
(ADR-0003 5: `o = (L*(swing-50)*2+50)/100` ticks, odd steps), the slide overlap `D`, GATE, ratchet spacing (at least
3 ticks, ADR-0003 7.3). The scheduler stamps `due(tick)` for the offset tick with M1/M2, so an offset adds exactly
what it says and nothing that depends on the block, the interrupt or the USB state. Two limits to state honestly:

- The grid is 192 PPQN: an offset is quantised to a tick (2.60 ms at 120 BPM, so a swing step can be up to
  ±1.3 ms from the ideal percentage). The error is the **same every bar**, so it is feel, not jitter. A finer grid was
  rejected in ADR-0003 7.1 and stays rejected.
- No event is placed *earlier* than its tick: the T-1 clamped negative delay to now (`torso_t1_pipeline.c:308`), and
  v1 has no negative nudge. If one is added later it needs a scheduler look-ahead of `NUDGE_MAX` ticks.

### M10. The tempo constant

`beat_samples()` (`fx.c:219-222`) must count in the real sample rate: `beat = 60 * 44117.647 / BPM` samples. To keep
240 BPM within 5 ppm, return the length in **1/16 samples** (`2647058.8 * 16 / BPM`, rounded) and accumulate the tick
counter in the same unit; every other caller (`div_samples`, `click`, the arp) divides by 16 or is updated by the dev
task. The constant is `SEQ_FS_Q4`, a flex row. ⚠ The test build measures the real rate (counter `halves` against
`fm1_ticks()` over 60 s, section 6.3); the constant is replaced by the measured value if it differs from 44117.65 by
more than 50 ppm.

### M11. Following an external clock without jitter or drift

1. **Stamp in ticks, at the receiving poll.** `midi_in_ms[]` becomes `midi_in_t[]` holding `fm1_ticks()` (`usb.c:171,187`).
   The 0.5 ms poll granularity stays (polling EP1 at 10 kHz would cost three SIE round trips per call, `usb.c:~1304`);
   it is a uniform ±0.25 ms error that the loop below averages.
2. **A phase-locked loop on the pulse train.** State: `phase` (the expected time of the next pulse, ticks) and `P`
   (period, 1/256 tick). On each accepted pulse at stamp `a`: `e = a - phase`; `phase += P + e/4`; `P += e/64`. `P` is
   clamped to the 40..240 BPM range (62.5 .. 10.4 ms per pulse). The loop has a time constant of about 4 pulses for
   phase and 64 pulses for frequency, so a 0.5 ms wobble becomes about 0.1 ms and a host tempo step is followed within
   three beats. T13 (a 1 BPM per beat ramp within 0.3 BPM) is asserted on `peff = P + e/4`, the period the loop advances `phase` by, because that is what places ticks (`clkx_tick_time`, `clkx_beat_q4`). With gains 1/4 and 1/64 the loop's `P` alone lags a 1 BPM/beat ramp by about 0.65 BPM (16 pulses of ramp), which is not a timing error; `peff` has no lag (0.002 BPM). The displayed integer BPM (1/16 smoothing plus hysteresis) may lag up to 1.5 BPM on that ramp; this display lag is accepted. **Output jitter** ("under 0.15 ms", section 6.3) is RMS with +-0.5 ms uniform input noise (measured 111-121 us RMS, 0.65-0.72 ms peak-to-peak); the peak-to-peak figure sits inside the T10 budget (3.0 ms), so T10 is the peak-to-peak bound and 0.15 ms is the RMS bound.
3. **Bursts and gaps.** A pulse arriving less than `P/4` after the previous one is a USB burst (several events in one
   packet share a stamp): it is **counted for position** but excluded from the loop. A gap over `1.5 P` is counted as
   the missing pulses only if the next stamp shows it; a gap over 500 ms is a lost clock (section 4). "Counted" here means **loop phase only**, not position: the position stays count-locked to the pulses actually accepted (4.3). So a host that drops a pulse shifts the music by that pulse until its next Start. This is accepted: USB bulk is lossless, and a stalled host is not a pulse the host counted.
4. **Position is a pulse count, time is the loop.** `pos` still advances by one pulse's worth per accepted `0xF8`
   (`midi_clock.c:73-75`), so the invariant of 1.4 holds: no tempo error can accumulate (T9). The loop only places the
   ticks *between* pulses: `due(tick 8k + j) = phase_k + j * P/8 + LEAD`, replacing the interpolation of
   `midi_clock.c:86-104`. The tempo shown to the user is `P` smoothed (1/16 per pulse), integer BPM with ±1
   hysteresis.
5. **No 6-pulse quantisation.** The BPM estimate updates every pulse; `midi_clock.c:36-67` goes. The first estimate is
   still taken from the first 7 pulses (`EXT --` for that long, SCREEN_SPEC 6.2).
6. **Tempo changes mid-play** rescale each channel's phase by the new/old ratio, exactly as today
   (`midi_clock.c:47-63`).

### M12. Start, Continue and Song Position when following

- `0xFA`: latch "start pending"; **step 0 begins at the first `0xF8` after it** (the MIDI rule), with `pos = 0` and
  `s` from that pulse's stamp. Today tick 0 runs at the Start message (1.4); this fixes T11. The transport shows
  playing from the Start message so the header does not lag.
- `0xFB`: continue from the held position: `pos` as it was, begins at the next `0xF8`.
- `0xF2` (new, stopped only): `tick = spp * 48`; on the next `0xFB` or `0xFA` the channels' `t` and `grid` are
  derived from it (`t = tick / L`, so a mid-step position waits for the next step boundary); a `0xFA` resets it to 0.
- `0xFC`: stop and `sq_all_off()` (M8).

## 4. Clock source model (owner rulings 2026-11-10, AUTO)

### 4.1 State

| Field | Meaning | Where |
| --- | --- | --- |
| `bpm_own` | the user's tempo, `song.g[G_BPM]`, **never written by the clock** (replaces `midi_clock.c:65`) | existing |
| `clk_ext_p` | the PLL period (M11), 0 when no clock is held | new, ISR |
| `clk_src` | `INT` or `EXT`, with the physical port (USB or TRS) that won | new, ISR |
| `clk_last` | `fm1_ticks()` of the last accepted pulse | new, ISR (replaces `last_ms`, `midi_clock.c:83`) |
| `clk_known` | at least 7 pulses measured | new |
| `clk_armed` | PLAY pressed while EXT and the host has not started (WAIT) | new, one flag |
| `clk_pulse_n` | 8-bit count of accepted pulses, free-running while a clock is heard; the dot's host beat | new |

Together about 24 B. The menu row `CLOCK` keeps its place; its values become the question in section 10 (Q1).

### 4.2 Rules

| # | Event | Result |
| --- | --- | --- |
| R1 | Default (power-on) | `INT`. The tempo is `bpm_own`. FM-1 is the master. |
| R2 | A `0xF8` is accepted from USB or TRS while **stopped** | source becomes `EXT` on that port (the first port heard wins; the other is ignored until this one has been silent 500 ms). Header `EXT --`. |
| R3 | A `0xF8` arrives while **playing as INT master** | ignored (it may be our own clock looped back through a patch bay or a hub; the FM-1 has been the master and stays it). |
| R4 | A host `0xFA` arrives while playing as INT | the host takes over: stop, `sq_all_off()`, source `EXT`, restart at step 0 at the next pulse (M12). A bare `0xFB` is treated the same way. |
| R5 | EXT, no pulse for **500 ms** (`CLK_TIMEOUT_MS`), stopped | source returns to `INT` at `bpm_own`; nothing else changes. The 500 ms is the existing value (`seq.c:1410`): 24 pulses at 120 BPM, 8 at 40 BPM. |
| R6 | EXT, no pulse for 500 ms, **playing** | the transport stops as today (`seq.c:1410-1411`), `sq_all_off()` runs, then as R5. Strip `CLOCK LOST: STOPPED` for 3 s. The sequencer does **not** continue on the internal clock: a mid-set tempo jump from the host's to the user's is worse than silence. |
| R7 | EXT, host `0xFC` | stop as today. The source stays `EXT` while pulses continue (many hosts keep sending). |
| R8 | EXT, user presses PLAY | `clk_armed` = 1: the header shows `WAIT`, the dot blinks (section 5). Starts on the next host `0xFA` or `0xFB` (M12). **No silent fall back to internal.** PLAY again (or STOP) cancels. |
| R9 | `clk_armed` and the clock is lost (R5) | armed clears, source `INT`, strip `CLOCK LOST: PLAY TO START` for 3 s; nothing starts by itself. |
| R10 | EXT, user presses STOP while the host runs | the FM-1 stops locally; the source stays `EXT`; the next PLAY is R8. |
| R11 | EXT, SET K1 | inert (locked, `knob_map.tsv`); the cell mirrors the host tempo; `bpm_own` is untouched and returns on R5. |
| R12 | Clock out | on when the source is `INT` and the FM-1 is playing (M7); off in every EXT state and while stopped. No menu row. |
| R13 | Host tempo outside 40..240 BPM | not accepted as a clock (treated as no clock). The tempo is first known after 7 pulses, so the header shows `EXT --` for those 7 pulses (about 56 ms at 300 BPM) and then returns to `INT <bpm_own>`; a source that was already `EXT` stops advancing `clk_last`, so R5 / R6 apply after 500 ms. It never stays in `EXT --` (P2-011, F-18). |
| R14 | A tempo jump over 25 % between pulses | PLL re-seeds `P` after two consecutive intervals that agree with each other (within 25 %) and both disagree with `P`; a lone off interval is an outlier (counted, phase kept), and exactly 25 % is tracked by the loop, not re-seeded. A jump faster than 4x (e.g. 40 to 240 BPM) makes the new intervals shorter than `P/4`, which are treated as bursts (M11.3) and is not followed; position is never touched (count-locked), so the sequence continues at the new tempo without a skip. |
| R15 | TRS and USB both send clock | the first heard wins (R2); the other is ignored until silent. |
| R16 | A host `0xFA` (or `0xFB`) arrives while **stopped** on `INT`, before any `0xF8` (P2-011, F-07) | the FM-1 latches a pending start (M12) and the source stays `INT`; the first `0xF8` that follows makes the source `EXT` (R2) and step 0 begins on that pulse. Nothing changes on screen until the pulse. If no `0xF8` arrives within 500 ms the pending start is dropped. |

### 4.3 Why the position stays locked

The chain in the follower is: pulse count (exact) -> `pos` -> tick = `8 * pulses + j` -> `due`. Nothing in it integrates
a measured tempo, so a wrong tempo estimate moves a tick by at most a fraction of a pulse and resets at the next pulse.
Start, Continue and SPP set the pulse count; a pulse that is not accepted (R3, ignored source) is not counted. A pulse the host never sent (or that was dropped upstream of the FM-1) is likewise not counted: M11.3 credits a gap to the loop phase only, so the music stays shifted by that pulse until the next Start (accepted).

## 5. Visibility: the clock and its source on every screen

Supersedes SCREEN_SPEC 6 (rewritten in that file to match). Additive to the locked UX: it fills the header slot
H2 and the SET status lines; no control changes.

### 5.1 The dot

| Item | Specification |
| --- | --- |
| Size and place | **8 x 8 px** disc at x = 12, y = 6 inside header slot H2 (x 12..68, y 4..16); the text starts at x = 23 (8 px dot + 3 px gap). `EXT 126` is 45 px: 23 + 45 = 68, exactly the slot. |
| Redraw | only the 8 x 8 cell when its phase changes; the text redraws on a tempo or state change (8 redraws per second at most at 240 BPM). The main loop is 60 fps, so a flash edge is within ±8 ms of the beat, which is a display, not a timing, figure. |
| Beat flash | **on** for one sixth of a beat, clamped to 50..120 ms; the first beat of the bar twice that. **Off-phase** is the same shape blended 65 % toward `T_BG` (so the dot is always findable). |
| INT | colour **amber**, **solid disc**. Flashes on the internal beat (`beat_pos` against `beat_samples()`, `seq.c:350,1437-1441`), **also while stopped**, so the user sees the tempo before PLAY. |
| EXT (live or syncing) | colour **cyan**, **disc with a 2 x 2 hole in the centre**. Flashes on the host beat: `clk_pulse_n % 24 < 4` is on (4 pulses = 1/6 beat); the bar's first beat is marked only after a host Start. While syncing (`EXT --`) the dot is steady at the off-phase level. |
| WAIT | cyan, **ring** (disc with a 4 x 4 hole), blinking **2 Hz** (250 ms on, 250 ms off), independent of tempo: it means "armed, waiting", not "beat". |
| Colours (RGB565) | dark theme background: amber `0xFD80` (255,176,0), cyan `0x071F` (0,224,255). Light background (luminance of `T_BG` over 50 %, for example PAPER): amber `0xCB80` (200,112,0), cyan `0x03D4` (0,120,160). Fixed values, not theme roles, so the meaning survives theme changes. |
| Not by colour alone | MONO and GREY themes and colour-blind users: INT is solid, EXT has the hole, WAIT is hollow; amber and cyan lie on the blue-yellow axis, safe for red-green colour blindness. The word `INT` / `EXT` / `WAIT` is always printed beside the dot. |

### 5.2 Header text (slot H2, measured with the real `text_w`, AF_S)

| State | Dot | Text | Width | Condition |
| --- | --- | --- | --- | --- |
| INT | amber solid | `INT 120` | 42 | no clock accepted, or R5 |
| EXT syncing | cyan hole, steady dim | `EXT --` | 34 | clock heard, fewer than 7 pulses |
| EXT live | cyan hole, beat | `EXT 126` (T_ACCENT) | 45 | `clk_known` |
| WAIT | cyan ring, 2 Hz | `WAIT` (T_ACCENT) | 28 | `clk_armed` |

`EXT?` and the dimmed last-host-tempo state of the old SCREEN_SPEC 6.2 are gone: when the clock stops the source
simply returns to `INT` (R5) and the user's own tempo is shown.

### 5.3 Placement on every screen

| Screen | Dot |
| --- | --- |
| PERF, NOTE_EDIT, CHAN, SET, SCALE, REC, RESET and their overlays (FIRST_RUN, ADD_PAGE, MOVE_HELD) | header H2 as above |
| HELP | the header copy is dimmed, the dot is **not** dimmed (it is live state) |
| MENU and its INFO / ABOUT pages | no header slot: the dot is drawn immediately after the title text, `x = 30 + title width + 8`, `y = H_HEAD/2 - 4` (`ui_menu.c:156-159`); no tempo text. The dev task checks it clears the REC mark and the CLOSE hint (`ui_menu.c:164-170`), which sit at the right. |
| Count-in, confirm dialogs, NAME and any other full-screen mode that has no Z0 | the corner cell **x 4, y 4, 8 x 8 is reserved** for the dot (those screens draw nothing in it) |
| UBOOT (update mode) | none |

### 5.4 LEDs

| Situation | PLAY green LED (`LED_PLAY_GREEN`, `ui_input.c:30,457`) | PLAY's own LED |
| --- | --- | --- |
| Stopped, INT | dark | idle glow as today (`ui_input.c:438`) |
| Playing (INT or EXT) | **mid level steady, full level for the beat flash** (same on-phase as the dot, from `beat_pos`) so "playing" and "tempo" are one LED; today it is steady (`ui_input.c:456-457`) | dark as today |
| WAIT (armed) | **full / dark at 2 Hz**, the same blink as the dot | dark |
| Stopped, EXT clock heard | dark | **flashes on the host beat** (`clk_pulse_n`), so an external clock is visible without looking at the screen |

⚠ `fm1_led_mid` is per column of the key matrix (`ui_input.c:459-462`); that the PLAY green LED has a mid level is
inferred from it and checked on the device in the first hardware gate.

### 5.5 Strings added or changed (SCREEN_SPEC 6.4-6.5 carry them in full)

Cue strip: `WAIT FOR HOST START  PLAY=cancel` (32 characters, 198 px) while armed; `CLOCK LOST: STOPPED` and
`CLOCK LOST: PLAY TO START` (126 / about 160 px) for 3 s; `EXT: K1 OFF, HOST SETS TEMPO` stays on SET.

## 6. Measurement, test mode and tests

### 6.1 Test mode: the timing-test image

- **How it is entered:** a build flag, `TIMING_TEST=1 ./build.sh`, giving `build/felucca_timing_test.fwsc`. It is a
  separate image (not a hidden menu entry), so the normal build contains none of it: **0 B of RAM and 0 B of code in
  the normal build** (the code is under `#if FELUCCA_TIMING_TEST`, like `FELUCCA_CDC`). The normal image is therefore
  never different from what ships, and the test image exercises the *production* timing path: only the pattern and the
  counters are test code.
- **What it plays** (no algorithm, no random, no swing, no morph): channel 1, MIDI channel 1, one note `60` (C4),
  straight **1/16 notes** (4 per beat), gate 50 %, **velocity 90 (non-accent, decision 104), and velocity 127 on step 1 of each bar** (the bar
  marker the tool aligns on; nothing else varies). BPM **120** by default; SET K1 steps it through 60, 90, 120, 180,
  240 (the tool's sweep). Pattern length 16, one bar. It starts on PLAY, nothing else is needed.
- **Send or follow:** the same image does both by the AUTO rule (section 4): with no clock input it is INT master and
  sends clock out; with a host clock it follows (R2) and the dot says so. So one image covers "FM-1 to Ableton with
  sync off" (it sends), "Ableton to FM-1" (it follows) and "FM-1 to TB-3" (it sends, the TB-3 is the sink).
- **Counters** printed on the CDC console (`con_kv`, `console.c:55`, the interface that already exists) in the test
  image only: `tm_rel_hist[16]` (histogram of release lateness, 100 us bins: T1), `tm_rel_max`, `txr_hwm` (ring high
  water), `txr_drops`, `audio_halves` and `fm1_ticks()` at two instants (the real sample rate, M10), `sq_drops`, the
  existing `felucca_dbg.max_us`/`late` (`audio.c:21-22,125,133`). About 100 B, test image only.
- **Dev task:** T-A (section 9).

### 6.2 The measurement tool

A new `tools/fm1_timing.py` (the existing `tools/fm1_midi_listen.py` skips clock and uses 2 ms polling with Python
timestamps, `fm1_midi_listen.py:63-76`, which is itself 1 to 2 ms of noise, so it is not extended). Requirements:

- Open the FM-1 port with `python-rtmidi` using a **callback**, taking the CoreMIDI event time (the delta the driver
  supplies) rather than `time.monotonic()` in a polling loop; log every message including `0xF8 0xFA 0xFB 0xFC 0xF2`,
  as `time_ns, status, d1, d2`.
- **Calibrate the floor first:** `--loopback` sends a known pulse train from the Mac to an IAC bus and back through the
  same callback path and reports its own RMS jitter, so the host noise is measured and subtracted from any claim.
- **Report** (one line each, plus a verdict against the thresholds of section 2): note-on and note-off jitter against
  the straight-line fit of the ideal grid (peak-to-peak, RMS, worst five), constant latency, mean tempo error in ppm,
  clock-out pulses per quarter (the count must be exactly 24 over every quarter), spacing jitter and mean error,
  note-on before note-off count, stuck-note count after Stop, drift after N minutes, and for the follow test the
  jitter measured from the host's pulse to the FM-1's note (round trip, T10).
- **Sweep:** 60, 90, 120, 180, 240 BPM, five minutes each (the 20-minute suite is not run by this task; the sweep is the
  hardware gate).
- Exit code 0 only if every threshold passes; the log carries the numbers.
- **Dev task:** T-B (section 9).

### 6.3 Host tests (where timing logic can be tested without the device)

All use fake time (`fm1_ticks()` and the `tref` driven by the test), in the style of `tests/torso_phase*_test.c`.

| Test | Asserts |
| --- | --- |
| tick position | M1: for BPM 40..240 and every accumulator phase, `s` is within 1 sample of the exact tick time; ticks per block never exceed the bound |
| stamp | M2: `due` is non-decreasing over 10 minutes at every BPM; equal for events of one tick |
| ordering | M4: for 4 bars of dense ratchets, at every tick `0xF8`, then offs, then ons; no same-note on before off. **Exception: at tick 0 of a start the order is `0xFA`, then `0xF8`** (M7) |
| clock out count | M7: exactly 24 `0xF8` per quarter over 4 bars at 90, 120, 180 BPM; zero in every EXT state; `0xFA`/`0xFB`/`0xFC`/SPP only on the right transitions |
| release | M3: with a fake `fm1_ticks`, a packet is released within one 100 us tick of its due; 16 per transfer; ring at 128 never reorders; the reserve rule drops ONs before OFFs |
| tempo constant | M10: 5 minutes of ticks at 40..240 BPM against 44117.65 Hz samples stay within 5 ppm of the ideal |
| PLL | M11: a pulse train with ±0.5 ms uniform stamp noise has output jitter under 0.15 ms; a burst (4 pulses sharing a stamp) leaves the count right and the phase within 0.2 ms; a 1 BPM per beat ramp is tracked within 0.3 BPM (on `peff`, M11.2); output jitter under 0.15 ms is RMS (peak-to-peak is held to T10); no pulse is lost or added over 10 minutes (T9) |
| source rules | section 4: a table-driven test of R1..R16 with events in, state out |
| start | M12: a Start followed by pulses begins step 0 at the first pulse, not at the Start; SPP then Continue resumes the right step |
| header strings | the strings of 5.2 and SCREEN_SPEC 6 against the slot widths with the real `text_w` (the measurement SCREEN_SPEC section 5 requires) |

### 6.4 Hardware-test gate (human; run on the timing-test image; owner signs in the P2-010 or E4 log)

Equipment: FM-1 on USB to the Mac, Ableton Live 12, `tools/fm1_timing.py`, then the TB-3 through the USB-MIDI host box
(CLAUDE.md). Sync settings recorded in the log.

| # | Test | Pass |
| --- | --- | --- |
| G1 | **FM-1 sends, no DAW sync.** 5 minutes at 60, 90, 120, 180, 240 BPM, tool only | T2, T3, T5 (tempo against the Mac's clock), T6, T7, T8, T12 all pass; device counter `tm_rel` p-p at most 0.25 ms (T1) |
| G2 | **FM-1 sends, Ableton records.** Ableton records the MIDI track and follows the FM-1's clock (Sync input on) 5 minutes at 120 | recorded notes sit on the grid with a max deviation of 1.5 ms after the constant latency is removed; Ableton's tempo shows 120.0 +-0.05 for the whole take; no note drifts relative to the clock |
| G3 | **FM-1 follows Ableton.** Ableton plays and sends clock; the FM-1 plays the pattern; Ableton records it. Tempos 60, 120, 180, tempo ramp 100 to 140 BPM | T9 (0 steps of drift in 10 minutes), T10, T11, T13; header reads `EXT <bpm>`, dot cyan with hole flashing on Ableton's beat |
| G4 | **Hand-over.** Cold start with Ableton stopped; start Ableton; stop it; pull the cable while playing | R2, R4, R5, R6, R7, R8, R9 each observed with the screen text and dot; no stuck note on the TB-3 or in Ableton |
| G5 | **FM-1 to TB-3.** The pattern on MIDI channel 2 for 10 minutes at 120 and 180 | no stuck notes, no missed notes (the TB-3 plays what the tool logs), the groove tight to the ear against a metronome on the Mac |
| G6 | **CPU and safety.** Run G1 with the audio engine at the heaviest preview preset | `felucca_dbg.max_us` rises by under 5 % of a half over the old image (CPU target in section 9); `txr_drops` and `sq_drops` zero; the nested release (R1) causes no USB reset or lost packet |
| G7 | **LEDs and dot.** Look at the device while G1 and G4 run | section 5 behaviours, including the PLAY green mid level (5.4) |

## 7. Failure modes and the answer to each

| Failure | Answer |
| --- | --- |
| Host's USB bulk poll is slower than 1 ms | T2 is a measured threshold; if G1 fails only on T2 and T1 passes, the limit is the host, reported to the owner and not tuned away |
| Nested release (M3) proves unsafe | fallback in M3 (release between audio interrupts); T1 becomes the render time; reassess `LEAD` |
| Real sample rate is not 44117.65 Hz | M10 takes the measured value (a one-line flex row) |
| Ring overflows (a host that stops reading) | ADR-0003 8.3 reserve rule: ONs dropped and counted first, OFFs and clock kept while a slot is free; at a completely full ring all are dropped (M5), so stuck notes are prevented by the reserve, not by an absolute guarantee |
| Host stalls over 89 s with packets queued | M3 accepted limit: the signed-difference compare wedges the head for up to 89 s more; the ring is cleared on bus reset only |
| Host sends clock at 300 BPM | R13: not accepted as a clock; header shows `EXT --` for 7 pulses, then `INT` |
| A host loops our clock back | R3 and R12 (we send only while playing as master and ignore pulses then) |

## 8. Corrections to earlier documents (found while measuring)

1. **ADR-0003 7.1** says "Timing jitter is therefore at most one block (0.73 ms), below USB-MIDI's own 1 ms frame".
   That is true of the *scheduling* only. On the wire the jitter is up to 3.4 ms because of the half-buffer burst and the
   deferred transmitter (1.2, 1.3). The addendum to ADR-0003 replaces the claim with the numbers of section 2.
2. **ADR-0003 7.4** (clock out): "following ... sends nothing" and "pulses in the 64-entry ring" stand, but the pulse
   is now stamped (M7), SPP is added, and the source model is AUTO (4). Rewritten in the addendum.
3. **SCREEN_SPEC 6**: the four states INT / EXT SYNC / EXT LIVE / EXT WAIT with the "last host tempo" and the "MENU >
   CLOCK source" are replaced (the owner's AUTO ruling); the out-mark glyph is replaced by the dot.
4. **Tempo constant:** nothing earlier mentions the 400 ppm error (M10).

## 9. Cost and dev tasks

| Item | RAM | CPU / code |
| --- | --- | --- |
| `txr[128]` {pkt, due} | 1024 B, replaces ADR-0003 8.3's 512 B out ring: **+512 B** | one head compare per 100 us TIMER5 tick when idle; the transfer cost it already paid in `ep1_tx` |
| `tref`, nudge state, `SEQ_FS_Q4` accumulator unit | 16 B | one add and one shift per half |
| PLL, source state, flags, `clk_pulse_n` (4.1) | about 40 B (replaces `midi_clock` fields: net about +16 B) | one multiply-free update per pulse (at most 96 per second) |
| `midi_in_t[]` instead of `midi_in_ms[]` | 0 | none |
| Test mode counters | 0 normal, about 100 B test image | none |
| **Total vs ADR-0003 11** | **about +0.6 KB `.bss`**; the 39534 B headroom after removals (ADR-0003 11) is untouched in effect, the 4 KB floor and 8 KB trigger are not approached | CPU gate: `felucca_dbg.cpu_q8` / `max_us` (`audio.c:128-138`) must not rise by more than 1 percentage point on the same preset (G6) |

Dev tasks (E4 / E5; P2-005 turns these into ready tasks with this document in Read first):

| Task | Does | Depends on | Cites |
| --- | --- | --- | --- |
| **T-A** timing-test image and counters | `FELUCCA_TIMING_TEST` flag, the fixed pattern, counters on the console, `build.sh` target | ADR-0003 task 6 (`sq_core.c`) | 6.1 |
| **T-B** `tools/fm1_timing.py` | the tool of 6.2 with `--loopback` and the verdicts | none (can be built now, against `tools/fm1_midi_listen.py` ports) | 6.2 |
| **T-C** release ring and exact stamps | M1-M7, M9, M10; `usb.c` `midi_out_event_at`, `midi_tx_release`, nested TIMER5 branch; host tests of 6.3 (tick position, stamp, ordering, clock out count, release, tempo constant) | ADR-0003 tasks 5, 6 | 3, 6.3 |
| **T-D** clock source model and PLL | M11, M12, section 4, `midi_in_t`, SPP parse (`usb.c:821`), the host tests (PLL, source rules, start) | T-C | 4, 3 |
| **T-E** visibility | the dot, the header text, LED behaviour, strings, the width test | SCREEN_SPEC rewrite, E4 UI tasks | 5, SCREEN_SPEC 6 |
| **T-F** hardware gate | G1-G7 on the timing-test image, results and sign-off | T-A..T-E | 6.4 |

## 10. Flex points (table or constant edits, no redesign)

| Item | Where | Default |
| --- | --- | --- |
| `LEAD` (wall-clock lead) | M2 | 128 samples (2.90 ms) |
| PLL gains | M11 | phase 1/4, frequency 1/64, display smoothing 1/16 |
| `CLK_TIMEOUT_MS` | 4.2 R5 | 500 |
| Dot size, colours, flash length, WAIT blink rate | 5.1 | 8 px, amber/cyan, 1/6 beat clamped 50..120 ms, 2 Hz |
| `SEQ_FS_Q4` | M10 | 44117.65 Hz in 1/16 samples |
| `txr` size | M5 | 128 |
| Test BPM list and bar marker velocity | 6.1 | 60, 90, 120, 180, 240; 127 |

## 11. Questions for the owner

1. **ANSWERED 2026-11-10 (OWNER_QUESTIONS 76): (b) AUTO / INTERNAL ONLY.** **The CLOCK menu row after AUTO.** AUTO listens to USB and TRS and needs no setting. Options: (a) delete the row
   (no clock setting at all; a user whose host loops clock back has no escape except unplugging); (b) keep a single
   two-value row `CLOCK: AUTO / INTERNAL ONLY` for the rare looping or unwanted-follower case, default AUTO;
   (c) keep today's `INT / USB / TRS`. **Recommended: (b).** It does not add a row (the existing `CLOCK` row loses two
   values), so the tab row counts of `tests/ui_test.c` do not change.
2. **ANSWERED (76): (a) stop.** **Clock lost while playing (R6).** Options: (a) stop, as today and as specified; (b) keep playing on the internal clock
   at the user's tempo. **Recommended: (a):** a silent tempo jump mid-set is the failure the owner already rejected in
   SCREEN_SPEC question 3, and a stop is recoverable with one PLAY.
3. **ANSWERED (76): (a) PLAY again cancels, nothing else.** **Escape from WAIT.** PLAY pressed while waiting for the host: (a) PLAY again cancels (specified); (b) in addition,
   hold PLAY to start on the internal clock immediately (a deliberate override, not a silent fall back). **Recommended:
   (a) only** for v1; (b) adds a gesture and a state to explain.
4. **ANSWERED (76): as specified.** **Take over from a playing internal clock (R3, R4).** A host `0xF8` while the FM-1 plays as master is ignored; a host
   `0xFA` takes over. Accept, or (b) never take over while playing. **Recommended: as specified.**
5. **ANSWERED (76): accept.** **PLAY green LED (5.4).** It becomes mid-level steady with a beat flash instead of steady. Accept, or keep it steady
   and let only the PLAY's own LED and the dot show tempo. **Recommended: accept**, falling back to "steady" if the
   device has no mid level on that LED.
6. **ANSWERED (76): separate build.** **Timing test image as a separate build (6.1)** rather than a hidden menu entry. **Recommended: separate build**
   (zero cost, proves the production path); the cost is flashing twice for a hardware session.
7. **ANSWERED (76): LEAD 2.90 ms.** **`LEAD`.** Constant 2.90 ms keeps MIDI aligned with the preview synth (M2). Accept, or shrink it after the first
   measurements to lower T4 (the table above stays valid; only T4 changes). **Recommended: accept for v1.**


## Review notes (P2-010 review, applied)
- The nested release (risk R1) must not re-enter `usb_poll` when the audio interrupt preempts it: guard with a busy flag, and the dev task must test it.

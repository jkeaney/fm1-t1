# 8. Output and dispatch

## 8.1 The micro-event queue **[V]**

Every fired pulse produces `NOTE_ON` and `NOTE_OFF` events per repeat and per chord voice, plus an optional `CC` event. They go into one shared circular queue of **256 events** (`T1MicroEvent`: tick, sub-tick, track, type, note, velocity, channel, cc number and value).

**Insertion keeps the queue time-ordered:** each push shifts later events back (insertion sort from the head). At equal tick and sub-tick, **note-offs sort before note-ons**, so a note ending exactly when its repeat starts is released first.

```
push(evt):
    if queue is full: dropped_events++ ; return false          # the event is lost
    shift later events up; store evt in order
```
The pipeline **ignores the return value** of `push`.

## 8.2 The dispatcher **[V]**

At the end of every `t1_pipeline_tick` the queue head is drained of all events with `tick <= current_tick`:

| Track | Handling |
| --- | --- |
| 0 | `NOTE_ON`: allocate a voice (§8.3) and call `synth_note_on(voice, note, velocity)`; `NOTE_OFF`: free the voice holding that note and call `synth_note_off(voice)`. `CC` is ignored. |
| 1 to 5 | build the 3-byte channel message (`0x90|ch`, `0x80|ch`, `0xB0|ch`) and push it to the UART and USB ring buffers (256 B each) |

The sub-tick is **not** compared in the pipeline's dispatcher (the host-only ISR dispatcher in `torso_t1.c` does compare it, which would delay sub-tick events by a tick; that path is unused on the device).

On the FM-1 the tick runs in the audio ISR, then `t1_pipeline_flush_midi(0, t1_usb_out)` converts the bytes into 4-byte USB-MIDI packets (cable/CIN from the status nibble) and sends them; the UART ring is read and discarded.

## 8.3 Voice allocation for track 0 **[V]**

Six voices (`T1_FM_POLYPHONY`). `t1_pipeline_alloc_voice(note, velocity)`:
1. **Retrigger:** if a voice is already playing this exact note, reuse it (update velocity and age).
2. Else take any free voice.
3. Else **steal**: the voice with the **lowest velocity**, ties broken by the **oldest age**; send it a note-off first.

`free_voice(note)` frees the first active voice with that note. On the FM-1, `synth_note_on(v, …)` maps voice *v* to a note on **Felucca track 1** (`trk_note_on`), releasing whatever that voice slot held before; Felucca then does its own voicing.
A consequence of keying by note number: two overlapping hits of the *same* note share one voice, so the first note-off releases both (the second note-off finds nothing).

## 8.4 MIDI out and stop behaviour **[V]**

- Tracks 2 to 6 appear on USB-MIDI OUT; there is no hardware MIDI OUT jack on the FM-1.
- On PLAY, STOP, external Start/Stop and when T1 mode is turned off, `t1_all_off`: kills the FM voices, **sends an immediate note-off for every queued note-off on tracks 1 to 5**, clears both queues and rings, and sends **All Notes Off (CC 123)** on each MIDI track's channel. This is what limits the damage from dropped note-offs (F9).
- A TB-3 style slide works by overlapping notes (gate longer than the step), which the default bass uses (`sustain = 90`, `time = 127`).

## 8.5 CC events **[V]**

If `cc_target > 0`, each fired pulse sends one CC (`cc_target`, `cc_value`, optionally LFSR-modulated by the CC target bit) at the pulse tick on the track's channel. CC on track 0 (the internal synth) is dropped. The CC is sent once per pulse, not once per repeat, and is not affected by `delay_ticks`.

## 8.6 Capacity **[V]**

Verified by stress tests:

| Load | Result |
| --- | --- |
| 6 tracks, 16 pulses, 4 repeats, 2-voice chords (16 events per pulse, up to 96 per step across the tracks) | **0 dropped** |
| 6 tracks, 16 pulses, **16 repeats, 4-voice chords**, 1/16, two bars | **16,416 events dropped**; 32 of 2,048 MIDI note-ons never got their note-off (they stay on until the next stop) |

The queue holds 256 events; one pulse at 16 repeats × 4 voices queues 128 events (64 on + 64 off), so two such pulses at once fill it. Dropped events are counted (`dropped_events`) but not reported anywhere on the device.

## 8.7 Invariants (tests)

- Events leave the queue in non-decreasing tick order, offs before ons at the same tick.
- Every note-on pushed is matched by a note-off unless the queue reported a drop.
- A track-0 note never uses more than 6 voices at once; stealing picks the lowest velocity then the oldest.
- On stop, the queue and rings are empty and each MIDI channel has received CC 123.

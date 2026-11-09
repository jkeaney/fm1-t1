# 5. Timing

## 5.1 Division → note-value name → period **[V]**

`TimeDiv` combines Division (`1/1` to `1/64`) with the Dotted / Triplet switches into a Max note-value symbol:

| Division | Plain | Dotted | Triplet |
| --- | --- | --- | --- |
| 1/1 | `1n` | `1nd` | `1nt` |
| 1/2 | `2n` | `2nd` | `2nt` |
| … | `4n`, `8n`, `16n`, `32n` | `…nd` | `…nt` |
| 1/64 | `64n` | `64nd` | `64nt` |

Ticks (480 per quarter ⇒ 1920 per whole note): plain = `1920 / d`; dotted = ×1.5; triplet = ×2/3. Example: `16n` = 120 ticks; `16nt` = 80; `16nd` = 180.
The Max `translate` object cannot convert `64nt`, so that value is hard-coded as **20 ticks** (frequency path: 48 Hz special case). **[V]**

## 5.2 The clock (`Triggers`) **[V structure]**

A signal-rate **phasor** runs at **half the step frequency** (one cycle = two steps) so swing can place the second step inside the cycle. Two edge detectors produce the step triggers:

```
phasor period = 2 * step_ticks        # locked to the Live transport when `Sync` + playing
trigger A: when phase wraps to 0                    (the "on-beat" step)
trigger B: when phase passes swing_threshold        (the "off-beat" step)
swing_threshold = map(swing, 0..1 → 0.5 .. 0.749)
```
- `swing = 0` → threshold 0.5: the second step is exactly halfway (straight timing).
- `swing = 1` → 0.749: the second step is pushed to about three quarters through the cycle.
- The phasor is gated so the trigger only goes up after the gate is open (prevents a missed first trigger), and its phase is reset when the lock is released or when the device is enabled.

## 5.3 Sync vs free-running **[V]**

| Time Mode | Behaviour |
| --- | --- |
| `Sync` | phasor locked to the Live transport at the chosen note value; used when the transport is playing |
| `Freq` (unsynced) | step frequency from the manual BPM: `step_hz = BPM / 15` (sixteenths per second), halved for the phasor; Division is expressed as a frequency in the translate path |

`Lock` is true only when the transport is playing **and** sync is selected; when the transport stops the clock re-phases on restart.

## 5.4 Step counter (`MasterClock`) **[V]**

A counter runs from 1 to `LoopLength` and is advanced by each trigger. `StepDirection` selects its mode:

| StepDirection | Counter behaviour |
| --- | --- |
| `Forwards` | 1, 2, … L, 1, … |
| `Backwards` | L, L−1, … 1, L, … |
| `Alternate` | 1 … L … 1 (up and down; ⚠ whether the end steps repeat) |
| `Random` | a new random step each trigger, `random(L)` (⚠ range and repeat behaviour) |

The same step number is broadcast to the step on/off row and all lanes.

## 5.5 Key Retrig **[V structure]**

With **Key Retrig** on, a new key press (note-on with velocity ≠ 0) sends a reset to the step counter so the pattern restarts at step 1 together with the new chord. The step counter is also reset when the transport stops. The note-engine counter resets when the held-note list becomes non-empty.

## 5.6 Tempo **[V]**

The host tempo is read through the Live API (`live_set` → `tempo`) and used for the manual BPM display and for the frequency conversions.

## 5.7 Invariants (tests)

- A 16th at 120 BPM → 4 steps per beat → 8 Hz step rate; the phasor runs at 4 Hz.
- With `swing = 0`, the off-beat trigger lands at exactly half a cycle.
- `Backwards` with L = 4 → 4 3 2 1 4 3 …
- A dotted note has period 1.5× the plain note; a triplet has 2/3×.

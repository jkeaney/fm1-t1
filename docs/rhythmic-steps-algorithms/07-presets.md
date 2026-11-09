# 7. Preset review (all 19 factory presets, decoded)

The 19 `.adv` presets in the pack are gzip-compressed XML files. Each stores the 59 device parameters plus a text blob (hex-encoded JSON) holding the step data for **4 lanes × 5 layers × 16 steps**.
The tables below were decoded from those files. Step levels are 0 to 6 (see [04](04-lane-layers.md) for what each level means).

**How to read a lane:** five 16-character strings, one per layer. `Chance` is the trigger layer (0 = never, 6 = always). `Velo` 0 to 6 maps to velocity 10 to 127. `Alt` picks the alternate pad note, `Roll` adds repeats, `Slide` is the modulation (MPE slide) value.

Pad note names assume a standard General MIDI drum mapping (36 = kick); the pad notes can be changed per preset. ⚠ Names are labels for orientation only.

## Constant Variations

- Stored name: *Constant Variations 100 bpm*
- Clock speed: 100% · Global shuffle: 100% · Shuffle length: 3 · Shuffle values: [0.0, 10.2, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
- Roll rate: 1/32t · Auto retrigger: off

| Lane | Pad note (alt) | Delay ms | Mute | Lengths G / Alt / Chance / Roll / Slide / Velo | Lock | Pattern period (steps) |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 39 Clap (37 Side stick) | 0.0 | no | 16 / 16 / 13 / 16 / 16 / 3 | off | 624 |
| 2 | 42 Closed hat (44 Pedal hat) | 0.0 | no | 5 / 16 / 16 / 16 / 16 / 16 | on | 5 |
| 3 | 38 Snare (40 Snare 2) | 0.0 | no | 16 / 16 / 8 / 16 / 16 / 2 | off | 16 |
| 4 | 36 Kick (41 Low tom) | 0.0 | no | 16 / 16 / 4 / 16 / 16 / 4 | off | 16 |

Lane 1: `Chance 0060000001106000` `Velo 2534444444444444` `Alt 0000000000000000` `Roll 0000000000100000` `Slide 0000000000000000`  
Lane 2: `Chance 3535300000000000` `Velo 3542344444444444` `Alt 0060000000000000` `Roll 0001000000000000` `Slide 0420500000000000`  
Lane 3: `Chance 0000600000000000` `Velo 4244444444445444` `Alt 0000000000006000` `Roll 0002000000000000` `Slide 0000000000000000`  
Lane 4: `Chance 6001000000000000` `Velo 5111444444444444` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  

Triggering steps per lane (chance > 0): [4, 5, 1, 2] · Expected hits per 16 steps: [2.3, 3.2, 1.0, 1.2]

## Dembow Rhythm 95 bpm

- Stored name: *Dembow Rhythm 95 bpm*
- Clock speed: 100% · Global shuffle: 100% · Shuffle length: 4 · Shuffle values: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
- Roll rate: 1/32 · Auto retrigger: on, every 2 bars

| Lane | Pad note (alt) | Delay ms | Mute | Lengths G / Alt / Chance / Roll / Slide / Velo | Lock | Pattern period (steps) |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 44 Pedal hat (37 Side stick) | 4.7 | no | 16 / 10 / 11 / 16 / 16 / 16 | off | 880 |
| 2 | 42 Closed hat (46 Open hat) | 0.0 | no | 16 / 16 / 15 / 16 / 16 / 16 | off | 240 |
| 3 | 38 Snare (40 Snare 2) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 4 | 36 Kick (41 Low tom) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |

Lane 1: `Chance 6000660060006600` `Velo 3533413435455132` `Alt 6060060600000000` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 2: `Chance 0006066000666000` `Velo 3555453554433445` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 3: `Chance 0006006000060060` `Velo 4444444444444444` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 4: `Chance 6000600060006000` `Velo 4444444444444444` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  

Triggering steps per lane (chance > 0): [6, 6, 4, 4] · Expected hits per 16 steps: [6.0, 6.0, 4.0, 4.0]

## DnB Steppa 165 bpm

- Stored name: *Steppa 175 bpm*
- Clock speed: 100% · Global shuffle: 100% · Shuffle length: 4 · Shuffle values: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
- Roll rate: 1/32 · Auto retrigger: on, every 2 bars

| Lane | Pad note (alt) | Delay ms | Mute | Lengths G / Alt / Chance / Roll / Slide / Velo | Lock | Pattern period (steps) |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 37 Side stick (40 Snare 2) | 8.7 | no | 16 / 16 / 16 / 16 / 16 / 12 | off | 48 |
| 2 | 42 Closed hat (46 Open hat) | 0.0 | no | 16 / 16 / 16 / 15 / 16 / 14 | off | 1680 |
| 3 | 38 Snare (40 Snare 2) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 4 | 36 Kick (41 Low tom) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |

Lane 1: `Chance 0000000606000606` `Velo 4444224142445441` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 2: `Chance 6060606060606060` `Velo 4445234344454535` `Alt 0000000000600000` `Roll 0003000000000000` `Slide 0000000000000000`  
Lane 3: `Chance 0000600000600000` `Velo 4444444444442444` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 4: `Chance 6000000000000000` `Velo 5442445444445444` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  

Triggering steps per lane (chance > 0): [4, 8, 2, 1] · Expected hits per 16 steps: [4.0, 8.0, 2.0, 1.0]

## Funky Drummer 100 bpm

- Stored name: *Funky Drummer 100 bpm (PMA edit)*
- Clock speed: 100% · Global shuffle: 100% · Shuffle length: 4 · Shuffle values: [0.0, 2.4, 0.8, 3.1, 0.0, 0.0, 0.0, 0.0]
- Roll rate: 1/32 · Auto retrigger: off

| Lane | Pad note (alt) | Delay ms | Mute | Lengths G / Alt / Chance / Roll / Slide / Velo | Lock | Pattern period (steps) |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 39 Clap (37 Side stick) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 2 | 42 Closed hat (46 Open hat) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 3 | 38 Snare (40 Snare 2) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 4 | 36 Kick (41 Low tom) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |

Lane 1: `Chance 0000000000000000` `Velo 4444444444444444` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 2: `Chance 6666666666666666` `Velo 1242124412421442` `Alt 0000000600000600` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 3: `Chance 0000600606066006` `Velo 4444644111214221` `Alt 0000000000060006` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 4: `Chance 6060006000600600` `Velo 5424444444444444` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  

Triggering steps per lane (chance > 0): [0, 16, 6, 5] · Expected hits per 16 steps: [0.0, 16.0, 6.0, 5.0]

## Garage Beat 135 bpm

- Stored name: *Garage Beat 135 bpm (PMA edit)*
- Clock speed: 100% · Global shuffle: 100% · Shuffle length: 4 · Shuffle values: [0.0, 40.0, 0.0, 39.8, 0.0, 15.0, -7.5, 32.0]
- Roll rate: 1/8t · Auto retrigger: off

| Lane | Pad note (alt) | Delay ms | Mute | Lengths G / Alt / Chance / Roll / Slide / Velo | Lock | Pattern period (steps) |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 44 Pedal hat (37 Side stick) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 3 | off | 48 |
| 2 | 42 Closed hat (46 Open hat) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 3 | 38 Snare (40 Snare 2) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 4 | 36 Kick (41 Low tom) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |

Lane 1: `Chance 0006600000066000` `Velo 2323232123222322` `Alt 0000000000000000` `Roll 0000100000001000` `Slide 0000000000000000`  
Lane 2: `Chance 0060006000600060` `Velo 4444442444444424` `Alt 0000000000000000` `Roll 0010000000100000` `Slide 0000000000000000`  
Lane 3: `Chance 0000600606006000` `Velo 4444544000445444` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 4: `Chance 6000060600600000` `Velo 5255555555555552` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  

Triggering steps per lane (chance > 0): [4, 4, 4, 4] · Expected hits per 16 steps: [4.0, 4.0, 4.0, 4.0]

## Hip Hop Swing 90 bpm

- Stored name: *Hip Hop Swing 90 bpm*
- Clock speed: 100% · Global shuffle: 100% · Shuffle length: 8 · Shuffle values: [0.0, 0.0, 9.4, 0.0, 12.6, 0.0, 11.4, 18.9]
- Roll rate: 1/8. · Auto retrigger: off

| Lane | Pad note (alt) | Delay ms | Mute | Lengths G / Alt / Chance / Roll / Slide / Velo | Lock | Pattern period (steps) |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 39 Clap (37 Side stick) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 2 | 42 Closed hat (46 Open hat) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 3 | 38 Snare (40 Snare 2) | 8.3 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 4 | 36 Kick (41 Low tom) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |

Lane 1: `Chance 0000600000006000` `Velo 4444444444444434` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 2: `Chance 6060606060606060` `Velo 4444444444444444` `Alt 0000000000000050` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 3: `Chance 0000600000006000` `Velo 4444544444445444` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 4: `Chance 6000000660000030` `Velo 6444444354444444` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  

Triggering steps per lane (chance > 0): [2, 8, 2, 4] · Expected hits per 16 steps: [2.0, 8.0, 2.0, 3.5]

## House Variations 125 bpm

- Stored name: *House Variations 125 bpm*
- Clock speed: 100% · Global shuffle: 100% · Shuffle length: 4 · Shuffle values: [0.0, 24.9, -6.2, 24.9, 0.0, 0.0, 0.0, 0.0]
- Roll rate: 1/16. · Auto retrigger: on, every 4 bars

| Lane | Pad note (alt) | Delay ms | Mute | Lengths G / Alt / Chance / Roll / Slide / Velo | Lock | Pattern period (steps) |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 43 Floor tom (47 Mid tom) | 0.0 | no | 16 / 16 / 13 / 16 / 16 / 16 | off | 208 |
| 2 | 42 Closed hat (46 Open hat) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 3 | 39 Clap (40 Snare 2) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 4 | 36 Kick (41 Low tom) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |

Lane 1: `Chance 6006006006006006` `Velo 3222222322222223` `Alt 1111111111111111` `Roll 0000000002000000` `Slide 0640144366505164`  
Lane 2: `Chance 6161616161616161` `Velo 0313432233312131` `Alt 1111111111111111` `Roll 0000000000000000` `Slide 2006445215461333`  
Lane 3: `Chance 0000600000006300` `Velo 4444544444445144` `Alt 0000000000000400` `Roll 0000000000000000` `Slide 0161432260246105`  
Lane 4: `Chance 6002600060006020` `Velo 5442544454446424` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  

Triggering steps per lane (chance > 0): [6, 16, 3, 6] · Expected hits per 16 steps: [6.0, 9.3, 2.5, 4.7]

## Impeach The Drummer 90 bpm

- Stored name: *Impeach The Drummer 90 bpm*
- Clock speed: 100% · Global shuffle: 100% · Shuffle length: 4 · Shuffle values: [0.0, -0.8, 4.7, -0.8, 0.0, 0.0, 0.0, 0.0]
- Roll rate: 1/32 · Auto retrigger: off

| Lane | Pad note (alt) | Delay ms | Mute | Lengths G / Alt / Chance / Roll / Slide / Velo | Lock | Pattern period (steps) |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 39 Clap (37 Side stick) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 2 | 42 Closed hat (46 Open hat) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 3 | 38 Snare (40 Snare 2) | 2.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 4 | 36 Kick (41 Low tom) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |

Lane 1: `Chance 0000000000000000` `Velo 4444444444444444` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 2: `Chance 6060606660606060` `Velo 2444342424443444` `Alt 0000000000600000` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 3: `Chance 0000600000006000` `Velo 4444644444446444` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 4: `Chance 6000000660600060` `Velo 5444444254344414` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  

Triggering steps per lane (chance > 0): [0, 9, 2, 5] · Expected hits per 16 steps: [0.0, 9.0, 2.0, 5.0]

## Improv Rhythms

- Stored name: *Improv Rhythms*
- Clock speed: 100% · Global shuffle: 100% · Shuffle length: 4 · Shuffle values: [0.0, 6.3, 3.1, 6.3, 0.0, 0.0, 0.0, 0.0]
- Roll rate: 1/32t · Auto retrigger: off

| Lane | Pad note (alt) | Delay ms | Mute | Lengths G / Alt / Chance / Roll / Slide / Velo | Lock | Pattern period (steps) |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 46 Open hat (37 Side stick) | 0.0 | no | 16 / 16 / 15 / 16 / 16 / 16 | off | 240 |
| 2 | 42 Closed hat (46 Open hat) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 3 | 38 Snare (40 Snare 2) | 0.0 | no | 16 / 16 / 14 / 15 / 16 / 16 | off | 1680 |
| 4 | 36 Kick (41 Low tom) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |

Lane 1: `Chance 1412121414121214` `Velo 1524342614363626` `Alt 0000000000000000` `Roll 0000000030000000` `Slide 0000000000000000`  
Lane 2: `Chance 3132323131323231` `Velo 5342535362525261` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 3: `Chance 1111611111116111` `Velo 1111511115115111` `Alt 0000000000000000` `Roll 0000000000000100` `Slide 0000000000000000`  
Lane 4: `Chance 6111111611611111` `Velo 5111111511511111` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  

Triggering steps per lane (chance > 0): [16, 16, 16, 16] · Expected hits per 16 steps: [5.3, 6.0, 4.3, 5.2]

## Percussion Shuffled

- Stored name: *Percussion Shuffled*
- Clock speed: 100% · Global shuffle: 100% · Shuffle length: 4 · Shuffle values: [0.0, 20.5, 0.0, -37.4, 0.0, 0.0, 0.0, 0.0]
- Roll rate: 1/32 · Auto retrigger: on, every 2 bars

| Lane | Pad note (alt) | Delay ms | Mute | Lengths G / Alt / Chance / Roll / Slide / Velo | Lock | Pattern period (steps) |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 47 Mid tom (48 Hi-mid tom) | 0.0 | no | 16 / 16 / 12 / 16 / 16 / 16 | off | 48 |
| 2 | 43 Floor tom (45 Low-mid tom) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 3 | 42 Closed hat (44 Pedal hat) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 4 | 37 Side stick (36 Kick) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |

Lane 1: `Chance 6000600060060000` `Velo 3322432222422442` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 2: `Chance 6006000666060000` `Velo 6435654354253545` `Alt 6000000060060000` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 3: `Chance 0600060000006600` `Velo 4222222242222222` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0200030000006200`  
Lane 4: `Chance 6006000600060606` `Velo 3333333333333333` `Alt 0000000000000606` `Roll 0000000000000000` `Slide 0000000000000000`  

Triggering steps per lane (chance > 0): [4, 6, 4, 6] · Expected hits per 16 steps: [4.0, 6.0, 4.0, 6.0]

## Polymetric Chill

- Stored name: *Polymetric Chill*
- Clock speed: 100% · Global shuffle: 100% · Shuffle length: 4 · Shuffle values: [0.0, 3.9, 0.8, 5.5, 0.0, 0.0, 0.0, 0.0]
- Roll rate: 1/8. · Auto retrigger: on, every 4 bars

| Lane | Pad note (alt) | Delay ms | Mute | Lengths G / Alt / Chance / Roll / Slide / Velo | Lock | Pattern period (steps) |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 46 Open hat (44 Pedal hat) | 0.0 | no | 16 / 16 / 15 / 16 / 16 / 16 | off | 240 |
| 2 | 42 Closed hat (44 Pedal hat) | 0.0 | no | 16 / 4 / 13 / 16 / 16 / 16 | off | 208 |
| 3 | 38 Snare (40 Snare 2) | 0.0 | no | 16 / 16 / 14 / 16 / 16 / 16 | off | 112 |
| 4 | 36 Kick (41 Low tom) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |

Lane 1: `Chance 6000000000000000` `Velo 4444444444444444` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 2: `Chance 0636036003606066` `Velo 0140014100014021` `Alt 0000000000000000` `Roll 0000000000003000` `Slide 1320311201111323`  
Lane 3: `Chance 0000600000060060` `Velo 4444544444554342` `Alt 0000060006000000` `Roll 4000000003000000` `Slide 0000000000000000`  
Lane 4: `Chance 6000000660000600` `Velo 4444444444444444` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  

Triggering steps per lane (chance > 0): [1, 10, 3, 4] · Expected hits per 16 steps: [1.0, 8.5, 3.0, 4.0]

## Pop-Rock Driver 110 bpm

- Stored name: *Bo Diddley Beat 110 bpm*
- Clock speed: 100% · Global shuffle: 100% · Shuffle length: 4 · Shuffle values: [0.0, 11.8, 0.0, 11.8, 0.0, 0.0, 0.0, 0.0]
- Roll rate: 1/32 · Auto retrigger: off

| Lane | Pad note (alt) | Delay ms | Mute | Lengths G / Alt / Chance / Roll / Slide / Velo | Lock | Pattern period (steps) |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 39 Clap (37 Side stick) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 2 | 42 Closed hat (46 Open hat) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 3 | 38 Snare (40 Snare 2) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 4 | 36 Kick (41 Low tom) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |

Lane 1: `Chance 0000000000000000` `Velo 4444444444444444` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 2: `Chance 6060606060606060` `Velo 4444444444444444` `Alt 0000000000600000` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 3: `Chance 0060006000600060` `Velo 4444444444444444` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 4: `Chance 6006060006006000` `Velo 4444444444444444` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  

Triggering steps per lane (chance > 0): [0, 8, 4, 5] · Expected hits per 16 steps: [0.0, 8.0, 4.0, 5.0]

## RnB Playful 95 bpm

- Stored name: *RnB Playful 95 bpm*
- Clock speed: 100% · Global shuffle: 100% · Shuffle length: 4 · Shuffle values: [2.0, 0.0, 0.0, 6.3, 0.0, 0.0, 0.0, 0.0]
- Roll rate: 1/32 · Auto retrigger: on, every 2 bars

| Lane | Pad note (alt) | Delay ms | Mute | Lengths G / Alt / Chance / Roll / Slide / Velo | Lock | Pattern period (steps) |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 46 Open hat (50 Hi tom) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 2 | 42 Closed hat (44 Pedal hat) | 0.0 | no | 16 / 16 / 15 / 13 / 16 / 16 | off | 3120 |
| 3 | 38 Snare (37 Side stick) | 0.0 | no | 16 / 16 / 15 / 16 / 16 / 16 | off | 240 |
| 4 | 36 Kick (41 Low tom) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |

Lane 1: `Chance 0000000000600066` `Velo 4444444444444431` `Alt 0000000000000066` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 2: `Chance 6600066666606066` `Velo 3333333333335333` `Alt 0600060606000006` `Roll 0000000100000000` `Slide 4565261625434155`  
Lane 3: `Chance 0006600000000600` `Velo 2322444444444344` `Alt 6666000000000300` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 4: `Chance 6006000060060000` `Velo 5333333353333333` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  

Triggering steps per lane (chance > 0): [3, 11, 3, 4] · Expected hits per 16 steps: [3.0, 11.0, 3.0, 4.0]

## Robot Takeover

- Stored name: *Robot Takeover*
- Clock speed: 100% · Global shuffle: 100% · Shuffle length: 4 · Shuffle values: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
- Roll rate: 1/16 · Auto retrigger: off

| Lane | Pad note (alt) | Delay ms | Mute | Lengths G / Alt / Chance / Roll / Slide / Velo | Lock | Pattern period (steps) |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 42 Closed hat (46 Open hat) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 2 | 37 Side stick (39 Clap) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 3 | 36 Kick (40 Snare 2) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 4 | 44 Pedal hat (48 Hi-mid tom) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |

Lane 1: `Chance 0333330000333330` `Velo 3535544443343334` `Alt 2222222222222222` `Roll 2000000200000020` `Slide 2313241556442504`  
Lane 2: `Chance 6030306006030306` `Velo 4333545555534544` `Alt 2222222222222222` `Roll 0200002020000202` `Slide 2122444463055451`  
Lane 3: `Chance 6033306006033306` `Velo 4433433545334544` `Alt 2222222222222222` `Roll 0020020002002000` `Slide 1404055203430350`  
Lane 4: `Chance 0333330330333330` `Velo 3535354535434534` `Alt 2222222222222222` `Roll 0000200000200000` `Slide 4060235021630414`  

Triggering steps per lane (chance > 0): [10, 8, 10, 12] · Expected hits per 16 steps: [5.0, 6.0, 7.0, 6.0]

## Shuffle Steps 110 bpm

- Stored name: *Shuffle Steps 110 bpm*
- Clock speed: 100% · Global shuffle: 100% · Shuffle length: 4 · Shuffle values: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
- Roll rate: 1/16t · Auto retrigger: on, every 2 bars

| Lane | Pad note (alt) | Delay ms | Mute | Lengths G / Alt / Chance / Roll / Slide / Velo | Lock | Pattern period (steps) |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 50 Hi tom (44 Pedal hat) | 0.0 | no | 16 / 16 / 16 / 16 / 13 / 4 | off | 208 |
| 2 | 46 Open hat (42 Closed hat) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 3 | 38 Snare (37 Side stick) | 3.9 | no | 16 / 16 / 16 / 16 / 15 / 16 | off | 240 |
| 4 | 36 Kick (41 Low tom) | 0.0 | no | 16 / 16 / 16 / 16 / 15 / 16 | off | 240 |

Lane 1: `Chance 6666666666666666` `Velo 2364234223424552` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 4221314005462330`  
Lane 2: `Chance 0066060606600600` `Velo 3434434344345435` `Alt 0000000000000600` `Roll 0000000000000100` `Slide 0060040006000200`  
Lane 3: `Chance 0000600000006000` `Velo 4444444444445444` `Alt 0000000000006000` `Roll 0000000000000000` `Slide 0000200000005000`  
Lane 4: `Chance 6006000000060060` `Velo 6444444444424444` `Alt 6006000000000060` `Roll 0000000000000000` `Slide 0000000000000060`  

Triggering steps per lane (chance > 0): [16, 7, 2, 4] · Expected hits per 16 steps: [16.0, 7.0, 2.0, 4.0]

## Three Over Four

- Stored name: *Three Over Four*
- Clock speed: 100% · Global shuffle: 100% · Shuffle length: 4 · Shuffle values: [0.0, 40.0, 0.0, -42.0, 0.0, 0.0, 0.0, 0.0]
- Roll rate: 1/8. · Auto retrigger: on, every 2 bars

| Lane | Pad note (alt) | Delay ms | Mute | Lengths G / Alt / Chance / Roll / Slide / Velo | Lock | Pattern period (steps) |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 50 Hi tom (48 Hi-mid tom) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 2 | 42 Closed hat (46 Open hat) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 6 | off | 48 |
| 3 | 38 Snare (40 Snare 2) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 4 | 36 Kick (41 Low tom) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |

Lane 1: `Chance 0000000000000000` `Velo 4444444444444444` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 2: `Chance 6060606060606060` `Velo 2342344444444444` `Alt 0000006000000000` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 3: `Chance 6000060000060000` `Velo 4444444444444444` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 4: `Chance 6000600060006000` `Velo 5424444444444444` `Alt 0000000000000006` `Roll 0000000000000000` `Slide 0000000000000000`  

Triggering steps per lane (chance > 0): [0, 8, 3, 4] · Expected hits per 16 steps: [0.0, 8.0, 3.0, 4.0]

## Trap Beat Halftime 140 bpm

- Stored name: *Trap Beat Halftime 140 bpm*
- Clock speed: 100% · Global shuffle: 100% · Shuffle length: 4 · Shuffle values: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
- Roll rate: 1/32 · Auto retrigger: on, every 4 bars

| Lane | Pad note (alt) | Delay ms | Mute | Lengths G / Alt / Chance / Roll / Slide / Velo | Lock | Pattern period (steps) |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 39 Clap (37 Side stick) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 2 | 42 Closed hat (46 Open hat) | 0.0 | no | 16 / 16 / 14 / 16 / 16 / 16 | off | 112 |
| 3 | 38 Snare (40 Snare 2) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 4 | 36 Kick (41 Low tom) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |

Lane 1: `Chance 0000000000000062` `Velo 4444444444444442` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 2: `Chance 6060606660606060` `Velo 4444444444444444` `Alt 0000003000003000` `Roll 0000000000100000` `Slide 3155166335001213`  
Lane 3: `Chance 0000000060000000` `Velo 4444444454444444` `Alt 0000000030000000` `Roll 0000000000000000` `Slide 0000000000000000`  
Lane 4: `Chance 6000006000602000` `Velo 5444443444543444` `Alt 0000000000000000` `Roll 0000000000000000` `Slide 0000000000000000`  

Triggering steps per lane (chance > 0): [2, 9, 1, 4] · Expected hits per 16 steps: [1.3, 9.0, 1.0, 3.3]

## Triplet Roller

- Stored name: *Triplet Roller 110 bpm*
- Clock speed: 100% · Global shuffle: 100% · Shuffle length: 4 · Shuffle values: [0.0, 0.0, 0.0, -50.0, 0.0, 0.0, 0.0, 0.0]
- Roll rate: 1/8t · Auto retrigger: on, every 2 bars

| Lane | Pad note (alt) | Delay ms | Mute | Lengths G / Alt / Chance / Roll / Slide / Velo | Lock | Pattern period (steps) |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 40 Snare 2 (37 Side stick) | 0.0 | no | 16 / 12 / 16 / 16 / 16 / 16 | off | 48 |
| 2 | 42 Closed hat (46 Open hat) | 19.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 3 | 38 Snare (39 Clap) | 0.0 | no | 16 / 16 / 12 / 16 / 16 / 16 | off | 48 |
| 4 | 36 Kick (41 Low tom) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |

Lane 1: `Chance 0000000660000000` `Velo 4444444444444444` `Alt 0060003000000000` `Roll 0000000020000000` `Slide 0000000000000000`  
Lane 2: `Chance 6000600000000006` `Velo 3555655555555555` `Alt 2020000000000020` `Roll 1000200000000000` `Slide 0000000000000000`  
Lane 3: `Chance 0000600000006000` `Velo 5434435235435454` `Alt 3333333333333333` `Roll 0000100000003000` `Slide 0000000000000000`  
Lane 4: `Chance 6000000060000000` `Velo 4444444444444444` `Alt 0000000000000000` `Roll 2000000000000000` `Slide 0000000000000000`  

Triggering steps per lane (chance > 0): [2, 3, 2, 2] · Expected hits per 16 steps: [2.0, 3.0, 2.0, 2.0]

## Wobbly Steps

- Stored name: *Wobbly Steps 115 bpm*
- Clock speed: 100% · Global shuffle: 100% · Shuffle length: 3 · Shuffle values: [0.0, 25.8, -25.8, 0.0, 0.0, 0.0, 0.0, 0.0]
- Roll rate: 1/8t · Auto retrigger: on, every 4 bars

| Lane | Pad note (alt) | Delay ms | Mute | Lengths G / Alt / Chance / Roll / Slide / Velo | Lock | Pattern period (steps) |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 48 Hi-mid tom (47 Mid tom) | 0.0 | no | 16 / 16 / 16 / 16 / 9 / 16 | off | 144 |
| 2 | 50 Hi tom (51 Ride) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |
| 3 | 38 Snare (37 Side stick) | 0.0 | no | 16 / 16 / 8 / 16 / 16 / 16 | off | 16 |
| 4 | 36 Kick (41 Low tom) | 0.0 | no | 16 / 16 / 16 / 16 / 16 / 16 | off | 16 |

Lane 1: `Chance 6000005000000300` `Velo 4444444444444244` `Alt 0000000000000600` `Roll 0000000000000000` `Slide 6000030000000000`  
Lane 2: `Chance 6000360020000060` `Velo 3443534335435424` `Alt 6000400000000006` `Roll 0000000000000000` `Slide 0000400000000030`  
Lane 3: `Chance 0006005000000000` `Velo 4444444414444444` `Alt 0003006000000000` `Roll 0000000000000000` `Slide 0005005002000000`  
Lane 4: `Chance 6000600060006000` `Velo 4444444444444444` `Alt 0000600000006000` `Roll 0000000000000000` `Slide 0000000000000000`  

Triggering steps per lane (chance > 0): [3, 5, 2, 4] · Expected hits per 16 steps: [2.3, 3.8, 1.8, 4.0]

## Cross-preset observations

| Preset | Lane periods (steps) | Non-zero shuffle steps | Expected hits per lane (16 steps) |
| --- | --- | --- | --- |
| Constant Variations | [624, 5, 16, 16] | 1 | [2.3, 3.2, 1.0, 1.2] |
| Dembow Rhythm 95 bpm | [880, 240, 16, 16] | 0 | [6.0, 6.0, 4.0, 4.0] |
| DnB Steppa 165 bpm | [48, 1680, 16, 16] | 0 | [4.0, 8.0, 2.0, 1.0] |
| Funky Drummer 100 bpm | [16, 16, 16, 16] | 3 | [0.0, 16.0, 6.0, 5.0] |
| Garage Beat 135 bpm | [48, 16, 16, 16] | 5 | [4.0, 4.0, 4.0, 4.0] |
| Hip Hop Swing 90 bpm | [16, 16, 16, 16] | 4 | [2.0, 8.0, 2.0, 3.5] |
| House Variations 125 bpm | [208, 16, 16, 16] | 3 | [6.0, 9.3, 2.5, 4.7] |
| Impeach The Drummer 90 bpm | [16, 16, 16, 16] | 3 | [0.0, 9.0, 2.0, 5.0] |
| Improv Rhythms | [240, 16, 1680, 16] | 3 | [5.3, 6.0, 4.3, 5.2] |
| Percussion Shuffled | [48, 16, 16, 16] | 2 | [4.0, 6.0, 4.0, 6.0] |
| Polymetric Chill | [240, 208, 112, 16] | 3 | [1.0, 8.5, 3.0, 4.0] |
| Pop-Rock Driver 110 bpm | [16, 16, 16, 16] | 2 | [0.0, 8.0, 4.0, 5.0] |
| RnB Playful 95 bpm | [16, 3120, 240, 16] | 2 | [3.0, 11.0, 3.0, 4.0] |
| Robot Takeover | [16, 16, 16, 16] | 0 | [5.0, 6.0, 7.0, 6.0] |
| Shuffle Steps 110 bpm | [208, 16, 240, 240] | 0 | [16.0, 7.0, 2.0, 4.0] |
| Three Over Four | [16, 48, 16, 16] | 2 | [0.0, 8.0, 3.0, 4.0] |
| Trap Beat Halftime 140 bpm | [16, 112, 16, 16] | 0 | [1.3, 9.0, 1.0, 3.3] |
| Triplet Roller | [48, 16, 48, 16] | 1 | [2.0, 3.0, 2.0, 2.0] |
| Wobbly Steps | [144, 16, 16, 16] | 2 | [2.3, 3.8, 1.8, 4.0] |

## Review notes

These observations come from the decoded files, not from listening to the presets.

### What the presets reveal about how the device is meant to be used

1. **Most presets are simple 16-step loops.** 5 of the 19 (*Funky Drummer*, *Hip Hop Swing*, *Impeach The Drummer*, *Pop-Rock Driver*, *Robot Takeover*) leave every layer at length 16,
   so they repeat every 16 steps. Their character comes from the step data, the shuffle sliders and (for some) the lane delay.
2. **Polymeter is concentrated in a few presets.** The longest lane periods are *RnB Playful* (3,120 steps in lane 2), *DnB Steppa* (1,680, lane 2), *Improv Rhythms* (1,680, lane 3), *Dembow Rhythm* (880, lane 1)
   and *Constant Variations* (624, lane 1). A 16-step grid at a 16th resolution is one bar, so these would not repeat for hundreds of bars without **Auto Retrigger**.
3. **Auto Retrigger exists to tame polymeter.** 11 of 19 presets turn it on, with intervals of 2 bars (the commonest, 7 presets) or 4 bars (4 presets). Presets with long periods and Auto Retrigger on
   effectively become 2 to 4 bar phrases whose *accents and ghost notes* vary against a fixed trigger pattern.
4. **Lock Length is used once.** Only *Constant Variations* uses Lock (lane 2, length 5), producing a 5-step hat against a 16-step bar, a classic 5-over-16 cross-rhythm.
5. **Every preset leaves Clock Speed at 100%, Global Shuffle at 100% and Mute off.** Variation is carried by per-step shuffle values, not by global timing controls.
6. **Shuffle is used for feel, in small amounts.** Typical non-zero shuffle values are 2 to 12 (about 0.004 to 0.024 beat). Large values appear in *Garage Beat* (40 and 39.8), *Three Over Four* (+40 and −42),
   *Triplet Roller* (−50), *Wobbly Steps* (±25.8) and *Percussion Shuffled* (+20.5 and −37.4). Five presets (*Dembow*, *DnB Steppa*, *Robot Takeover*, *Shuffle Steps*, *Trap Beat*) are completely straight.
7. **Delays are tiny and mostly used on the hat or snare.** 6 presets use a lane delay: 2 to 19 ms. The largest is 19 ms on lane 2 in *Triplet Roller*; *DnB Steppa* delays lane 1 by 8.7 ms.
8. **Alt and Roll are accents, not the main material.** Only 37 of 1,216 Roll cells and 169 of 1,216 Alt cells are non-zero, so they work as occasional embellishments.
9. **Velocity layer is mostly flat at level 4 (velocity 88) with a few accents** (62% of all cells), and short Velo layers (lengths 2 to 4 in *Constant Variations*, *Garage Beat*, *Shuffle Steps*) add rotating accents.

### Data quality observations

- **Name mismatches inside the files.** The filename and the stored display name differ for 7 presets: *DnB Steppa 165 bpm* is stored as "Steppa 175 bpm"; *Pop-Rock Driver 110 bpm* is stored as "Bo Diddley Beat 110 bpm";
  *Funky Drummer* and *Garage Beat* carry a "(PMA edit)" suffix; *Constant Variations*, *Triplet Roller* and *Wobbly Steps* have a tempo in the stored name but not in the filename. The tempo in a name is advisory only; the preset does not store a BPM. **[V]**
- **No preset sets Clock Speed or Mute**, so these code paths are not exercised by the factory presets (use your own tests).
- **The Lock/Global Length interaction is barely exercised** (one preset). Do not treat the factory set as proof of how unlocked layers use `Global Length`. See open question 1 in [06](06-rebuild-checklist.md).
- All stored cell values are in 0..6 and all lengths in 2..16, as the device's parameter ranges imply. **[V]**

### How to use the presets as a test corpus for an FM-1 rebuild

| Test | Preset | Why |
| --- | --- | --- |
| Straight grid, no shuffle | *Robot Takeover* | simplest timing; every layer length 16 |
| Heavy shuffle | *Garage Beat*, *Three Over Four*, *Triplet Roller* | ±40 to ±50 shuffle values exercise the ±0.1 beat window |
| Cross-rhythm via Lock | *Constant Variations* | 5-step lane against 16 |
| Long polymeter + retrigger | *RnB Playful*, *Improv Rhythms*, *DnB Steppa* | LCM periods above 1,600; verify retrigger cuts them off |
| Short Velo cycle | *Constant Variations* lane 1 (Velo length 3) | accent rotation |
| Roll Rate and alt notes | *Garage Beat*, *Triplet Roller* | non-default roll rates plus alt notes |
| Per-lane delay | *Dembow Rhythm*, *DnB Steppa* | small positive delays |

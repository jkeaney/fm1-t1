# PATTERNS research: reference patterns and common starter patterns (P2-014)

Done 2026-11-10 by the executor. Two honesty notes first, because the owner's rulings depend on them.

1. **Method.** Web searches (links below) for programming guides, tutorials and community pattern libraries; plus the executor's own music knowledge. No page was
   mined for note data, and **no exact transcription was available to verify**. Every shipped pattern was therefore written as a degree/semitone-relative idiom
   (SPEC 3 grammar) and tagged `G` (generic idiom) or `F` (written in the idiom of a famous record, table 2).
2. **Decision 90 versus the acceptance line.** Decision 90 says famous-song patterns ship "with their exact notes". The notes of the `F` patterns are the executor's
   best reconstruction of each riff's shape, **not a verified transcription**, and several are longer than the record's riff would need. Verify each `F` pattern against
   a transcription the owner trusts, or approve them as idiom-faithful (open item O1 in the SPEC log). The no-transcription-of-protected-melody line is met as follows:
   every shipped pattern is one or two bars of bass or riff ostinato, with the note choices altered and decoration added for morphing; none is a melody of a song.

## 1. Categories chosen by the research (owner addition 2C)

Six categories of 20 (120 patterns). The count follows the research: each of these six idioms has at least twenty distinct, widely used shapes
(a 20-pattern browse fits one encoder sweep and one category jump), and fewer categories keep the PICK control a single list.

| Category (on screen) | Covers (owner's genre list) | Why it is a category of its own |
| --- | --- | --- |
| `ACID` | acid house, acid techno, 303 lines | the reference workflow (TB-303 / TB-3): accent + slide + octave jump idiom |
| `HOUSE` | house, deep, tech house, garage/UKG, Italo house | offbeat, rolling and syncopated 4/4 basses |
| `FUNK` | funky bass, disco, soul, boogie, Motown | octave, ghost-note and walking figures |
| `TECHNO` | techno, minimal, electro, EBM, rolling bass, trance gallop | one-note ostinatos where rhythm and accent carry the pattern |
| `DUB` | dub, reggae, DnB / reese, jungle, dubstep half-time | sparse sub lines and drop figures |
| `MELODY` | melody lines, synth-pop and Italo arps, call and answer, rising/falling sequences | the only category in scale-degree mode (SPEC 3.2) |

## 2. Reference patterns (studied, not copied)

Sources are searched and read as pages; none was fetched in full. A pattern marked **F** below is the shipped pattern written in that idiom (names are style names only; titles live in this file).

| Shipped pattern | Reference (title - artist, year) | Source | What makes it a classic (the idiom we encode) |
| --- | --- | --- | --- |
| ACID 1 | Acid Tracks - Phuture, 1987 | DP-303 pattern notes (S1) | step 1 accent + slide into step 2 is the canonical 303 move; short accented root, slid octave |
| ACID 2 | Higher State of Consciousness - Josh Wink, 1995 | MusicRadar guide (S2) | sparse accented root against a single high accented figure; the sweep does the rest |
| ACID 3 | Acperience - Hardfloor, 1992 | DP-303 pattern notes (S1) | sparse 8-step punctuation with off-beat accents |
| ACID 6 | Da Funk - Daft Punk, 1995 | DP-303 pattern notes (S1) | E to C slide motif with accented peaks (written as root / octave / fifth shapes) |
| FUNK 1 | Another One Bites the Dust - Queen, 1980 | Bass Musician Magazine disco/funk patterns (S6) | two-step root figure with an octave answer and one ghost note |
| FUNK 2 | Good Times - Chic, 1979 | Bass Musician Magazine (S6) | disco eighth-note octave pump |
| FUNK 3 | Billie Jean - Michael Jackson, 1982 | '70s Funk and Disco Bass pattern book (S7) | syncopated root figure with a pickup, one octave leap |
| FUNK 15 | Le Freak - Chic, 1978 | S6 | root on the one, octave on the and-of-two, fifth/octave fill |
| TECHNO 1 | Energy Flash - Joey Beltram, 1990 | Attack Magazine eight bassline styles (S3) | offbeat rolling 16ths on one pitch, accent from the kick gap |
| TECHNO 3 | Planet Rock - Afrika Bambaataa, 1982 | Attack Magazine (S3) | electro: root, ghost root, octave and fifth on the offbeats |
| DUB 5 | Just Want Another Chance - Reese, 1988 | NI Reese bass article (S4), MusicRadar Reese (S5) | the reese: sparse long root with a minor-seventh answer (legato, detuned saws are sound design, not pattern) |
| MELODY 4 | Sweet Dreams (Are Made of This) - Eurythmics, 1983 | executor knowledge; no source page read | root-fifth-third ostinato arpeggio; chord-tone figure |
| MELODY 5 | I Feel Love - Donna Summer, 1977 | executor knowledge; no source page read | rising tonic/third/fifth 16th sequencer figure |

Sources (all from the 2026-11-10 searches; reached as search results only):

- S1 DP-303 design notes, pattern library of reconstructions of well-known records, community consensus: https://tnn1t1s.github.io/day-303/about.html
- S2 MusicRadar, The producer's guide to the Roland TB-303: https://www.musicradar.com/news/producers-guide-roland-tb-303
- S3 Attack Magazine, Low end theory: exploring eight common bassline styles: https://www.attackmagazine.com/technique/tutorials/low-end-theory-exploring-eight-common-bassline-styles/
- S4 Native Instruments, Reese bass: https://www.native-instruments.com/blogs/blog/reese-bass
- S5 MusicRadar, classic Reese bass like Renegade's Terrorist: https://www.musicradar.com/how-to/classic-reese-bass-renegade-terrorist
- S6 Bass Musician Magazine, disco bass lines, basic patterns: https://bassmusicianmagazine.com/2019/08/bass-lines-disco-basic-bass-patterns/amp/
- S7 '70s Funk and Disco Bass: 101 Groovin' Bass Patterns (book): https://www.micheomusic.com/products/70s-funk-disco-bass-101-groovin-bass-paterns
- S8 The Producer School, 5 tech house bass patterns: https://theproducerschool.com/blogs/featured-blogs/5-tech-house-bass-patterns-every-producer-needs-to-master
- S9 Soundfingers, authentic reggae dub bass tutorial: https://soundfingers.com/techniques/authentic-reggae-dub-bass-tutorial/
- S10 Elektronauts, Advice on doing TB-303 bass lines: https://www.elektronauts.com/t/advice-on-doing-tb-303-bass-lines/9483

What the sources say that the design uses: the 303 has only accent on/off and slide (S10); two notes in a bar can carry a house groove and a plain offbeat bass
sounds generic fast, so small variations matter (S8), which is why every pattern is built to be morphed; a 12-step pattern over a 4/4 kick is a classic acid-techno
trick (S2), hence `TECHNO 8` is 12 steps; disco bass is octaves with the offbeat eighths split to sixteenths (S6); dub bass is sparse and sub-heavy (S9).

### Song-title list (the naming check, decision 90)

`python3 tests/design/patterns_proto.py --check` greps `patterns.tsv`, every `firmware/src` source and header, and every `docs/design/*.tsv` for each title below
(case-insensitive) and fails if one is found. The shipped names are `<CATEGORY> <n>` only.

<!-- TITLES -->
- Acid Tracks | Phuture
- Higher State of Consciousness | Josh Wink
- Acperience | Hardfloor
- Da Funk | Daft Punk
- Another One Bites the Dust | Queen
- Good Times | Chic
- Billie Jean | Michael Jackson
- Le Freak | Chic
- Energy Flash | Joey Beltram
- Planet Rock | Afrika Bambaataa
- Just Want Another Chance | Reese
- Sweet Dreams | Eurythmics
- I Feel Love | Donna Summer
<!-- /TITLES -->

## 3. Common starter patterns (our generic idioms, tag `G`)

Written as degree-relative data (patterns.tsv); the idiom each family encodes, with the shipped ids:

| Idiom | Ids | Anchor / decoration scheme |
| --- | --- | --- |
| offbeat bass | HOUSE 1, 2, 7, 12, 17 | anchors on the offbeats; doubled or octave notes are rank 3 decoration or optional adds |
| root-fifth-octave | HOUSE 6, 8, 15; FUNK 6, 8; DUB 1, 12; TECHNO 13, 17 | root and fifth are anchors, octave answers are rank 2 |
| 16th acid with slides on weak steps | ACID 4, 7, 10, 11, 14-16, 18-20 | accented roots are anchors; slid passing notes are rank 1-3; slide flag on the weak step |
| rolling / pulsing 16ths | HOUSE 3, 16; TECHNO 6, 7, 12, 15, 20 | the offbeat 16ths are anchors; the in-between 16ths are rank 3 |
| walking bass | FUNK 5, 14, 19; DUB 3, 14, 20 | quarter-note roots/fifths anchor, approach tones rank 2-3 |
| octave disco pump | FUNK 2, 10, 16; ACID 8 | the root is the anchor, the octave rank 2 |
| sparse sub / drop | TECHNO 2, 11; DUB 2, 4, 9, 13, 17 | one or two anchors a bar; the optional adds fill in with DENS |
| call and answer | MELODY 2, 3, 12, 20 | question bar anchored, answer bar rank 2 and optional |
| rising / falling sequences | MELODY 1, 10, 14, 19 | stepwise runs, corners anchored |
| arps and ostinati | MELODY 4, 5, 6, 15, 16 | chord tones flagged `C` so a progression re-voices them |

# ADR-0005: Preset and settings flash map

Status: accepted, design locked by the owner 2026-11-13 (P7-001). Flash line numbers below are as of commit 939c424.

## Context

16 preset slots must survive power-off and a firmware update (VISION rule 5, ADR-0003 section 10, decision 80). A slot
payload is a `SeqPreset` v4 (1556 B) plus its `ModSave` sibling (100 B) = **1656 B** (ADR-0003 section 22 row 1,
ADR-0003:918), before the storage layer's 32 B commit record and CRC. The storage layer already exists and is
host-tested: every object is an A/B sector pair, the payload goes at sector offset 256, and the commit record at offset
0 is written last (firmware/src/storage.c:3-10, 17-18, 157-165). The 1 MiB part is checked at boot
(firmware/src/project.c:338, JEDEC 0x856014). This ADR picks the sectors; it changes no code.

## The full flash map (1 MiB part, 4 KiB sectors)

Ends are exclusive. "Writer" = who may erase or program it.

| Region | Start | End | Writer | What | Source |
| --- | --- | --- | --- | --- | --- |
| Flash head (SPL, isd_config) | 0x00000 | 0x04000 | nobody | boot head, never written by the loader | firmware/loader/ldr_core.c:17, 31 (`LDR_APP_LO` 0x4000) |
| App image | 0x04000 | 0x93000 | update loader | `app.bin` slot, `APP_SLOT` 0x8DFBC bytes max (the XIP `LENGTH`); the package flash is `FLASH_SIZE` 0x93000 | ldr_core.c:15-16, 31-32; tools/fm1pkg_make.py:30-31; firmware/app.ld:15; tools/build.py:269 (build fails above the slot) |
| Update records + gap | 0x93000 | 0x97000 | loader (records) | records live "above the app"; the loader may write below 0x97000 | ldr_core.c:33; firmware/loader/loader.c:21 (`FL_RANGE_OK` first range 0x4000..0x97000) |
| **Felucca main store** | 0x97000 | 0xE0000 | app (`FL_STORE_OK`) | Felucca data; the loader never writes it | firmware/hal/fm1_flash.h:38-39; ldr_core.c:35-36; loader.c:20-21 |
| - retired projects A/B x4 | 0x97000 | 0x9F000 | none | retired P3-011 | storage.c:21, 74 |
| - retired FM6 patches, copy A | 0x9F000 | 0xA0000 | none | retired, still named by `st_sector` | storage.c:22, 68-69 |
| - **presets P01..P16 (new)** | **0xA0000** | **0xC0000** | app | this ADR | below |
| - retired sample slots (rest) | 0xC0000 | 0xDC000 | none | retired P3-009; left free | storage.c:21 |
| - retired user preset banks | 0xDC000 | 0xE0000 | none | retired P3-011 | storage.c:21, 72-73 |
| M-UPGRADE loader staging | 0xE0000 | 0xE5000 | app (ota.c), loader | staged loader, 5 sectors; update record at 0xE4F00 | fm1_flash.h:42-43; ota.c:25-27 |
| Autosave A/B | 0xE5000 | 0xE7000 | app | `OBJ_AUTOSAVE` | fm1_flash.h:44-45; storage.c:70-71 |
| FM6 voice bank | 0xE7000 | 0xE8000 | none | retired P3-009 | fm1_flash.h:46-47; storage.c:29 |
| Stock firmware VM backup + records | 0xE8000 | 0xFC000 | stock only | "never written" (stock record at 0xE8F00) | fm1_flash.h:47 (comment); ldr_core.c:35 (comment) |
| **Settings A/B** | 0xFC000 | 0xFE000 | app | `OBJ_SETTINGS`: A 0xFC000, B 0xFD000 | storage.c:66-67; fm1_flash.h:40-41 |
| Retired FM6 patches, copy B | 0xFE000 | 0xFF000 | none | retired | storage.c:69; fm1_flash.h:41 |
| Tail | 0xFF000 | 0x100000 | nobody | outside every write window; header says `flash_size 0xFF000` | fm1_flash.h:41; tools/fm1pkg_make.py:33 |

The loader's whole write window is two ranges: 0x4000..0x97000 and 0xE0000..0xFC000 (loader.c:21). The app's is
`FL_STORE_OK` plus the OTA staging (fm1_flash.h:53-58): 0x97000..0xE0000, 0xFC000..0xFF000, 0xE5000..0xE8000.

## Decision

1. **Presets: object ids `OBJ_PRESET0 .. OBJ_PRESET0 + 15`, one A/B sector pair each, 32 sectors at 0xA0000..0xBFFFF.**
   Slot k (0-based, P01 = 0): copy A at `0xA0000 + k * 0x2000`, copy B at `0xA0000 + k * 0x2000 + 0x1000`. Slot 15 B
   is the last sector, 0xBF000..0xBFFFF. No new storage code is needed beyond one `st_sector` branch.
2. **Settings stay where they are**: `OBJ_SETTINGS`, 0xFC000 / 0xFD000 (storage.c:67). Nothing moves, so existing
   settings survive. `persist_t` is about 0.6 KB, far under `ST_PAYLOAD_MAX` 3840 B (storage.c:18;
   firmware/src/settings_persist.c:20-26).
3. **The autosave stays at 0xE5000 / 0xE6000** (storage.c:71). It holds the working state, not a slot.
4. **Object ids.** New ids start at 10 (the current `OBJ_COUNT`, storage.c:31-32): `OBJ_PRESET0` = 10 .. 25, then
   `OBJ_COUNT` = 26. Old ids 1..9 keep their meaning. 26 stays below `OBJ_FM6VB` 32 (storage.c:33). Because
   `st_head` requires `h->type == obj` and `h->slot == copy` (storage.c:85), any stale record from the retired sample
   slots in these sectors (types never above 9) is invalid for the new objects: the slot reads as empty until its first
   save, which erases the sector first (storage.c:150).
5. **`st_sector` order (a trap for P7-002).** The existing test `obj >= OBJ_UPRESET0` (storage.c:72) is true for the new
   ids and would send them to 0xDC000. The `OBJ_PRESET0` branch must come **before** it.
6. **Payload fit.** 256 B offset + 1656 B = 1912 B in a 4096 B sector, 2184 B free. A bigger `SeqPreset` fits up to
   `ST_PAYLOAD_MAX` 3840 B total; past that the object needs two sectors per copy (a new ADR).

### Proof of no overlap

| Region under test | Preset region 0xA0000..0xC0000 | Result |
| --- | --- | --- |
| Image: slot ends at 0x4000 + 0x8DFBC = 0x91FBC [I], hard stop 0x93000 (`FLASH_SIZE`) | starts 0xA0000 | clear by 0xD000 (52 KiB) beyond the hard stop |
| Loader write window 1: 0x4000..0x97000 (loader.c:21) | starts 0xA0000 | clear by 0x9000 |
| Loader write window 2: 0xE0000..0xFC000 (loader.c:21) | ends 0xC0000 | clear by 0x20000 |
| OTA staging 0xE0000..0xE5000 and record 0xE4F00 (ota.c:25-27) | ends 0xC0000 | clear |
| Autosave 0xE5000..0xE7000, voice bank 0xE7000..0xE8000 | ends 0xC0000 | clear |
| Settings 0xFC000..0xFE000 | ends 0xC0000 | clear |
| Retired projects / FM6 copy A 0x97000..0xA0000 | starts 0xA0000 | clear, adjacent only |
| App write guard `FL_STORE_OK` first range 0x97000..0xE0000 (fm1_flash.h:53) | inside it | allowed, no hal change |

The image can only grow into the 0x93000 hard stop, because the build refuses an image over `APP_SLOT`
(tools/build.py:269) and the loader writes only below 0x93000 for the image (ldr_core.c:31-32). The 0x4000 start of the
image in flash is read from the loader constants; the 0x91FBC figure is derived and marked [I]. Only the 0x93000 bound
is used for the proof.

## Wear and power loss

- **Wear.** A save erases and programs one 4 KiB sector of one slot and alternates between its two (storage.c:146), so
  saving slot k never touches another slot. NOR endurance for this part class is commonly 100,000 erase cycles per
  sector [I, datasheet for JEDEC 0x856014 not checked]; with two sectors per slot that is about 200,000 saves of one
  slot, far more than a producer makes. The settings pair already uses the same scheme. One save is about one 17 ms erase
  (storage_hw.c:36) plus 7 page programs for 1656 B: main loop only, never in the audio ISR (storage.c:77, storage_hw.c:41).
- **Power loss mid-save.** The old copy is never touched: the new one goes to the other sector, the 32 B header is
  written last, and on load the valid copy with the newest `seq` wins (storage.c:5-9, 98-121). Cut at any moment:
  during the erase or payload the new copy has no valid header, so the previous copy loads; during the header
  program the header CRC (storage.c:164) fails, same result; the first-ever save of a slot cut anywhere leaves it
  empty. A save is verified by read-back before it reports success (storage.c:167-172).
- **Corruption.** Bit rot in the newer copy fails the payload CRC and falls back to the older copy
  (storage.c:106-118); both bad = empty.
- **Image upgrade.** The loader erases and writes only 0x4000..0x93000 for the image and the update-record sectors
  in its two windows (ldr_core.c:15-21, loader.c:21). 0xA0000..0xBFFFF is in neither, and the settings sectors
  (0xFC000..0xFE000) sit above the loader's second window (ends 0xFC000). Presets and settings therefore survive an
  update; the stock firmware's own updater is not covered by this proof (it only ever stages above 0xE0000, ldr_core.c:34-36). A **preset version bump** (ADR-0003:918) is handled in firmware, not flash: the
  payload still has a valid CRC, so `sq_preset_load` rejects the old version and returns -1 with the live pattern
  untouched (ADR-0003 section 10).

## Decisions left to P7-002 (written down so Haiku does not guess)

- A slot with no valid record (never saved, or rejected by version) is **EMPTY**: PRESETS shows `P09 EMPTY: NOTHING
  LOADED` and changes nothing (decision 79, OWNER_QUESTIONS.md:80). Rejected-by-version is shown the same way. The
  storage layer, not `sq_preset_load`, distinguishes them; the user-visible text is the same.
- The staging copy and the "undo last load" copy are pool RAM, not flash (ADR-0003 section 10, SEQUENCER_UX 9.5).
- `OBJ_COUNT` and the `st_sector` branch are the only storage.c changes. `FL_STORE_OK` already admits the region.

## Host test plan for P7-002 (tests/preset_test.c, on the simulated NOR of tests/storage_test.c)

1. **Region proof.** For all 16 slots and both copies, `st_sector()` is `0xA0000 + k*0x2000 + c*0x1000`, `FL_STORE_OK`
   accepts it, and no sector falls in 0x4000..0x97000, 0xE0000..0x100000. 32 distinct sectors.
2. **Empty slot.** Fresh (0xFF) flash: `st_load` fails for every slot; the preset code reports EMPTY; the live pattern
   is byte-identical after the call.
3. **Round trip.** Save then load each of 16 distinct presets (each with its 100 B `ModSave`); slots do not bleed
   into each other (erase-count map shows only the saved slot's two sectors were erased).
4. **Power loss mid-write.** A fault hook on the fake NOR cuts power after the erase, after each 256 B page, and
   after the header program (and mid-header at each byte). Per cut: reload gives the previous payload exactly, or
   EMPTY for a first save; a second save afterwards succeeds. Also cut during the *second* save of a slot.
5. **Corruption.** Flip a bit in the newer copy's payload: older copy loads. Flip one in both: EMPTY. Flip a header
   byte: that copy ignored.
6. **Version mismatch.** A valid-CRC payload with `version` 3 and with 5: `sq_preset_load` returns -1, live state
   untouched, the slot shows EMPTY, the slot is not erased by loading.
7. **Stale sectors.** Pre-fill 0xA0000..0xBFFFF with records of types 1..9 and with a valid-looking `FELU` header of
   the wrong type: all 16 slots read EMPTY; the first save works.
8. **seq wrap.** Seed a copy with `seq` 0xFFFFFFFF, save: the next copy (seq 0) wins (storage.c:105).
9. **Order trap.** `st_sector(OBJ_PRESET0 + 5, 0)` is not in 0xDC000..0xE0000 (guards decision 5).
10. **Settings coexist.** Save settings and 16 presets in any order; all load; settings' sectors 0xFC000/0xFD000 stay
    unchanged by preset saves (byte compare of both).
11. **Pseudo-random sequence (hazard rule).** A seeded LFSR drives 2000 operations (save slot k, load slot k, cut power
    mid-save, bit-flip, erase-all) against a model of "last fully committed payload per slot"; after every step each
    slot equals the model or EMPTY, no index leaves 0..15, and no byte outside the region and the settings pair ever
    changes.

## Consequences

- No hal, loader or linker change: the region is inside windows that already exist.
- The pool and RAM cost of presets is unchanged from ADR-0003 section 22 (pool +29808 B), not flash.
- Unused: 0xC0000..0xDBFFF (112 KiB) stays free as headroom for later slot growth.
- Update is safe by construction (above), but the owner should confirm with HW-4 (P7-004): save presets, run an update
  over USB, power-cycle, and check they are still there.

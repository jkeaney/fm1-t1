# SPDX-License-Identifier: GPL-3.0-only
"""Print the SYSTEM_INVENTORY.md tables. usage (repo root, after attrib.py and cov.py):
  python3 docs/design/inventory/gen_table.py build/inventory/hdr.txt
Reads build/inventory/{res,nm,cov}.json. Sizes: attributed symbol bytes; the 'unattributed' row is the section total
(objdump -h) minus what was attributed, so every column sums to the build's own section sizes."""
import json, re, sys, glob, os, collections
R = json.load(open("build/inventory/res.json"))["res"]
COV = json.load(open("build/inventory/cov.json"))
NM = json.load(open("build/inventory/nm.json"))
# (row, files, purpose, runs-in)  -- runs-in: ISR = audio/timer ISR, main = main loop, boot = once at start, mixed = both
ROWS = [
 ("libc", "libc.c", "memset/memcpy/memcmp the compiler emits (no hosted libc)", "boot+ISR"),
 ("lcd", "lcd.c", "ST7789 240x240 panel on SPI1, polled DMA", "main"),
 ("gfx", "gfx.c ui_fonts.h ui_keycaps.h ui_palettes.h", "canvas renderer (cv_*), AA text, palettes; generated font / keycap data", "main"),
 ("sound core", "core.h dsp.c engines.c", "track/part structs, DSP helpers, engine table, factory patterns, engine memory pool", "ISR (+data)"),
 ("engines: synth set", "eng_analog.c eng_phase.c eng_lofi.c eng_trio.c eng_wheel.c eng_formant.c eng_noise.c", "7 synth engines (ANALOG PHASE LOFI TRIO WHEEL VOICE NOISE)", "ISR"),
 ("engines: sample family", "eng_sample.c eng_grain.c eng_slice.c felucca_samples.h", "SAMPLE / GRAIN / SLICE engines + 126 KB ADPCM sample data", "ISR"),
 ("engine: FM6", "eng_fm6.c fm6_core.c felucca_fm6.h fm6_vbank.c fm4_convert.c", "6-op FM (msfa), factory patches, voice bank, DIGITAL->FM6 conversion (eng_digital.c is NOT built: FELUCCA_FM4=0)", "ISR (+main for bank)"),
 ("engine: PHYS", "eng_phys.c phys_dsp.c phys_symp.c", "physical-model engine (DaisySP / Rings ports)", "ISR"),
 ("engine: DRUM", "eng_drum.c drum_voice.c", "8-lane drum kit engine", "ISR"),
 ("params", "params.c", "parameter descriptors, value formatting, page table", "main (+ISR reads)"),
 ("mod matrix", "mod.c", "4-slot modulation matrix per track", "ISR"),
 ("voice", "voice.c", "per-track voice render / note on-off", "ISR"),
 ("slicer insert", "slicer.c", "per-track 16-step gate / stutter insert (+32 KB pool buffer)", "ISR"),
 ("fx", "fx.c", "mix_part, DIST/INSERT, chorus / delay / reverb buses (pool buffers)", "ISR"),
 ("perform", "perform.c", "FX-hold layer: master performance effects", "ISR"),
 ("click", "click.c", "metronome blip", "ISR"),
 ("usb", "usb.c uac_fir.h", "USB device: MIDI in/out rings, UAC audio, CDC, descriptors", "ISR (poll) + main"),
 ("midi_uart", "midi_uart.c", "TRS MIDI IN parser (RX DMA ring, polled from TIMER5)", "ISR"),
 ("song chain", "song_chain.c", "song = rows of 4 saved projects (lanes)", "ISR+main"),
 ("seq core", "seq.c", "keyboard, scale, arp, pattern sequencer, transport, events_block", "ISR"),
 ("seq: chord", "chord.c", "chord keys (CHRD / VOIC)", "ISR"),
 ("seq: motion", "motion.c", "step automation + p-locks", "ISR"),
 ("seq: midi clock/control", "midi_clock.c midi_control.c", "external MIDI clock follower; shared USB/TRS channel controls", "ISR"),
 ("T-1 core", "torso_t1_port.c torso_t1.c torso_t1_algo.c torso_t1_pipeline.c torso_t1_state.c torso_t1.h", "freestanding shims, data model + event queue, algos, 192-PPQN pipeline, TEMP/GLOBAL state (torso_t1.h holds the .pool buffers)", "ISR (state/ui helpers: main)"),
 ("T-1 ui helper", "torso_t1_ui.c", "knob->parameter mapping and card text", "main"),
 ("T-1 glue", "t1_glue.c", "T-1 clock/transport/MIDI-out in the audio ISR", "ISR"),
 ("T-1 screen", "ui_t1.c", "T-1 screen + controls (its functions are inlined into ui_input.c / ui_draw.c: see caveats)", "main"),
 ("audio", "audio.c", "I2S ISR, events_block caller, mix to codec / USB", "ISR"),
 ("panel", "panel.c", "button / encoder -> printed-label table, calibration", "main"),
 ("ui core", "ui.c category.c favorites.c midi_learn.c ui_events.c ui_slice.c", "UI state, page logic, categories, favorites, MIDI learn, automation list, slice edit", "main"),
 ("menu", "menu_items.c ui_menu.c", "MENU rows / settings, shared with the editor; menu screen", "main"),
 ("icons", "icons.c ui_icons.h", "Fukiai icon cells (Huffman-coded)", "main"),
 ("ui graphs", "ui_graph.c ui_fm6op.c", "page graphs, scope, piano roll, FM6 operator pages", "main"),
 ("ui draw", "ui_draw.c ui_help.c", "header, knob cards, footer, frame, HELP hints", "main"),
 ("ui input", "ui_input.c ui_name.c", "LEDs, knob / button handling, step entry, NAME screen", "main"),
 ("ui layers", "ui_layer.c ui_tools.c", "quick layers (FX GLO SCL EDIT REC held), SEQ TOOLS", "main"),
 ("storage", "storage_hw.c storage.c", "SPI-NOR A/B-sector object store (flash HW side + format)", "main"),
 ("user presets", "upreset.c up_fm6.c", "32 user presets (RAM mirror + flash), FM6 patches", "main"),
 ("project", "project.c settings_persist.c slice_store.c", "4 project slots (.noinit + flash), settings persistence, slice store", "main"),
 ("ota", "ota.c ota_hw.c", "M-UPGRADE SysEx firmware update entry", "main"),
 ("editor", "editor.c editor_backup.c editor_fm6.c editor_learn.c editor_menu.c editor_palette.c editor_preferences.c", "web-editor SysEx protocol", "main"),
 ("console", "console.c", "read-only CDC serial console", "main"),
 ("master", "master.c", "MASTER pot read before audio start", "boot+main"),
 ("main", "main.c", "boot sequence and the ~60 fps main loop", "boot+main"),
 ("HAL + tables", "fm1_adc.h fm1_flash.h fm1_guard.h fm1_input.h fm1_irq.h fm1_lcd_hw.h fm1_sys.h fm1_time.h fm1_led_anim.h felucca_tables.h", "register-level hal/*.h (header-only; key/LED scan, flash driver in .ram_text) + generated tables", "ISR+boot"),
]
SECS = {}
for ln in open(sys.argv[1]):
    p = ln.split()
    if len(p) >= 4 and p[1].startswith("."): SECS[p[1]] = int(p[2], 16)
fileset = {f for _, fs, _, _ in ROWS for f in fs.split()}
file2row = {f: r for r, fs, _, _ in ROWS for f in fs.split()}
missing = [f for f in R if f not in fileset and not f.startswith("?")]
def g(files, sec): return sum(R.get(f, {}).get(sec, 0) for f in files)
DEPS = collections.defaultdict(collections.Counter)
dj = json.load(open("build/inventory/deps.json")) if os.path.exists("build/inventory/deps.json") else {}
for a, d in dj.items():
    for b, n in d.items():
        ra, rb = file2row.get(a), file2row.get(b)
        if ra and rb and ra != rb: DEPS[ra][rb] += n
USEDBY = collections.defaultdict(collections.Counter)
for a, d in DEPS.items():
    for b, n in d.items(): USEDBY[b][a] += n
def short(c, k=5): return ", ".join(r for r, _ in c.most_common(k)) or "-"
def tests(files):
    ts = set()
    for f in files:
        ts |= set(COV.get(f, []))
    if not ts: return "none (hardware only)"
    return f"{len(ts)}: " + ", ".join(sorted(ts)[:4]) + (" ..." if len(ts) > 4 else "")
print("| Subsystem | Files | Purpose | Flash B (.text+.ram_text+.data) | RAM B (.data+.bss) | Pool B | .noinit B | Runs in | Depends on | Depended on by | Host tests covering it |")
print("|---|---|---|---:|---:|---:|---:|---|---|---|---|")
tot = collections.Counter()
for r, fs, pur, run in ROWS:
    files = fs.split()
    fl = g(files, ".text") + g(files, ".ram_text") + g(files, ".data")
    ram = g(files, ".data") + g(files, ".bss"); pool = g(files, ".pool"); ni = g(files, ".noinit")
    for k, v in (("fl", fl), ("ram", ram), ("pool", pool), ("ni", ni)): tot[k] += v
    print(f"| {r} | {fs} | {pur} | {fl} | {ram} | {pool} | {ni} | {run} | {short(DEPS[r])} | {short(USEDBY[r])} | {tests(files)} |")
um = {s: R.get("?unmatched", {}).get(s, 0) + R.get("?ambiguous:seq.c|t1_glue.c", {}).get(s, 0) for s in (".text", ".bss", ".data", ".pool", ".noinit")}
for k in R:
    if k.startswith("?") and k != "?unmatched": print("<!-- stray key", k, R[k], "-->", file=sys.stderr)
fl_a = sum(R[k].get(".text", 0) + R[k].get(".ram_text", 0) + R[k].get(".data", 0) for k in R)
ram_a = sum(R[k].get(".data", 0) + R[k].get(".bss", 0) for k in R)
pool_a = sum(R[k].get(".pool", 0) for k in R); ni_a = sum(R[k].get(".noinit", 0) for k in R)
FL = SECS[".text"] + SECS[".ram_text"] + SECS[".data"]; RAM = SECS[".data"] + SECS[".bss"]
print(f"| (symbols not matched to a file) | - | `.L_*` leftovers, compiler-generated, unmatched statics | {fl_a - tot['fl']} | {ram_a - tot['ram']} | {pool_a - tot['pool']} | {ni_a - tot['ni']} | - | - | - | - |")
print(f"| (unnamed bytes) | - | string literals, switch tables, padding, asm objects (crt0, vectors, isr): no symbol | {FL - fl_a} | {RAM - ram_a} | {SECS['.pool'] - pool_a} | {SECS['.noinit'] - ni_a} | - | - | - | - |")
print(f"| **Total (objdump -h sections)** | | | **{FL}** | **{RAM}** | **{SECS['.pool']}** | **{SECS['.noinit']}** | | | | |")
print("missing files from ROWS:", missing, file=sys.stderr)

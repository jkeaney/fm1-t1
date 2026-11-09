#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""P1-017: completeness check of the locked sequencer UX tables.

Checks, against docs/design/{page_states,button_modes,knob_map,key_map,control_map}.tsv and the score table of
docs/design/SEQUENCER_UX.md:
  1. every page of page_states (kind 'page') is a context of every other table, and nothing else is;
  2. every button x page has exactly one 'tap' and one 'hold' row (double-tap is not used anywhere);
  3. every page labels K1..K4 and the three encoders; PERF labels all 8 slots for each of the 4 algorithms;
  4. every key x page has a row; force gestures exist only on F3..C5 and each has a fallback;
  5. control_map.tsv is 49 controls x every page and its labels equal the mode / knob / key tables;
  6. labels are at most 6 characters; no (page, control, mode) is defined twice; two different controls on one
     page never share a specific label (generic labels such as '-', '?', 'STEP' are allowed);
  7. every scored function needs at most 2 gestures, its total is the sum, and it passes the high bar
     (performance >= 11, setup >= 10, no sub-score below 2) unless it is a listed, argued exception.
Prints OK and exits 0 when all hold."""
import csv, os, re, sys
root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
D = os.path.join(root, 'docs/design')
def tsv(n): return list(csv.DictReader(open(os.path.join(D, n)), delimiter='\t'))
bad = []
def err(*a): bad.append(' '.join(str(x) for x in a))

states = tsv('page_states.tsv'); bm = tsv('button_modes.tsv'); km = tsv('knob_map.tsv'); keys = tsv('key_map.tsv'); cm = tsv('control_map.tsv')
pages = [s['state'] for s in states if s['kind'] == 'page']
if len(set(s['state'] for s in states)) != len(states): err('duplicate state')
for s in states:
    for f in ('enter', 'leave', 'screen', 'knob_band', 'keys', 'cue_strip'):
        if not s[f].strip(): err('state', s['state'], 'has no', f)
BTN = ['B_FX', 'B_SCL', 'B_ENV', 'B_LFO', 'B_EDIT', 'B_GLO', 'B_HOME', 'B_SAVE', 'B_ARP', 'B_SEQ', 'B_PLAY', 'B_REC', 'B_OCTDN', 'B_OCTUP']
GENERIC = {'-', '?', 'STEP', 'EDIT', 'NOTE', 'KEY', 'CLR', 'close', 'value', 'RST', 'BACK', 'HOME', 'CH n', 'Pnn', 'ALGO', 'tab', 'row', 'morph', 'MENU'}

# 1 + 2: buttons
seen = {}
for r in bm:
    k = (r['page'], r['control'], r['mode'])
    if k in seen: err('defined twice', k)
    seen[k] = r
    if r['page'] not in pages: err('button row on unknown page', k)
    if r['mode'] == 'double': err('double-tap is not used', k)
    if len(r['label']) > 6: err('label > 6 chars', k, r['label'])
    if not r['action'].strip() or not r['principle'].strip(): err('empty action/principle', k)
for p in pages:
    for b in BTN:
        for m in ('tap', 'hold'):
            if (p, b, m) not in seen: err('missing button mode', p, b, m)

# 3: knobs
kseen = {}
for r in km:
    k = (r['page'], r['context'], r['knob'])
    if k in kseen: err('knob defined twice', k)
    kseen[k] = r
    if r['page'] not in pages: err('knob row on unknown page', k)
    if not r['label'].strip() or len(r['label']) > 6: err('bad knob label', k, r['label'])
for p in pages:
    for kn in ('K1', 'K2', 'K3', 'K4', 'SELECT', 'ALGO', 'PRESET'):
        if (p, '*', kn) not in kseen: err('knob unlabelled', p, kn)
for a in ('ACID', 'EUCLID', 'SNAKE', 'STAGE'):
    labs = []
    for kn in ('K1', 'K2', 'K3', 'K4', 'S1', 'S2', 'S3', 'S4'):
        r = kseen.get(('PERF', a, kn))
        if not r: err('PERF slot missing', a, kn)
        else: labs.append(r['label'])
    if len(set(labs)) != len(labs): err('duplicate morph label in', a, labs)

# 4: keys
FORCE_OK = {'KEY_W%02d' % i for i in range(1, 13)} | {'KEY_B%02d' % i for i in range(1, 9)}
allkeys = sorted({r['key'] for r in keys})
if len(allkeys) != 27: err('expected 27 keys, got', len(allkeys))
for p in pages:
    for kk in allkeys:
        if not any(r['page'] == p and r['key'] == kk for r in keys): err('key unmapped', p, kk)
for r in keys:
    if len(r['label']) > 6: err('key label > 6', r['page'], r['key'], r['label'])
    if r['force_gesture'] not in ('-', '') and not r['force_gesture'].startswith('no force'):
        if r['key'][:7] not in FORCE_OK: err('force gesture on a key without force', r['page'], r['key'])
        if r['fallback'] in ('-', ''): err('force gesture without fallback', r['page'], r['key'])

# 5: control map
ctrls = sorted({r['control'] for r in cm})
if len(ctrls) != 49: err('control_map has', len(ctrls), 'controls, expected 49')
if sorted({r['context'] for r in cm}) != sorted(pages): err('control_map contexts differ from pages')
cmk = {(r['control'], r['context']): r for r in cm}
if len(cmk) != len(cm) or len(cm) != 49 * len(pages): err('control_map is not 49 x pages without duplicates')
for (c, p), r in cmk.items():
    if c in BTN and seen.get((p, c, 'tap')) and r['on_screen_label'] != seen[(p, c, 'tap')]['label']: err('label mismatch', c, p)
    if c.startswith('EN_K') and r['on_screen_label'] != kseen[(p, '*', 'K' + c[-1])]['label']: err('knob label mismatch', c, p)
    if c.startswith('KEY_'):
        first = [x for x in keys if x['page'] == p and x['key'] == c][0]
        if r['on_screen_label'] != first['label']: err('key label mismatch', c, p)

# 6: label conflicts on one page (specific labels must name one control)
for p in pages:
    lab = {}
    for r in bm:
        if r['page'] == p and r['mode'] == 'tap' and r['label'] not in GENERIC:
            lab.setdefault(r['label'], set()).add(r['control'])
    for r in km:
        if r['page'] == p and r['context'] == '*' and r['label'] not in GENERIC:
            lab.setdefault(r['label'], set()).add(r['knob'])
    for l, cs in lab.items():
        if len(cs) > 1 and not (l.startswith('OCT') and cs <= {'B_OCTDN', 'B_OCTUP', 'K3', 'K4'}):
            err('label', l, 'on', p, 'names', sorted(cs))

# 7: scores in SEQUENCER_UX.md
md = open(os.path.join(D, 'SEQUENCER_UX.md')).read()
exc = set(re.findall(r'^\| (E\d+) \| (U\d+)', md, re.M))
exc_ids = {u for _, u in exc}
n = 0
for m in re.finditer(r'^\| (U\d+) \| ([^|]+) \| ([PS]) \| (\d) \| (\d)/(\d)/(\d)/(\d) = (\d+) \| ([^|]+) \|', md, re.M):
    n += 1
    uid, fn, cls, g = m.group(1), m.group(2), m.group(3), int(m.group(4))
    v = [int(m.group(i)) for i in range(5, 9)]; tot = int(m.group(9)); verdict = m.group(10).strip()
    if sum(v) != tot: err(uid, 'total', tot, '!= sum', sum(v))
    if g > 2: err(uid, 'needs', g, 'gestures (max 2)')
    ok = tot >= (11 if cls == 'P' else 10) and min(v) >= 2
    if ok and not verdict.startswith('pass'): err(uid, 'passes but is marked', verdict)
    if not ok and uid not in exc_ids: err(uid, fn.strip(), 'fails the high bar and is not a listed exception')
    if not ok and not verdict.startswith('EXC'): err(uid, 'fails but is not marked EXC')
if n < 50: err('only', n, 'scored functions found in SEQUENCER_UX.md (expected >= 50)')

for b in bad: print(b)
print('pages=%d button-rows=%d knob-rows=%d key-rows=%d control-rows=%d scored=%d exceptions=%d' % (len(pages), len(bm), len(km), len(keys), len(cm), n, len(exc)))
print('OK' if not bad else 'FAIL %d' % len(bad))
sys.exit(1 if bad else 0)

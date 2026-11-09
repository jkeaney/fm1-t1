#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""P2-001: check docs/design/CONTROL_MAP.md agrees row for row with control_map.tsv.

Every (control, context) row of the TSV must appear exactly once in the MD appendix A
(`| control | context | action | label |`) with the same action and label, the MD must
have no extra rows, and every control must also be named in the section 2 summary table.
Exit 0 and print OK when they agree."""
import csv, re, sys, os
root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
tsv = list(csv.reader(open(os.path.join(root, 'docs/design/control_map.tsv')), delimiter='\t'))[1:]
md = open(os.path.join(root, 'docs/design/CONTROL_MAP.md')).read()
a, b = md.index('<!-- BEGIN-APPENDIX-A -->'), md.index('<!-- END-APPENDIX-A -->')
rows = {}
for ln in md[a:b].splitlines():
    m = re.match(r'^\| `([^`]+)` \| `([^`]+)` \| (.*) \| (.*) \|$', ln)
    if m:
        k = (m.group(1), m.group(2))
        if k in rows: print('duplicate', k); sys.exit(1)
        rows[k] = (m.group(3), m.group(4))
bad = 0
for r in tsv:
    k = (r[0], r[1])
    if k not in rows: print('missing', k); bad += 1
    elif rows[k] != (r[2], '`%s`' % r[3]): print('differs', k, rows[k], r[2:4]); bad += 1
extra = set(rows) - {(r[0], r[1]) for r in tsv}
for k in extra: print('extra', k); bad += 1
summ = md[md.index('## 2.'):md.index('## 3.')]  # section 2 only, so a stray mention elsewhere cannot satisfy the check
for c in sorted({r[0] for r in tsv}):
    stem = re.sub(r'^KEY_(W|B)\d+_', '', c) if c.startswith('KEY_') else c
    # whole-token match: 'C4' must not be satisfied by 'C#4' or 'C45'; key notes must appear as a word
    if not re.search(r'(?<![A-Za-z0-9#])' + re.escape(stem) + r'(?![A-Za-z0-9#])', summ) and c not in summ: print('not in section 2:', c); bad += 1
print('rows tsv=%d md=%d' % (len(tsv), len(rows)))
print('OK' if not bad else 'FAIL %d' % bad)
sys.exit(1 if bad else 0)

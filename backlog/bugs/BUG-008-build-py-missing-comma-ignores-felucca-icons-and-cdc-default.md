---
id: BUG-008
title: tools/build.py line ~190 is missing a comma, so FELUCCA_ICONS and FELUCCA_CDC_DEFAULT are silently ignored
phase: P4
type: bug
status: todo
priority: 3
executor: haiku
depends_on: []
gate: none
severity: low   # pre-existing; the two env flags have no effect
found_in: P4-007 review
---
## Goal
Fix: the missing comma in `tools/build.py` near line 190 (string concatenation merges two flag names), so that `FELUCCA_ICONS` and `FELUCCA_CDC_DEFAULT` take effect or are removed if obsolete.

## Read first
- tools/build.py around lines 180-200; BUILDING.md (the documented `FELUCCA_*` flags)

## Deliverable
- Add the comma (or delete the two flags and their BUILDING.md mentions if the icons and CDC features no longer exist in the firmware: check with grep first).

## Steps to reproduce
1. Read the tuple of flag names in tools/build.py near line 190: two adjacent string literals concatenate into one name.

## Expected
Each flag name is its own entry.

## Actual
The merged name matches no env var, so both flags are ignored.

## Acceptance criteria
- [ ] `python3 -c "import ast,sys; ast.parse(open('tools/build.py').read())"` exits 0 and the tuple has separate entries for the two flags (or both are removed with BUILDING.md updated)
- [ ] `sh tools/check.sh` ends with `CHECK PASS`, with the RAM line in the Log (headroom at least 8192 B); the image is unchanged unless a flag now takes effect, in which case explain

## Verification
```sh
sh tools/check.sh
```

## UX gate
n/a, build tooling.

## Log

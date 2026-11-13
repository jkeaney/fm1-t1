---
id: P4-025
title: TRS Song Position Pointer parse (midi_uart drops system common)
phase: P4
type: task
status: todo
priority: 12
executor: sonnet
depends_on: [P4-009]
gate: none
---
## Goal
A DIN host on the TRS jack that sends SPP then Continue (M12, R15 treat both ports alike) must resume at the right step; today only USB delivers 0xF2 to clk_ext.c.

## Read first
- firmware/src/midi_uart.c (parser, lines ~30-60: system common cancels running status and is dropped)
- firmware/src/usb.c midi_in_event (the USB 0xF2 forward added in P4-009), docs/design/TIMING_SPEC.md M12, R15
- firmware/src/clk_ext.c clkx_event (0xF2 branch)

## Deliverable
- midi_uart.c parses 0xF2 + two data bytes and enqueues it with source 2 (TRS), like the USB path; running status still cancelled
- A parser test in tests/ (the existing midi_uart_test) for SPP split across reads

## Acceptance criteria
- [ ] SPP via TRS reaches midi_in_q with source 2 and valid 14-bit value
- [ ] Truncated or data-byte-less 0xF2 enqueues nothing
- [ ] `sh tools/check.sh` ends with `CHECK PASS`

## Verification
```sh
sh tools/check.sh
```

## UX gate
n/a, internal plumbing.

## Out of scope
- Wiring clkx into the core (P4-012)

## Log
<!-- filed by the P4-009 review -->

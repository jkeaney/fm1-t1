#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Regenerates perf_fixtures.json (run from the repo root; needs build/gen). Compiles only a host probe; it does not touch the firmware build.
set -e
cc -std=gnu11 -O1 -w -Ibuild/gen -Ifirmware/src -o /tmp/ui_sq_probe docs/design/ui_review/mock/fixtures/probe.c -lm
/tmp/ui_sq_probe > docs/design/ui_review/mock/fixtures/perf_fixtures.json

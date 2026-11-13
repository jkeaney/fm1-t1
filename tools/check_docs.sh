#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Fails if a term listed in docs/design/forbidden_terms.txt is still in the live design docs.
cd "$(dirname "$0")/.." || exit 1
bad=0
grep -v -e '^#' -e '^$' docs/design/forbidden_terms.txt | while IFS= read -r t; do
    hits=$(grep -rIlF -e "$t" docs/design/*.md docs/design/*.tsv docs/design/scenarios docs/design/features 2>/dev/null)
    [ -z "$hits" ] || { echo "stale wording '$t' in: $hits"; echo x > build/check/.docbad; }
done
[ ! -f build/check/.docbad ] || { rm -f build/check/.docbad; exit 1; }
exit 0

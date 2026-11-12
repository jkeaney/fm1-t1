# SPDX-License-Identifier: GPL-3.0-only (evidence script; repo root, .venv/bin/python): md_template.md + data/*.txt -> EVIDENCE_B_v3_2.md
# placeholders: @@file@@ whole file; @@file#a-b@@ lines a..b (1-based); @@file~~start~~end~~n@@ from the n-th occurrence of start up to (excluding) the first end after it
import os, re
D = "docs/design/ui_review/critique_v3_2"
t = open(os.path.join(D, "scripts", "md_template.md")).read()
def sub(m):
    spec = m.group(1)
    if "#" in spec and "~~" not in spec:
        f, r = spec.split("#"); a, b = [int(x) for x in r.split("-")]
        return "".join(open(os.path.join(D, "data", f)).read().splitlines(True)[a - 1:b]).rstrip("\n")
    parts = spec.split("~~")
    f = parts[0]; txt = open(os.path.join(D, "data", f)).read()
    if len(parts) == 1: return txt.rstrip("\n")
    start, end, n = parts[1], parts[2].replace("\\n", "\n"), int(parts[3]) if len(parts) > 3 else 1
    i = -1
    for _ in range(n): i = txt.index(start, i + 1)
    j = txt.index(end, i + len(start))
    return txt[i:j].rstrip("\n")
out = re.sub(r"@@(.+?)@@", sub, t)
out = "\n".join(l for l in out.split("\n") if not l.startswith("<!-- toast") and not l.startswith("<!-- state"))
open(os.path.join(D, "EVIDENCE_B_v3_2.md"), "w").write(out)
print("written", len(out), "bytes;", out.count("\n"), "lines; unresolved placeholders:", out.count("@@"))

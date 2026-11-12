"""s0_build.py: runs every measurement script and concatenates their output into ../EVIDENCE_B_v7.md"""
import subprocess, sys, os
H = os.path.dirname(os.path.abspath(__file__))
for s in ("s1_repro_inventory.py", "s1b_marks.py", "s2_contrast.py", "s3_geometry.py", "s3b_bars.py", "s5_sweep.py", "s5b_labels.py", "s6_accuracy_coverage.py", "s7_coverage.py", "s8_cost_grey.py"):
    subprocess.run([sys.executable, os.path.join(H, s)], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
parts = ["summary.md", "s1_out.md", "s1b_out.md", "s2_out.md", "s3_out.md", "s3b_out.md", "s5_out.md", "s5b_out.md", "s6a_out.md", "s7_out.md", "s8_out.md"]
txt = "\n\n".join(open(os.path.join(H, p)).read() for p in parts)
open(os.path.join(H, "..", "EVIDENCE_B_v7.md"), "w").write(txt + "\n")
print(len(txt))

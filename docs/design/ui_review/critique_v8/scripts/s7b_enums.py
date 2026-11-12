import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import *
print("ENUM / FRACTION CHECK over all registered TSV algorithms (names lists, value count, meter fraction at max)")
for a in A.ORDER[:6]:
    for r in range(2):
        for i, k in enumerate(A.ALGOS[a]["rows"][r]):
            n = k["max"] - k["min"] + 1
            if k["names"]:
                flag = "" if len(k["names"]) == n and len(set(k["names"])) == len(k["names"]) else "  <-- names/range/duplicates inconsistent"
                print("  %-9s ROW%d K%d %-6s min %d max %d def %d names %s%s" % (a, r+1, i+1, k["label"], k["min"], k["max"], k["def"], k["names"], flag))
            if A.frac(k, k["max"]) != 100: print("  frac(max)!=100:", a, k["label"], A.frac(k, k["max"]))
import csv
rows = [r for r in csv.reader(open(ROOT+"/docs/design/knob_map.tsv"), delimiter="\t", quotechar=None) if r[0]=="PERF" and r[1]=="ARP" and r[2]=="S4"]
print("TSV ARP S4 spec:", rows[0][5])
k = A.ALGOS["ARP"]["rows"][1][3]
print("v8 ARP DIR: values", [A.value_text(k, v) for v in range(k["min"], k["max"]+1)], "frac at CHORD (index 5):", A.frac(k, 5), " TSV has 6 directions (UP DOWN UPDN RAND ASPL CHORD), v8 yields", k["max"]-k["min"]+1)

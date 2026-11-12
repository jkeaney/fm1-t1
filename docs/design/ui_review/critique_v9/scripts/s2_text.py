from common import *
import el
from collections import Counter
ART=[]
_a = el.P.art
def art(self,x,y,a,c):
    ART.append((x,y,len(a.strip("\n").split("\n")),max(map(len,a.strip("\n").split("\n"))),c)); return _a(self,x,y,a,c)
el.P.art = art
print("scene|string|font|fg|bg|contrast|box")
for n in ["playing","dice","hold_edit","midi_off_after","rec_armed","pending_p02"]:
    del ART[:]
    sc, log = render(scene(n)); assert len(log)==len(sc.texts); [t.update(box=sc.texts[i][:4]) for i,t in enumerate(log)]
    print("\n## %s: %d text draws, icon draws %d (art calls, incl. glyphs)" % (n,len(log),len(ART)))
    # note P.text skips fully offscreen strips: dedupe by box
    L=log
    c=Counter(t['font'] for t in L); print("per font:",dict(c),"total",len(L))
    for t in L:
        print("|%s|%s|%s|%s|%.2f|%s%s" % (t['s'],t['font'],hexc(t['fg']),hexc(t['bg']),contrast_ratio(t['fg'],t['bg']),t['box'],' CROSSES120' if t['box'][1]<120<t['box'][3] else ''))

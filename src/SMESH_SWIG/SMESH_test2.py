
from SMESH_test1 import *

# ---- compute box

print "-------------------------- compute box"
ret=gen.Compute(mesh,idbox)
print ret
log=mesh.GetLog(0); # no erase trace
for linelog in log:
    print linelog


sg.updateObjBrowser(1);

# ---- compute edge

##print "-------------------------- compute edge"
##ret=gen.Compute(mesh,idedge)
##print ret
##log=mesh.GetLog(1);
##for a in log:
##    print a

# ---- add hypothesis to face

# ---- compute face

#print "-------------------------- compute face"
#ret=gen.Compute(mesh,idface)
#print ret
#log=mesh.GetLog(1);
#for a in log:
#    print a

##shell=salome.IDToObject(ids)
##submesh=mesh.GetElementsOnShape(shell)
##ret=mesh.AddHypothesis(shell,algoReg)
##print ret
##ret=mesh.AddHypothesis(shell,hypLen1)
##print ret
##ret=gen.Compute(mesh,ids)
##print ret

#  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
#  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
# 
#  This library is free software; you can redistribute it and/or 
#  modify it under the terms of the GNU Lesser General Public 
#  License as published by the Free Software Foundation; either 
#  version 2.1 of the License. 
# 
#  This library is distributed in the hope that it will be useful, 
#  but WITHOUT ANY WARRANTY; without even the implied warranty of 
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
#  Lesser General Public License for more details. 
# 
#  You should have received a copy of the GNU Lesser General Public 
#  License along with this library; if not, write to the Free Software 
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
# 
#  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
#
#
#
#  File   : SMESH_test2.py
#  Module : SMESH

from SMESH_test1 import *

# ---- compute box

print "-------------------------- compute box"
ret = smesh.Compute(mesh,box)
print ret
log = mesh.GetLog(0); # no erase trace
for linelog in log:
    print linelog

salome.sg.updateObjBrowser(1);

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

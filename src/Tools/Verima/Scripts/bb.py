# Get Information About Mesh by GetMeshIn
import salome
salome.salome_init()

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh = smeshBuilder.New(salome.myStudy)

aMesh,aResult = smesh.CreateMeshesFromMED('/tmp/MASSIF.new.med')
print aResult, aMesh

monMesh=aMesh[0]

# Get Information About Group by GetMeshInfo
print "\nInformation about group by GetMeshInfo:"
lGroups=monMesh.GetGroups()
print lGroups
for g in lGroups :
    print g.GetName()
info = smesh.GetMeshInfo(lGroups[0])
keys = info.keys(); keys.sort()
for i in keys:
  print "  %s  :  %d" % ( i, info[i] )
  pass
#

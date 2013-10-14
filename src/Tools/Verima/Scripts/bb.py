# Get Information About Mesh by GetMeshIn
import salome
import smesh
import SMESH, SALOMEDS

salome.salome_init()
theStudy = salome.myStudy
smesh.SetCurrentStudy(theStudy)

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

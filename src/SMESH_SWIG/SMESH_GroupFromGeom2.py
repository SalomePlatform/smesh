#==============================================================================
#  Info.
#  Bug (from script, bug)   : SMESH_GroupFromGeom.py, PAL6945
#  Modified                 : 25/11/2004
#  Author                   : Kovaltchuk Alexey
#  Project                  : PAL/SALOME
#============================================================================== 
from SMESH_test1 import *
import SMESH

# Compute the mesh created in SMESH_test1

smesh.Compute(mesh, box)

# Create geometry groups on plane:
aGeomGroup1 = geompy.CreateGroup(face , geompy.ShapeType["FACE"])
geompy.AddObject(aGeomGroup1, 1)

aGeomGroup2 = geompy.CreateGroup(face , geompy.ShapeType["EDGE"])

geompy.AddObject(aGeomGroup2, 3)
geompy.AddObject(aGeomGroup2, 6)
geompy.AddObject(aGeomGroup2, 8)
geompy.AddObject(aGeomGroup2, 10)

geompy.addToStudy(aGeomGroup1, "Group on Faces")
geompy.addToStudy(aGeomGroup2, "Group on Edges")

aSmeshGroup1 = mesh.CreateGroupFromGEOM(SMESH.FACE, "SMESHGroup1", aGeomGroup1)
aSmeshGroup2 = mesh.CreateGroupFromGEOM(SMESH.EDGE, "SMESHGroup2", aGeomGroup2)

print "Create aGroupOnShell - a group linked to a shell"
aGroupOnShell = mesh.CreateGroupFromGEOM(SMESH.EDGE, "GroupOnShell", shell)
print "aGroupOnShell type =", aGroupOnShell.GetType()
print "aGroupOnShell size =", aGroupOnShell.Size()
print "aGroupOnShell ids :", aGroupOnShell.GetListOfID()

print " "

print "Modify <LocalLength> hypothesis: 100 -> 50"
hypLen1.SetLength(50)
print "Contents of aGroupOnShell changes:"
print "aGroupOnShell size =", aGroupOnShell.Size()
print "aGroupOnShell ids :", aGroupOnShell.GetListOfID()

print " "

print "Re-compute mesh, contents of aGroupOnShell changes again:"
smesh.Compute(mesh, box)
print "aGroupOnShell size =", aGroupOnShell.Size()
print "aGroupOnShell ids :", aGroupOnShell.GetListOfID()

salome.sg.updateObjBrowser(1);

#  Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
#  See http://www.salome-platform.org/
#
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


"""
Check that creating a mesh on a shaperstudy object does not raise orb not found in GetExistingSubObjects.
It is called in SMESH GUI on Create mesh's Apply, in SMESHGUI_MeshOp::createSubMeshOnInternalEdges.
We explicitly call GetExistingSubObjects here to be able to test it in python.
"""

import sys
import salome

salome.standalone()
salome.salome_init()

###
### SHAPER component
###

from salome.shaper import model

model.begin()
partSet = model.moduleDocument()

### Create Part
Part_1 = model.addPart(partSet)
Part_1_doc = Part_1.document()

### Create Box
Box_1 = model.addBox(Part_1_doc, 10, 10, 10)

### Create Group
Group_1 = model.addGroup(Part_1_doc, "Edges", [model.selection("EDGE", "[Box_1_1/Left][Box_1_1/Bottom]")])
Group_1.setName("edge_ox")
Group_1.result().setName("edge_ox")

### Create Group
Group_2 = model.addGroup(Part_1_doc, "Edges", [model.selection("EDGE", "[Box_1_1/Back][Box_1_1/Left]")])
Group_2.setName("edge_oz")
Group_2.result().setName("edge_oz")

model.end()

###
### SHAPERSTUDY component
###

model.publishToShaperStudy()
import SHAPERSTUDY
Box_1_1, edge_ox, edge_oz = SHAPERSTUDY.shape(model.featureStringId(Box_1))

###
### SMESH component
###

import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()
#smesh.SetEnablePublish( False ) # Set to False to avoid publish in study if not needed or in some particular situations:
                                 # multiples meshes built in parallel, complex and numerous mesh edition (performance)

Mesh_1 = smesh.Mesh(Box_1_1)
Mesh_1.Segment().LocalLength(5)
Mesh_1.Triangle()

if not Mesh_1.Compute():
  raise Exception("Error when computing Mesh_1")

edge_ox_1 = Mesh_1.GroupOnGeom(edge_ox,'edge_ox',SMESH.EDGE)
edge_oz_1 = Mesh_1.GroupOnGeom(edge_oz,'edge_oz',SMESH.EDGE)

# check that ObjectToSObject works (called in GetExistingSubObjects)
Box_1_1_sobj = salome.ObjectToSObject(Box_1_1)
if not Box_1_1_sobj:
  raise Exception("No SObject for Box_1_1")

# check that GetExistingSubObjects works (called in SMESHGUI_MeshOp::createSubMeshOnInternalEdges)
shaperBuilder = salome.lcc.FindOrLoadComponent("FactoryServer","SHAPERSTUDY")
sOp = shaperBuilder.GetIShapesOperations()
geomGroups = sOp.GetExistingSubObjects(Box_1_1, True)

assert(len(geomGroups)==2)

assert(Mesh_1.GetMesh().NbTriangles()>16)

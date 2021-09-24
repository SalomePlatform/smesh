#!/usr/bin/env python

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

### Create Sketch
Sketch_1 = model.addSketch(Part_1_doc, model.defaultPlane("XOZ"))

### Create SketchLine
SketchLine_1 = Sketch_1.addLine(100, 0, 0, 0)

### Create SketchProjection
SketchProjection_1 = Sketch_1.addProjection(model.selection("VERTEX", "PartSet/Origin"), False)
SketchPoint_1 = SketchProjection_1.createdFeature()
Sketch_1.setCoincident(SketchLine_1.endPoint(), SketchPoint_1.result())

### Create SketchLine
SketchLine_2 = Sketch_1.addLine(0, 0, 0, 100)

### Create SketchLine
SketchLine_3 = Sketch_1.addLine(0, 100, 100, 100)

### Create SketchLine
SketchLine_4 = Sketch_1.addLine(100, 100, 100, 0)
Sketch_1.setCoincident(SketchLine_4.endPoint(), SketchLine_1.startPoint())
Sketch_1.setCoincident(SketchLine_1.endPoint(), SketchLine_2.startPoint())
Sketch_1.setCoincident(SketchLine_2.endPoint(), SketchLine_3.startPoint())
Sketch_1.setCoincident(SketchLine_3.endPoint(), SketchLine_4.startPoint())
Sketch_1.setHorizontal(SketchLine_1.result())
Sketch_1.setVertical(SketchLine_2.result())
Sketch_1.setHorizontal(SketchLine_3.result())
Sketch_1.setVertical(SketchLine_4.result())
Sketch_1.setEqual(SketchLine_3.result(), SketchLine_4.result())
Sketch_1.setLength(SketchLine_1.result(), 100)

### Create SketchLine
SketchLine_5 = Sketch_1.addLine(0, 50, 100, 50)
Sketch_1.setCoincident(SketchLine_5.startPoint(), SketchLine_2.result())
Sketch_1.setCoincident(SketchLine_5.endPoint(), SketchLine_4.result())
Sketch_1.setHorizontal(SketchLine_5.result())

### Create SketchLine
SketchLine_6 = Sketch_1.addLine(50, 50.00000000000001, 50, 0)
Sketch_1.setCoincident(SketchLine_6.endPoint(), SketchLine_1.result())
Sketch_1.setVertical(SketchLine_6.result())
Sketch_1.setCoincident(SketchLine_6.startPoint(), SketchLine_5.result())
Sketch_1.setMiddlePoint(SketchLine_6.startPoint(), SketchLine_5.result())
Sketch_1.setMiddlePoint(SketchLine_5.startPoint(), SketchLine_2.result())
model.do()

### Create Extrusion
Extrusion_1 = model.addExtrusion(Part_1_doc, [model.selection("COMPOUND", "Sketch_1")], model.selection(), 100, 0, "Faces|Wires")

### Create Group
Group_1 = model.addGroup(Part_1_doc, "Faces", [model.selection("FACE", "Extrusion_1_1_1/Generated_Face&Sketch_1/SketchLine_5"), model.selection("FACE", "(Extrusion_1_1_3/Generated_Face&Sketch_1/SketchLine_4)(Extrusion_1_1_3/From_Face)(Extrusion_1_1_3/To_Face)(Extrusion_1_1_3/Generated_Face&Sketch_1/SketchLine_3)2(Extrusion_1_1_3/Generated_Face&Sketch_1/SketchLine_2)2")])
Group_1.setName("crack_1")
Group_1.result().setName("crack_1")

### Create Group
Group_2 = model.addGroup(Part_1_doc, "Faces", [model.selection("FACE", "Extrusion_1_1_1/Generated_Face&Sketch_1/SketchLine_6")])
Group_2.setName("crack_2")
Group_2.result().setName("crack_2")

### Create Group
Group_3 = model.addGroup(Part_1_doc, "Faces", [model.selection("COMPSOLID", "Extrusion_1_1")])
Group_3.setName("all_faces")
Group_3.result().setName("all_faces")

### Create GroupSubstraction
GroupSubstraction_1 = model.addGroupSubstraction(Part_1_doc, [model.selection("COMPOUND", "all_faces")], [model.selection("COMPOUND", "crack_1"), model.selection("COMPOUND", "crack_2")])
GroupSubstraction_1.result().setName("sides")

### Create Group
Group_4 = model.addGroup(Part_1_doc, "Edges", [model.selection("EDGE", "[Extrusion_1_1_3/Generated_Face&Sketch_1/SketchLine_3][Extrusion_1_1_3/To_Face]")])
Group_4.setName("top_edge")
Group_4.result().setName("top_edge")

### Create Group
Group_5 = model.addGroup(Part_1_doc, "Solids", [model.selection("SOLID", "Extrusion_1_1_3")])
Group_5.setName("Solid_1")
Group_5.result().setName("Solid_1")

### Create Group
Group_6 = model.addGroup(Part_1_doc, "Solids", [model.selection("SOLID", "Extrusion_1_1_1")])
Group_6.setName("Solid_2")
Group_6.result().setName("Solid_2")

### Create Group
Group_7 = model.addGroup(Part_1_doc, "Solids", [model.selection("SOLID", "Extrusion_1_1_2")])
Group_7.setName("Solid_3")
Group_7.result().setName("Solid_3")

model.end()

###
### SHAPERSTUDY component
###

model.publishToShaperStudy()
import SHAPERSTUDY
shapes = SHAPERSTUDY.shape(model.featureStringId(Extrusion_1))

Extrusion_1_1 = shapes[0]
groups = shapes[1:]

# dict of groups by their name
d_groups = {}
for gr in groups:
  name = gr.GetName()
  d_groups[name] = gr

###
### SMESH component
###

import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()

# Create an hexahedral mesh or polyhedral mesh
# @param algo: "hexahedra", "polyhedra" or "polygons"
def createMesh(algo):
  
  nb_segs = 5
  Mesh_1 = smesh.Mesh(Extrusion_1_1)
  mesh_name = "Mesh_%s"%algo
  Mesh_1.SetName(mesh_name)

  algo_1d = Mesh_1.Segment()
  algo_1d.NumberOfSegments(nb_segs)

  if algo == "hexahedra":
    Mesh_1.Quadrangle(algo=smeshBuilder.QUADRANGLE)
    Mesh_1.Hexahedron(algo=smeshBuilder.Hexa)
  elif algo == "polyhedra":
    Mesh_1.Quadrangle(algo=smeshBuilder.QUADRANGLE)
    Mesh_1.Polyhedron()
  elif algo == "polygons":
    Mesh_1.Polygon()
    Mesh_1.Polyhedron()
  else:
    raise Exception("not expected algo: ", algo)

  d_mesh_groups = {}
  # Create group of faces
  for name, gr in d_groups.items():
    if name.startswith("crack") or name.startswith("Solid_"):
      gr_mesh = Mesh_1.Group(gr)
      d_mesh_groups[name] = gr_mesh

  # Group on nodes
  gr_sides_1 = Mesh_1.GroupOnGeom(d_groups["sides"],'sides',SMESH.NODE)

  # sub-mesh on top edge
  algo_1d_sub = Mesh_1.Segment(geom=d_groups["top_edge"])
  algo_1d_sub.NumberOfSegments(2*nb_segs)
  algo_1d_sub.Propagation()

  isDone = Mesh_1.Compute()

  nb_nodes = Mesh_1.NbNodes()
  # Create 2 cracks by two calls of DoubleNodeElemGroups

  # FIRST CRACK
  # get affected elements on crack_1
  [ affectedVolumes_1, affectedFaces_1, affectedEdges_1 ] = Mesh_1.AffectedElemGroupsInRegion( [ d_mesh_groups["crack_1" ] ], [ gr_sides_1 ], None )
  # affectedVolumes_1 is d_mesh_groups["Solid_1"] => use one or the other
  # double nodes on crack_1
  [ crack_1_double_faces, crack_1_double_nodes ] = Mesh_1.DoubleNodeElemGroups( [ d_mesh_groups["crack_1" ] ], [ gr_sides_1 ], [ affectedVolumes_1, affectedFaces_1, affectedEdges_1 ], 1, 1 )

  # check new nodes were added
  new_nb_nodes_1 = Mesh_1.NbNodes()
  assert new_nb_nodes_1 > nb_nodes
  
  # check number of new nodes
  if algo != "polygons":
    assert new_nb_nodes_1-nb_nodes == (nb_segs*2-1)*(nb_segs-1)
  else:
    assert new_nb_nodes_1-nb_nodes == nb_segs-1

  # check new nodes where affected to volume elements
  affectedVolumes_1_nodes = affectedVolumes_1.GetNodeIDs()
  for n in range(nb_nodes +1, new_nb_nodes_1):
    assert n in affectedVolumes_1_nodes, "New node not affected to affectedVolumes_1 in %s"%mesh_name

  # SECOND CRACK
  # get affected elements on crack_2
  [ affectedVolumes_2, affectedFaces_2, affectedEdges_2 ] = Mesh_1.AffectedElemGroupsInRegion( [ d_mesh_groups["crack_2" ] ], [ gr_sides_1 ], None )
  # double nodes on crack_2
  # affectedVolumes_2 is d_mesh_groups["Solid_3"] => use one or the other
  [ crack_2_double_faces, crack_2_double_nodes ] = Mesh_1.DoubleNodeElemGroups( [ d_mesh_groups["crack_2" ] ], [ gr_sides_1 ], [ affectedVolumes_2, affectedFaces_2, affectedEdges_2 ], 1, 1 )

  # check new nodes were added
  new_nb_nodes_2 = Mesh_1.NbNodes()
  assert new_nb_nodes_2 > new_nb_nodes_1
  
  # check number of new nodes
  if algo != "polygons":
    assert new_nb_nodes_2-new_nb_nodes_1 == (nb_segs-1)*nb_segs
  else:
    assert new_nb_nodes_2-new_nb_nodes_1 == nb_segs-1

  # check new nodes where affected to volume elements
  affectedVolumes_2_nodes = affectedVolumes_2.GetNodeIDs()
  for n in range(new_nb_nodes_1 +1, new_nb_nodes_2):
    assert n in affectedVolumes_2_nodes, "New node not affected to affectedVolumes_2 in %s"%mesh_name

createMesh("hexahedra")
createMesh("polyhedra")
createMesh("polygons")


if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()

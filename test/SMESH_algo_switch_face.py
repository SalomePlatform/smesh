# Tests that switching of algorithms back and forth does not lead to errors

import salome
salome.salome_init()

from salome.geom import geomBuilder

import  SMESH
from salome.smesh import smeshBuilder

# Create a simple face
geompy = geomBuilder.New()

O = geompy.MakeVertex(0, 0, 0)
OX = geompy.MakeVectorDXDYDZ(1, 0, 0)
OY = geompy.MakeVectorDXDYDZ(0, 1, 0)
OZ = geompy.MakeVectorDXDYDZ(0, 0, 1)
Face_1 = geompy.MakeFaceHW(100, 100, 1)
edge = geompy.CreateGroup(Face_1, geompy.ShapeType['EDGE'])
geompy.UnionIDs(edge, [6])
[edge] = geompy.GetExistingSubObjects(Face_1, False)
geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )
geompy.addToStudy( Face_1, 'Face_1' )
geompy.addToStudyInFather( Face_1, edge, 'edge' )

# Create a mesh from the face and a sub-mesh from an edge
smesh = smeshBuilder.New()

Mesh_1 = smesh.Mesh(Face_1,'Mesh_1')
Regular_1D = Mesh_1.Segment()
Number_of_Segments_1 = Regular_1D.NumberOfSegments(3)
Quadrangle_2D = Mesh_1.Quadrangle(algo=smeshBuilder.QUADRANGLE)
edge_1 = Mesh_1.GroupOnGeom(edge,'edge',SMESH.EDGE)
Regular_1D_1 = Mesh_1.Segment(geom=edge)
Number_of_Segments_2 = Regular_1D_1.NumberOfSegments(2)
Propagation_of_1D_Hyp = Regular_1D_1.Propagation()

# Compute initial mesh
Mesh_1.Compute()
Mesh_1.CheckCompute()
Sub_mesh_1 = Regular_1D_1.GetSubMesh()

# Get the number of faces in the mesh
num_faces_before = Mesh_1.NbFaces()
print('Number of faces before switching: %d' % num_faces_before)

# Switch to composite segment algorithm and compute the mesh
status = Mesh_1.RemoveHypothesis(Regular_1D)
CompositeSegment_1D = smesh.CreateHypothesis('CompositeSegment_1D')
Mesh_1.AddHypothesis(CompositeSegment_1D)
Mesh_1.Compute()
Mesh_1.CheckCompute()

# Switch back to regular segment algorithm and compute the mesh
status = Mesh_1.RemoveHypothesis(CompositeSegment_1D)
Mesh_1.AddHypothesis(Regular_1D)
Mesh_1.Compute()
Mesh_1.CheckCompute()

# Get the number of faces in the mesh
num_faces_after = Mesh_1.NbFaces()
print('Number of faces after switching: %d' % num_faces_after)
assert num_faces_before == num_faces_after, 'Number of faces before and after switching should be the same'

## Set names of Mesh objects
smesh.SetName(CompositeSegment_1D, 'CompositeSegment_1D')
smesh.SetName(Number_of_Segments_1, 'Number of Segments_1')
smesh.SetName(Mesh_1.GetMesh(), 'Mesh_1')
smesh.SetName(Number_of_Segments_2, 'Number of Segments_2')
smesh.SetName(edge_1, 'edge')
smesh.SetName(Sub_mesh_1, 'Sub-mesh_1')
smesh.SetName(Regular_1D.GetAlgorithm(), 'Regular_1D')
smesh.SetName(Propagation_of_1D_Hyp, 'Propagation of 1D Hyp. on Opposite Edges_1')
smesh.SetName(Quadrangle_2D.GetAlgorithm(), 'Quadrangle_2D')

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()

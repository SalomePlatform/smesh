# Viewing Mesh Infos

import salome
salome.salome_init_without_session()

import SMESH
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# create a box
box = geom_builder.MakeBox(0., 0., 0., 20., 20., 20.)
geom_builder.addToStudy(box, "box")
[Face_1,Face_2,Face_3,Face_4,Face_5,Face_5] = geom_builder.SubShapeAll(box, geom_builder.ShapeType["FACE"])

# create a mesh
tetra = smesh_builder.Mesh(box, "MeshBox")

algo1D = tetra.Segment()
algo1D.NumberOfSegments(3)

algo2D = tetra.Triangle()
algo2D.MaxElementArea(10.)

algo3D = tetra.Tetrahedron()
algo3D.MaxElementVolume(900.)

# Creation of SubMesh
Regular_1D_1_1 = tetra.Segment(geom=Face_1)
Nb_Segments_1 = Regular_1D_1_1.NumberOfSegments(5)
Nb_Segments_1.SetDistrType( 0 )
Quadrangle_2D = tetra.Quadrangle(geom=Face_1)
if not tetra.Compute(): raise Exception("Error when computing Mesh")
submesh = Regular_1D_1_1.GetSubMesh()

# compute the mesh
if not tetra.Compute(): raise Exception("Error when computing Mesh")

# Creation of group
group = tetra.CreateEmptyGroup( SMESH.FACE, 'Group' )
nbAdd = group.Add( [ 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76 ] )

# Print information about the mesh
print("Information about mesh:") 
print("Number of nodes       : ", tetra.NbNodes())
print("Number of edges       : ", tetra.NbEdges())
print("Number of faces       : ", tetra.NbFaces())
print("          triangles   : ", tetra.NbTriangles())
print("          quadrangles : ", tetra.NbQuadrangles())
print("          polygons    : ", tetra.NbPolygons())
print("Number of volumes     : ", tetra.NbVolumes())
print("          tetrahedrons: ", tetra.NbTetras())
print("          hexahedrons : ", tetra.NbHexas())
print("          prisms      : ", tetra.NbPrisms())
print("          pyramids    : ", tetra.NbPyramids())
print("          polyhedrons : ", tetra.NbPolyhedrons()) 

# Get Information About Mesh by GetMeshInfo
print("\nInformation about mesh by GetMeshInfo:")
info = smesh_builder.GetMeshInfo(tetra)
keys = list(info.keys()); keys.sort()
for i in keys:
  print("  %s   :  %d" % ( i, info[i] ))
  pass

# Get Information About Group by GetMeshInfo
print("\nInformation about group by GetMeshInfo:")
info = smesh_builder.GetMeshInfo(group)
keys = list(info.keys()); keys.sort()
for i in keys:
  print("  %s  :  %d" % ( i, info[i] ))
  pass

# Get Information About SubMesh by GetMeshInfo
print("\nInformation about Submesh by GetMeshInfo:")
info = smesh_builder.GetMeshInfo(submesh)
keys = list(info.keys()); keys.sort()
for i in keys:
  print("  %s  :  %d" % ( i, info[i] ))
  pass

# Change priority of submeshes in Mesh

import salome
import geompy
import smesh
import SMESH

Box_1 = geompy.MakeBoxDXDYDZ(200, 200, 200)
[Face_1,Face_2,Face_3,Face_4,Face_5,Face_6] = geompy.SubShapeAllSorted(Box_1, geompy.ShapeType["FACE"])

# create Mesh object on Box shape
Mesh_1 = smesh.Mesh(Box_1)

# assign mesh algorithms
Regular_1D = Mesh_1.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(20)
Nb_Segments_1.SetDistrType( 0 )
MEFISTO_2D = Mesh_1.Triangle()
Max_Element_Area_1 = MEFISTO_2D.MaxElementArea(1200)
Tetrahedron = Mesh_1.Tetrahedron()
Max_Element_Volume_1 = Tetrahedron.MaxElementVolume(40000)

# create submesh and assign algorithms on Face_1
Regular_1D_1 = Mesh_1.Segment(geom=Face_1)
Nb_Segments_2 = Regular_1D_1.NumberOfSegments(4)
Nb_Segments_2.SetDistrType( 0 )
MEFISTO_2D_1 = Mesh_1.Triangle(algo=smesh.MEFISTO,geom=Face_1)
Length_From_Edges_2D = MEFISTO_2D_1.LengthFromEdges()
SubMesh_1 = MEFISTO_2D_1.GetSubMesh()

# create submesh and assign algorithms on Face_2
Regular_1D_2 = Mesh_1.Segment(geom=Face_2)
Nb_Segments_3 = Regular_1D_2.NumberOfSegments(8)
Nb_Segments_3.SetDistrType( 0 )
MEFISTO_2D_2 = Mesh_1.Triangle(algo=smesh.MEFISTO,geom=Face_2)
Length_From_Edges_2D_1 = MEFISTO_2D_2.LengthFromEdges()
SubMesh_2 = MEFISTO_2D_2.GetSubMesh()

# create submesh and assign algorithms on Face_3
Regular_1D_3 = Mesh_1.Segment(geom=Face_3)
Nb_Segments_4 = Regular_1D_3.NumberOfSegments(12)
Nb_Segments_4.SetDistrType( 0 )
MEFISTO_2D_3 = Mesh_1.Triangle(algo=smesh.MEFISTO,geom=Face_3)
Length_From_Edges_2D_2 = MEFISTO_2D_3.LengthFromEdges()
SubMesh_3 = MEFISTO_2D_3.GetSubMesh()

# check exisiting submesh priority order
[ [ SubMesh_1, SubMesh_3, SubMesh_2 ] ] = Mesh_1.GetMeshOrder()
# set new submesh order
isDone = Mesh_1.SetMeshOrder( [ [ SubMesh_1, SubMesh_2, SubMesh_3 ] ])
# compute mesh
isDone = Mesh_1.Compute()

# clear mesh result and compute with other submesh order
Mesh_1.Clear()
isDone = Mesh_1.SetMeshOrder( [ [ SubMesh_2, SubMesh_1, SubMesh_3 ] ])
isDone = Mesh_1.Compute()

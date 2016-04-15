# Change priority of sub-meshes in Mesh

import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

Box_1 = geompy.MakeBoxDXDYDZ(200, 200, 200)
[Face_1,Face_2,Face_3,Face_4,Face_5,Face_6] = geompy.SubShapeAllSorted(Box_1, geompy.ShapeType["FACE"])

# create Mesh object on Box shape
Mesh_1 = smesh.Mesh(Box_1)

# assign mesh algorithms and hypotheses
Regular_1D = Mesh_1.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(20)
MEFISTO_2D = Mesh_1.Triangle()
Max_Element_Area_1 = MEFISTO_2D.MaxElementArea(1200)
Tetrahedron = Mesh_1.Tetrahedron()
Max_Element_Volume_1 = Tetrahedron.MaxElementVolume(40000)

# create sub-mesh and assign algorithms on Face_1
Regular_1D_1 = Mesh_1.Segment(geom=Face_1)
Nb_Segments_2 = Regular_1D_1.NumberOfSegments(4)
MEFISTO_2D_1 = Mesh_1.Triangle(algo=smeshBuilder.MEFISTO,geom=Face_1)
SubMesh_1 = MEFISTO_2D_1.GetSubMesh()

# create sub-mesh and assign algorithms on Face_2
Regular_1D_2 = Mesh_1.Segment(geom=Face_2)
Nb_Segments_3 = Regular_1D_2.NumberOfSegments(8)
MEFISTO_2D_2 = Mesh_1.Triangle(algo=smeshBuilder.MEFISTO,geom=Face_2)
SubMesh_2 = MEFISTO_2D_2.GetSubMesh()

# create sub-mesh and assign algorithms on Face_3
Regular_1D_3 = Mesh_1.Segment(geom=Face_3)
Nb_Segments_4 = Regular_1D_3.NumberOfSegments(12)
MEFISTO_2D_3 = Mesh_1.Triangle(algo=smeshBuilder.MEFISTO,geom=Face_3)
SubMesh_3 = MEFISTO_2D_3.GetSubMesh()

# check exisiting sub-mesh priority order
[ [ SubMesh_1, SubMesh_3, SubMesh_2 ] ] = Mesh_1.GetMeshOrder()
isDone = Mesh_1.Compute()
print "Nb elements at initial order of sub-meshes:", Mesh_1.NbElements()

# set new sub-mesh order
isDone = Mesh_1.SetMeshOrder( [ [ SubMesh_1, SubMesh_2, SubMesh_3 ] ])
# compute mesh
isDone = Mesh_1.Compute()
print "Nb elements at new order of sub-meshes:", Mesh_1.NbElements()

# compute with other sub-mesh order
isDone = Mesh_1.SetMeshOrder( [ [ SubMesh_2, SubMesh_1, SubMesh_3 ] ])
isDone = Mesh_1.Compute()
print "Nb elements at another order of sub-meshes:", Mesh_1.NbElements()

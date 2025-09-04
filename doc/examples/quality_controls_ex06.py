# Free Faces

from salome.kernel import salome
salome.salome_init_without_session()

from salome.kernel import SMESH
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

####### GEOM part ########

Box_1 = geom_builder.MakeBoxDXDYDZ(200, 200, 200)
Box_1_vertex_6 = geom_builder.GetSubShape(Box_1, [6])
Box_1 = geom_builder.GetMainShape(Box_1_vertex_6)
Box_1_vertex_16 = geom_builder.GetSubShape(Box_1, [16])
Box_1 = geom_builder.GetMainShape(Box_1_vertex_16)
Box_1_vertex_11 = geom_builder.GetSubShape(Box_1, [11])
Box_1 = geom_builder.GetMainShape(Box_1_vertex_11)
Plane_1 = geom_builder.MakePlaneThreePnt(Box_1_vertex_6, Box_1_vertex_16, Box_1_vertex_11, 2000)
Partition_1 = geom_builder.MakePartition([Box_1], [Plane_1], [], [], geom_builder.ShapeType["SOLID"], 0, [], 0)

Box_1_vertex_19 = geom_builder.GetSubShape(Box_1, [19])
Box_1_vertex_21 = geom_builder.GetSubShape(Box_1, [21])
Plane_2 = geom_builder.MakePlaneThreePnt(Box_1_vertex_16, Box_1_vertex_19, Box_1_vertex_21, 2000)

geom_builder.addToStudy( Box_1, "Box_1" )
geom_builder.addToStudyInFather( Box_1, Box_1_vertex_6, "Box_1:vertex_6" )
geom_builder.addToStudyInFather( Box_1, Box_1_vertex_16, "Box_1:vertex_16" )
geom_builder.addToStudyInFather( Box_1, Box_1_vertex_11, "Box_1:vertex_11" )
geom_builder.addToStudy( Plane_1, "Plane_1" )
geom_builder.addToStudy( Partition_1, "Partition_1" )
geom_builder.addToStudyInFather( Box_1, Box_1_vertex_19, "Box_1:vertex_19" )
geom_builder.addToStudyInFather( Box_1, Box_1_vertex_21, "Box_1:vertex_21" )
geom_builder.addToStudy( Plane_2, "Plane_2" )

###### SMESH part ######

Mesh_1 = smesh_builder.Mesh(Partition_1)
Mesh_1.Segment().MaxSize(34.641)
Mesh_1.Triangle()
Mesh_1.Tetrahedron()
if not Mesh_1.Compute(): raise Exception("Error when computing Mesh")

# create a group of free faces
aFilter = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_FreeFaces )
aFaceIds = Mesh_1.GetIdsFromFilter(aFilter)

aGroup = Mesh_1.CreateEmptyGroup(SMESH.FACE, "Free_faces")
aGroup.Add(aFaceIds)

# print the result
print("Criterion: Free faces Nb = ", len(aFaceIds))
j = 1
for i in range(len(aFaceIds)):
  if j > 20: j = 1; print("")
  print(aFaceIds[i], end=' ')
  j = j + 1
  pass
print("")

#filter faces from plane 2
aFilter = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_BelongToPlane, Plane_2)
aFaceIds = Mesh_1.GetIdsFromFilter(aFilter)
aGroup.Remove(aFaceIds)

# create a group of shared faces (located on partition boundary inside box)
aFilter = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_BelongToPlane, Plane_1)
aFaceIds = Mesh_1.GetIdsFromFilter(aFilter)

aGroup = Mesh_1.CreateEmptyGroup(SMESH.FACE, "Shared_faces")
aGroup.Add(aFaceIds)

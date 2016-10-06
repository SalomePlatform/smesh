# Free Faces


import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

####### GEOM part ########

Box_1 = geompy.MakeBoxDXDYDZ(200, 200, 200)
Box_1_vertex_6 = geompy.GetSubShape(Box_1, [6])
Box_1 = geompy.GetMainShape(Box_1_vertex_6)
Box_1_vertex_16 = geompy.GetSubShape(Box_1, [16])
Box_1 = geompy.GetMainShape(Box_1_vertex_16)
Box_1_vertex_11 = geompy.GetSubShape(Box_1, [11])
Box_1 = geompy.GetMainShape(Box_1_vertex_11)
Plane_1 = geompy.MakePlaneThreePnt(Box_1_vertex_6, Box_1_vertex_16, Box_1_vertex_11, 2000)
Partition_1 = geompy.MakePartition([Box_1], [Plane_1], [], [], geompy.ShapeType["SOLID"], 0, [], 0)

Box_1_vertex_19 = geompy.GetSubShape(Box_1, [19])
Box_1_vertex_21 = geompy.GetSubShape(Box_1, [21])
Plane_2 = geompy.MakePlaneThreePnt(Box_1_vertex_16, Box_1_vertex_19, Box_1_vertex_21, 2000)

geompy.addToStudy( Box_1, "Box_1" )
geompy.addToStudyInFather( Box_1, Box_1_vertex_6, "Box_1:vertex_6" )
geompy.addToStudyInFather( Box_1, Box_1_vertex_16, "Box_1:vertex_16" )
geompy.addToStudyInFather( Box_1, Box_1_vertex_11, "Box_1:vertex_11" )
geompy.addToStudy( Plane_1, "Plane_1" )
geompy.addToStudy( Partition_1, "Partition_1" )
geompy.addToStudyInFather( Box_1, Box_1_vertex_19, "Box_1:vertex_19" )
geompy.addToStudyInFather( Box_1, Box_1_vertex_21, "Box_1:vertex_21" )
geompy.addToStudy( Plane_2, "Plane_2" )

###### SMESH part ######

Mesh_1 = smesh.Mesh(Partition_1)
Regular_1D = Mesh_1.Segment()
Max_Size_1 = Regular_1D.MaxSize(34.641)
MEFISTO_2D = Mesh_1.Triangle()
Tetrahedronn = Mesh_1.Tetrahedron()
isDone = Mesh_1.Compute()

# create a group of free faces
aFilter = smesh.GetFilter(SMESH.FACE, SMESH.FT_FreeFaces )
aFaceIds = Mesh_1.GetIdsFromFilter(aFilter)

aGroup = Mesh_1.CreateEmptyGroup(SMESH.FACE, "Free_faces")
aGroup.Add(aFaceIds)

# print the result
print "Criterion: Free faces Nb = ", len(aFaceIds)
j = 1
for i in range(len(aFaceIds)):
  if j > 20: j = 1; print ""
  print aFaceIds[i],
  j = j + 1
  pass
print ""

#filter faces from plane 2
aFilter = smesh.GetFilter(SMESH.FACE, SMESH.FT_BelongToPlane, Plane_2)
aFaceIds = Mesh_1.GetIdsFromFilter(aFilter)
aGroup.Remove(aFaceIds)

# create a group of shared faces (located on partition boundary inside box)
aFilter = smesh.GetFilter(SMESH.FACE, SMESH.FT_BelongToPlane, Plane_1)
aFaceIds = Mesh_1.GetIdsFromFilter(aFilter)

aGroup = Mesh_1.CreateEmptyGroup(SMESH.FACE, "Shared_faces")
aGroup.Add(aFaceIds)

salome.sg.updateObjBrowser(True)

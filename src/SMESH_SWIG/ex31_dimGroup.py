import sys
import salome
import geompy
import math
import SALOMEDS
import SMESH
import smesh

salome.salome_init()
aStudyId = salome.myStudy._get_StudyId()

geompy.init_geom(salome.myStudy)

geompy.init_geom(salome.myStudy)
global Box_1
Box_1 = geompy.MakeBoxDXDYDZ(200, 200, 200)
geompy.addToStudy( Box_1, "Box_1" )

#smesh.smesh.SetCurrentStudy(theStudy)
import StdMeshers
Mesh_1 = smesh.Mesh(Box_1)
Regular_1D = Mesh_1.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(10)
Nb_Segments_1.SetDistrType( 0 )
Quadrangle_2D = Mesh_1.Quadrangle()
Hexa_3D = Mesh_1.Hexahedron()
isDone = Mesh_1.Compute()

### CreateDimGroup()

aListOf3d_1=range(721,821)

aGrp3D_1=Mesh_1.GetMesh().CreateGroup( smesh.VOLUME, "Src 3D 1" )
aGrp3D_1.Add( aListOf3d_1 )

aListOf3d_2=range(821, 921)
aGrp3D_2=Mesh_1.GetMesh().CreateGroup( smesh.VOLUME, "Src 3D 2" )
aGrp3D_2.Add( aListOf3d_2 )

aGrp2D = Mesh_1.CreateDimGroup( [aGrp3D_1, aGrp3D_2], smesh.FACE, "Faces" )

aGrp1D = Mesh_1.CreateDimGroup( [aGrp3D_1, aGrp3D_2], smesh.EDGE, "Edges" )

aGrp0D = Mesh_1.CreateDimGroup( [aGrp3D_1, aGrp3D_2], smesh.NODE, "Nodes" )

salome.sg.updateObjBrowser( 1 )


# Pattern Mapping


import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# define the geometry
Box_1 = geompy.MakeBoxDXDYDZ(200., 200., 200.)
geompy.addToStudy(Box_1, "Box_1")

faces = geompy.SubShapeAll(Box_1, geompy.ShapeType["FACE"])
Face_1 = faces[0]
Face_2 = faces[1]

geompy.addToStudyInFather(Box_1, Face_1, "Face_1")
geompy.addToStudyInFather(Box_1, Face_2, "Face_2")

# build a quadrangle mesh 3x3 on Face_1
Mesh_1 = smesh.Mesh(Face_1)
algo1D = Mesh_1.Segment()
algo1D.NumberOfSegments(3)
Mesh_1.Quadrangle()

isDone = Mesh_1.Compute()
if not isDone: print 'Mesh Mesh_1 : computation failed'

# build a triangle mesh on Face_2
Mesh_2 = smesh.Mesh(Face_2)

algo1D = Mesh_2.Segment()
algo1D.NumberOfSegments(1)
algo2D = Mesh_2.Triangle()
algo2D.MaxElementArea(240)

isDone = Mesh_2.Compute()
if not isDone: print 'Mesh Mesh_2 : computation failed'

# create a 2d pattern
pattern = smesh.GetPattern()

isDone = pattern.LoadFromFace(Mesh_2.GetMesh(), Face_2, 0)
if (isDone != 1): print 'LoadFromFace :', pattern.GetErrorCode()

# apply the pattern to a face of the first mesh
facesToSplit = Mesh_1.GetElementsByType(SMESH.FACE)
print "Splitting %d rectangular face(s) to %d triangles..."%(len(facesToSplit), 2*len(facesToSplit))
pattern.ApplyToMeshFaces(Mesh_1.GetMesh(), facesToSplit, 0, 0)
isDone = pattern.MakeMesh(Mesh_1.GetMesh(), 0, 0)
if (isDone != 1): print 'MakeMesh :', pattern.GetErrorCode()  

# create quadrangle mesh
Mesh_3 = smesh.Mesh(Box_1)
Mesh_3.Segment().NumberOfSegments(1)
Mesh_3.Quadrangle()
Mesh_3.Hexahedron()
isDone = Mesh_3.Compute()
if not isDone: print 'Mesh Mesh_3 : computation failed'

# create a 3d pattern (hexahedrons)
pattern_hexa = smesh.GetPattern()

smp_hexa = """!!! Nb of points:
15
      0        0        0   !- 0
      1        0        0   !- 1
      0        1        0   !- 2
      1        1        0   !- 3
      0        0        1   !- 4
      1        0        1   !- 5
      0        1        1   !- 6
      1        1        1   !- 7
    0.5        0      0.5   !- 8
    0.5        0        1   !- 9
    0.5      0.5      0.5   !- 10
    0.5      0.5        1   !- 11
      1        0      0.5   !- 12
      1      0.5      0.5   !- 13
      1      0.5        1   !- 14
  !!! Indices of points of 4 elements:
  8 12 5 9 10 13 14 11
  0 8 9 4 2 10 11 6
  2 10 11 6 3 13 14 7
  0 1 12 8 2 3 13 10"""

pattern_hexa.LoadFromFile(smp_hexa)

# apply the pattern to a mesh
volsToSplit = Mesh_3.GetElementsByType(SMESH.VOLUME)
print "Splitting %d hexa volume(s) to %d hexas..."%(len(volsToSplit), 4*len(volsToSplit))
pattern_hexa.ApplyToHexahedrons(Mesh_3.GetMesh(), volsToSplit,0,3)
isDone = pattern_hexa.MakeMesh(Mesh_3.GetMesh(), True, True)
if (isDone != 1): print 'MakeMesh :', pattern_hexa.GetErrorCode()  

# create one more quadrangle mesh
Mesh_4 = smesh.Mesh(Box_1)
Mesh_4.Segment().NumberOfSegments(1)
Mesh_4.Quadrangle()
Mesh_4.Hexahedron()
isDone = Mesh_4.Compute()
if not isDone: print 'Mesh Mesh_4 : computation failed'

# create another 3d pattern (pyramids)
pattern_pyra = smesh.GetPattern()

smp_pyra = """!!! Nb of points:
9
        0        0        0   !- 0
        1        0        0   !- 1
        0        1        0   !- 2
        1        1        0   !- 3
        0        0        1   !- 4
        1        0        1   !- 5
        0        1        1   !- 6
        1        1        1   !- 7
      0.5      0.5      0.5   !- 8
  !!! Indices of points of 6 elements:
  0 1 5 4 8
  7 5 1 3 8
  3 2 6 7 8
  2 0 4 6 8
  0 2 3 1 8
  4 5 7 6 8"""

pattern_pyra.LoadFromFile(smp_pyra)

# apply the pattern to a face mesh
volsToSplit = Mesh_4.GetElementsByType(SMESH.VOLUME)
print "Splitting %d hexa volume(s) to %d hexas..."%(len(volsToSplit), 6*len(volsToSplit))
pattern_pyra.ApplyToHexahedrons(Mesh_4.GetMesh(), volsToSplit,1,0)
isDone = pattern_pyra.MakeMesh(Mesh_4.GetMesh(), True, True)
if (isDone != 1): print 'MakeMesh :', pattern_pyra.GetErrorCode()  

#
# Triangulation of the skin of the geometry from a Brep representing a plane
# This geometry is from EADS
# Hypothesis and algorithms for the mesh generation are global
#

import os
import salome
import geompy
import StdMeshers

smesh = salome.lcc.FindOrLoadComponent("FactoryServer", "SMESH")

smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId)


# ---------------------------- GEOM --------------------------------------

# import a BRep
#before running this script, please be sure about
#the path the file fileName

filePath = os.environ["DATA_DIR"]
filePath = filePath + "/Shapes/Brep/"

filename = "flight_solid.brep"
filename = filePath + filename

shape = geompy.Import(filename, "BREP")
idShape = geompy.addToStudy(shape, "flight")

print "Analysis of the geometry flight :"
subShellList = geompy.SubShapeAll(shape, geompy.ShapeType["SHELL"])
subFaceList  = geompy.SubShapeAll(shape, geompy.ShapeType["FACE"])
subEdgeList  = geompy.SubShapeAll(shape, geompy.ShapeType["EDGE"])

print "number of Shells in flight : ", len(subShellList)
print "number of Faces  in flight : ", len(subFaceList)
print "number of Edges  in flight : ", len(subEdgeList)


### ---------------------------- SMESH --------------------------------------

print "-------------------------- create Hypothesis"

print "-------------------------- LocalLength"

lengthOfSegments = 0.3

hypLength = smesh.CreateHypothesis("LocalLength", "libStdMeshersEngine.so")
hypLength.SetLength(lengthOfSegments)

print hypLength.GetName()
print hypLength.GetId()
print hypLength.GetLength()

smeshgui.SetName(salome.ObjectToID(hypLength), "LocalLength_0.3")

print "-------------------------- LengthFromEdges"

hypLengthFromEdge = smesh.CreateHypothesis("LengthFromEdges", "libStdMeshersEngine.so")

print hypLengthFromEdge.GetName()
print hypLengthFromEdge.GetId()

smeshgui.SetName(salome.ObjectToID(hypLengthFromEdge), "LengthFromEdge")

print "-------------------------- create Algorithms"

print "-------------------------- Regular_1D"

regular1D = smesh.CreateHypothesis("Regular_1D", "libStdMeshersEngine.so")

smeshgui.SetName(salome.ObjectToID(regular1D), "Wire Discretisation")

print "-------------------------- MEFISTO_2D"

mefisto2D = smesh.CreateHypothesis("MEFISTO_2D", "libStdMeshersEngine.so")

smeshgui.SetName(salome.ObjectToID(mefisto2D), "MEFISTO_2D")

# ---- init a Mesh with the shell
shape_mesh = salome.IDToObject( idShape )

mesh = smesh.CreateMesh(shape_mesh)
smeshgui.SetName(salome.ObjectToID(mesh), "MeshFlight")

# ---- add hypothesis to flight

print "-------------------------- add hypothesis to flight"

mesh.AddHypothesis(shape_mesh,regular1D)
mesh.AddHypothesis(shape_mesh,hypLength)
mesh.AddHypothesis(shape_mesh,mefisto2D)
mesh.AddHypothesis(shape_mesh,hypLengthFromEdge)

salome.sg.updateObjBrowser(1)


print "-------------------------- compute the skin flight"
ret = smesh.Compute(mesh,shape_mesh)
print ret
if ret != 0:
    log = mesh.GetLog(0) # no erase trace
    for linelog in log:
        print linelog
    print "Information about the Mesh_mechanic_tetra:"
    print "Number of nodes      : ", mesh.NbNodes()
    print "Number of edges      : ", mesh.NbEdges()
    print "Number of faces      : ", mesh.NbFaces()
    print "Number of triangles  : ", mesh.NbTriangles()
    print "Number of volumes    : ", mesh.NbVolumes()
else:
    print "probleme when computing the mesh"

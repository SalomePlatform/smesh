#
# Hexahedrization of the geometry generated by the Python script
# SMESH_fixation.py
# Hypothesis and algorithms for the mesh generation are global
#

import SMESH_fixation

import StdMeshers

compshell = SMESH_fixation.compshell
idcomp = SMESH_fixation.idcomp
geompy = SMESH_fixation.geompy
salome = SMESH_fixation.salome

print "Analysis of the geometry to be meshed :"
subShellList = geompy.SubShapeAll(compshell, geompy.ShapeType["SHELL"])
subFaceList  = geompy.SubShapeAll(compshell, geompy.ShapeType["FACE"])
subEdgeList  = geompy.SubShapeAll(compshell, geompy.ShapeType["EDGE"])

print "number of Shells in compshell : ", len(subShellList)
print "number of Faces  in compshell : ", len(subFaceList)
print "number of Edges  in compshell : ", len(subEdgeList)

status = geompy.CheckShape(compshell)
print " check status ", status

### ---------------------------- SMESH --------------------------------------
smesh = salome.lcc.FindOrLoadComponent("FactoryServer", "SMESH")

smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId)

print "-------------------------- create Hypothesis"

print "-------------------------- NumberOfSegments"

numberOfSegments = 5

hypNbSeg = smesh.CreateHypothesis("NumberOfSegments", "libStdMeshersEngine.so")
hypNbSeg.SetNumberOfSegments(numberOfSegments)

print hypNbSeg.GetName()
print hypNbSeg.GetId()
print hypNbSeg.GetNumberOfSegments()

smeshgui.SetName(salome.ObjectToID(hypNbSeg), "NumberOfSegments_5")

print "-------------------------- create Algorithms"

print "-------------------------- Regular_1D"

regular1D = smesh.CreateHypothesis("Regular_1D", "libStdMeshersEngine.so")

smeshgui.SetName(salome.ObjectToID(regular1D), "Wire Discretisation")

print "-------------------------- Quadrangle_2D"

quad2D = smesh.CreateHypothesis("Quadrangle_2D", "libStdMeshersEngine.so")

smeshgui.SetName(salome.ObjectToID(quad2D), "Quadrangle_2D")

print "-------------------------- Hexa_3D"

hexa3D = smesh.CreateHypothesis("Hexa_3D", "libStdMeshersEngine.so")

smeshgui.SetName(salome.ObjectToID(hexa3D), "Hexa_3D")

# ---- init a Mesh with the compshell
shape_mesh = salome.IDToObject( idcomp  )

mesh = smesh.CreateMesh(shape_mesh)
smeshgui.SetName(salome.ObjectToID(mesh), "MeshCompShell")


print "-------------------------- add hypothesis to compshell"

mesh.AddHypothesis(shape_mesh,regular1D)
mesh.AddHypothesis(shape_mesh,hypNbSeg)

mesh.AddHypothesis(shape_mesh,quad2D)
mesh.AddHypothesis(shape_mesh,hexa3D)

salome.sg.updateObjBrowser(1)

print "-------------------------- compute compshell"
ret = smesh.Compute(mesh, shape_mesh)
print ret
if ret != 0:
    log = mesh.GetLog(0) # no erase trace
    for linelog in log:
        print linelog
    print "Information about the MeshcompShel:"
    print "Number of nodes       : ", mesh.NbNodes()
    print "Number of edges       : ", mesh.NbEdges()
    print "Number of faces       : ", mesh.NbFaces()
    print "Number of quadrangles : ", mesh.NbQuadrangles()
    print "Number of volumes     : ", mesh.NbVolumes()
    print "Number of hexahedrons : ", mesh.NbHexas()
else:
    print "problem when Computing the mesh"

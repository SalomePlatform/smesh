# Editing of a mesh

import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

def PrintMeshInfo(theMesh):
    aMesh = theMesh.GetMesh()
    print "Information about mesh:"
    print "Number of nodes       : ", aMesh.NbNodes()
    print "Number of edges       : ", aMesh.NbEdges()
    print "Number of faces       : ", aMesh.NbFaces()
    print "Number of volumes     : ", aMesh.NbVolumes()
    pass

# create a box
box = geompy.MakeBox(0., 0., 0., 20., 20., 20.)
geompy.addToStudy(box, "box")

# select one edge of the box for definition of a local hypothesis
subShapeList = geompy.SubShapeAll(box, geompy.ShapeType["EDGE"])
edge = subShapeList[0]
name = geompy.SubShapeName(edge, box)
geompy.addToStudyInFather(box, edge, name)

# create a mesh
tria = smesh.Mesh(box, "Mesh 2D")
algo1D = tria.Segment()
hyp1 = algo1D.NumberOfSegments(3)
algo2D = tria.Triangle()
hyp2 = algo2D.MaxElementArea(10.)

# create a sub-mesh
algo_local = tria.Segment(edge)
hyp3 = algo_local.Arithmetic1D(1, 6)
hyp4 = algo_local.Propagation()

# compute the mesh
tria.Compute()
PrintMeshInfo(tria)

# remove a local hypothesis
tria.RemoveHypothesis(hyp4, edge)

# compute the mesh
tria.Compute()
PrintMeshInfo(tria)

# change the value of the 2D hypothesis
hyp2.SetMaxElementArea(2.)

# compute the mesh
tria.Compute()
PrintMeshInfo(tria)

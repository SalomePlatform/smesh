# Editing of a mesh

from salome.kernel import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

def PrintMeshInfo(theMesh):
    aMesh = theMesh.GetMesh()
    print("Information about mesh:")
    print("Number of nodes       : ", aMesh.NbNodes())
    print("Number of edges       : ", aMesh.NbEdges())
    print("Number of faces       : ", aMesh.NbFaces())
    print("Number of volumes     : ", aMesh.NbVolumes())
    pass

# create a box
box = geom_builder.MakeBox(0., 0., 0., 20., 20., 20.)
geom_builder.addToStudy(box, "box")

# select one edge of the box for definition of a local hypothesis
subShapeList = geom_builder.SubShapeAll(box, geom_builder.ShapeType["EDGE"])
edge = subShapeList[0]
name = geom_builder.SubShapeName(edge, box)
geom_builder.addToStudyInFather(box, edge, name)

# create a mesh
tria = smesh_builder.Mesh(box, "Mesh 2D")
algo1D = tria.Segment()
hyp1 = algo1D.NumberOfSegments(3)
algo2D = tria.Triangle()
hyp2 = algo2D.MaxElementArea(10.)

# create a sub-mesh
algo_local = tria.Segment(edge)
hyp3 = algo_local.Arithmetic1D(1, 6)
hyp4 = algo_local.Propagation()

# compute the mesh
if not tria.Compute(): raise Exception("Error when computing Mesh")
PrintMeshInfo(tria)

# remove a local hypothesis
tria.RemoveHypothesis(hyp4, edge)

# compute the mesh
if not tria.Compute(): raise Exception("Error when computing Mesh")
PrintMeshInfo(tria)

# change the value of the 2D hypothesis
hyp2.SetMaxElementArea(2.)

# compute the mesh
if not tria.Compute(): raise Exception("Error when computing Mesh")
PrintMeshInfo(tria)

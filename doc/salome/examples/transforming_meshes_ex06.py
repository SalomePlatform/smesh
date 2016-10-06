# Merging Elements


import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# create a face to be meshed
px = geompy.MakeVertex(100., 0.  , 0.  )
py = geompy.MakeVertex(0.  , 100., 0.  )
pz = geompy.MakeVertex(0.  , 0.  , 100.)

vxy = geompy.MakeVector(px, py)
arc = geompy.MakeArc(py, pz, px)

wire = geompy.MakeWire([vxy, arc])
isPlanarFace = 1

face1 = geompy.MakeFace(wire, isPlanarFace)
id_face1 = geompy.addToStudy(face1, "Face1")

# create a circle to be an extrusion path
px1 = geompy.MakeVertex( 100.,  100.,  0.)
py1 = geompy.MakeVertex(-100., -100.,  0.)
pz1 = geompy.MakeVertex(   0.,    0., 50.)

circle = geompy.MakeCircleThreePnt(py1, pz1, px1)
id_circle = geompy.addToStudy(circle, "Path")
 
# create a 2D mesh on the face
trias = smesh.Mesh(face1, "Face : 2D mesh")

algo1D = trias.Segment()
algo1D.NumberOfSegments(6)
algo2D = trias.Triangle()
algo2D.LengthFromEdges()

trias.Compute()

# create a path mesh
circlemesh = smesh.Mesh(circle, "Path mesh")
algo = circlemesh.Segment()
algo.NumberOfSegments(10)
circlemesh.Compute()

# extrusion of the mesh
trias.ExtrusionAlongPath([], circlemesh, circle,
                         1, 0, [], 0, SMESH.PointStruct(0, 0, 0))

# merge nodes
print "Number of nodes before MergeNodes:", 
trias.NbNodes()
tolerance = 0.001
array_of_nodes_groups = trias.FindCoincidentNodes(tolerance)

trias.MergeNodes(array_of_nodes_groups)

print "Number of nodes after MergeNodes:", trias.NbNodes()
print ""
print "Number of elements before MergeEqualElements:"
print "Edges      : ", trias.NbEdges()
print "Triangles  : ", trias.NbTriangles()
print "Quadrangles: ", trias.NbQuadrangles()
print "Volumes    : ", trias.NbVolumes()

# merge elements
trias.MergeEqualElements()
print "Number of elements after MergeEqualElements:"
print "Edges      : ", trias.NbEdges()
print "Triangles  : ", trias.NbTriangles()
print "Quadrangles: ", trias.NbQuadrangles()
print "Volumes    : ", trias.NbVolumes()

salome.sg.updateObjBrowser(True)

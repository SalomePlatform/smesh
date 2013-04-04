# Find Element by Point


import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# Create a geometry to mesh
box = geompy.MakeBoxDXDYDZ(100,100,100)

# Create a mesh
mesh = smesh.Mesh(box,"Mesh")
mesh.AutomaticHexahedralization()
mesh.Compute()

# Create a point
x,y,z = 0, 0, 1

# Find all elements (except 0D ones) located at the point
all_elems_except_0D = mesh.FindElementsByPoint(x,y,z)
assert( len(all_elems_except_0D) == 4)

# Find nodes at the point
nodes = mesh.FindElementsByPoint(x,y,z, SMESH.NODE )
assert( len(nodes) == 0)
assert( len( mesh.FindElementsByPoint(x,y,0, SMESH.NODE)) == 1)

# Find an edge at the point
edges = mesh.FindElementsByPoint(x,y,z, SMESH.EDGE )
assert( len(edges) == 1)

# Find faces at the point
edges = mesh.FindElementsByPoint(x,y,z, SMESH.FACE )
assert( len(edges) == 2)

# Find a volume at the point
vols = mesh.FindElementsByPoint(x,y,z, SMESH.VOLUME )
assert( len(vols) == 1)

# Find 0D elements at the point
elems0d = mesh.FindElementsByPoint(x,y,z, SMESH.ELEM0D )
assert( len(elems0d) == 0)

# Find edges within a group
group1D = mesh.MakeGroupByIds("1D", SMESH.EDGE, [1,2] )
edges = mesh.FindElementsByPoint(x,y,z, SMESH.EDGE, group1D )

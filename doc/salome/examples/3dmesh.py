# 3d mesh generation and mesh exploration

import salome
salome.salome_init()
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

###
# Geometry: an assembly of a box, a cylinder and a truncated cone
# to be meshed with tetrahedra
###

# Define values
name = "ex21_lamp" 
cote = 60 
section = 20 
size = 200 
radius_1 = 80 
radius_2 = 40 
height = 100 

# Build a box
box = geompy.MakeBox(-cote, -cote, -cote, +cote, +cote, +cote) 

# Build a cylinder
pt1 = geompy.MakeVertex(0, 0, cote/3) 
di1 = geompy.MakeVectorDXDYDZ(0, 0, 1) 
cyl = geompy.MakeCylinder(pt1, di1, section, size) 

# Build a truncated cone
pt2 = geompy.MakeVertex(0, 0, size) 
cone = geompy.MakeCone(pt2, di1, radius_1, radius_2, height) 

# Fuse
box_cyl = geompy.MakeFuse(box, cyl) 
piece = geompy.MakeFuse(box_cyl, cone) 

# Add to the study
geompy.addToStudy(piece, name) 

# Create a group of faces
faces_group = geompy.CreateGroup(piece, geompy.ShapeType["FACE"]) 
group_name = name + "_grp" 
geompy.addToStudy(faces_group, group_name) 
faces_group.SetName(group_name) 

# Add faces to the group
faces = geompy.SubShapeAllIDs(piece, geompy.ShapeType["FACE"]) 
geompy.UnionIDs(faces_group, faces) 

###
# Create a mesh
###

# Define a mesh on a geometry
tetra = smesh.Mesh(piece, name) 

# Define 1D algorithm and hypothesis
algo1d = tetra.Segment() 
algo1d.LocalLength(10) 

# Define 2D algorithm and hypothesis
algo2d = tetra.Triangle() 
algo2d.LengthFromEdges() 

# Define 3D algorithm and hypothesis
algo3d = tetra.Tetrahedron()
algo3d.MaxElementVolume(100) 

# Compute the mesh
tetra.Compute() 

# Create a mesh group of all triangles generated on geom faces present in faces_group
group = tetra.Group(faces_group)

###
# Explore the mesh
###

# Retrieve coordinates of nodes
coordStr = ""
for node in tetra.GetNodesId():
    x,y,z = tetra.GetNodeXYZ( node )
    coordStr += "%s (%s, %s, %s) " % ( node, x,y,z )
    pass

# Retrieve nodal connectivity of triangles
triaStr = ""
for tria in tetra.GetElementsByType( SMESH.FACE ):
    nodes = tetra.GetElemNodes( tria )
    triaStr += "%s (%s, %s, %s) " % ( tria, nodes[0], nodes[1], nodes[2] )

# Retrieve group contents
groupStr = ""
for group in tetra.GetGroups():
    ids   = group.GetIDs()
    name  = group.GetName()
    eType = group.GetType()
    groupStr += "'%s' %s: %s \n" % ( name, eType, ids )


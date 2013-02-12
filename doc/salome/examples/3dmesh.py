# 3d mesh generation

from geompy import * 
import smesh 

###
# Geometry: an assembly of a box, a cylinder and a truncated cone
# meshed with tetrahedral 
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
box = MakeBox(-cote, -cote, -cote, +cote, +cote, +cote) 

# Build a cylinder
pt1 = MakeVertex(0, 0, cote/3) 
di1 = MakeVectorDXDYDZ(0, 0, 1) 
cyl = MakeCylinder(pt1, di1, section, size) 

# Build a truncated cone
pt2 = MakeVertex(0, 0, size) 
cone = MakeCone(pt2, di1, radius_1, radius_2, height) 

# Fuse
box_cyl = MakeFuse(box, cyl) 
piece = MakeFuse(box_cyl, cone) 

# Add to the study
addToStudy(piece, name) 

# Create a group of faces
group = CreateGroup(piece, ShapeType["FACE"]) 
group_name = name + "_grp" 
addToStudy(group, group_name) 
group.SetName(group_name) 

# Add faces to the group
faces = SubShapeAllIDs(piece, ShapeType["FACE"]) 
UnionIDs(group, faces) 

###
# Create a mesh
###

# Define a mesh on a geometry
tetra = smesh.Mesh(piece, name) 

# Define 1D hypothesis
algo1d = tetra.Segment() 
algo1d.LocalLength(10) 

# Define 2D hypothesis
algo2d = tetra.Triangle() 
algo2d.LengthFromEdges() 

# Define 3D hypothesis
algo3d = tetra.Tetrahedron()
algo3d.MaxElementVolume(100) 

# Compute the mesh
tetra.Compute() 

# Create a groupe of faces
tetra.Group(group)

# Usage of Body Fitting algorithm

import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# create a sphere
sphere = geom_builder.MakeSphereR( 50 )

# cut the sphere by a box
box = geom_builder.MakeBoxDXDYDZ( 100, 100, 100 )
partition = geom_builder.MakePartition([ sphere ], [ box ], theName="partition")

# create a mesh and assign a "Body Fitting" algo
mesh = smesh_builder.Mesh( partition )
cartAlgo = mesh.BodyFitted()

# define a cartesian grid using Coordinates
coords = list(range(-100,100,10))
cartHyp = cartAlgo.SetGrid( coords,coords,coords, 1000000)

# compute the mesh
if not mesh.Compute(): raise Exception("Error when computing Mesh")
print("nb hexahedra",mesh.NbHexas())
print("nb tetrahedra",mesh.NbTetras())
print("nb polyhedra",mesh.NbPolyhedrons())
print()

# define the grid by setting constant spacing
cartHyp = cartAlgo.SetGrid( "10","10","10", 1000000)

if not mesh.Compute(): raise Exception("Error when computing Mesh")
print("nb hexahedra",mesh.NbHexas())
print("nb tetrahedra",mesh.NbTetras())
print("nb polyhedra",mesh.NbPolyhedrons())
print("nb faces",mesh.NbFaces())
print()

# activate creation of faces
cartHyp.SetToCreateFaces( True )

if not mesh.Compute(): raise Exception("Error when computing Mesh")
print("nb hexahedra",mesh.NbHexas())
print("nb tetrahedra",mesh.NbTetras())
print("nb polyhedra",mesh.NbPolyhedrons())
print("nb faces",mesh.NbFaces())
print()

# enable consideration of shared faces
cartHyp.SetToConsiderInternalFaces( True )
if not mesh.Compute(): raise Exception("Error when computing Mesh")
print("nb hexahedra",mesh.NbHexas())
print("nb tetrahedra",mesh.NbTetras())
print("nb polyhedra",mesh.NbPolyhedrons())
print("nb faces",mesh.NbFaces())
print()

# define the grid by setting different spacing in 2 sub-ranges of geometry
spaceFuns = ["5","10+10*t"]
cartAlgo.SetGrid( [spaceFuns, [0.5]], [spaceFuns, [0.5]], [spaceFuns, [0.25]], 10 )

if not mesh.Compute(): raise Exception("Error when computing Mesh")
print("nb hexahedra",mesh.NbHexas())
print("nb tetrahedra",mesh.NbTetras())
print("nb polyhedra",mesh.NbPolyhedrons())
print()

# Example of customization of dirtections of the grid axes

# make a box with non-orthogonal edges
xDir = geom_builder.MakeVectorDXDYDZ( 1.0, 0.1, 0.0, "xDir" )
yDir = geom_builder.MakeVectorDXDYDZ(-0.1, 1.0, 0.0, "yDir"  )
zDir = geom_builder.MakeVectorDXDYDZ( 0.2, 0.3, 1.0, "zDir"  )
face = geom_builder.MakePrismVecH( xDir, yDir, 1.0 )
box  = geom_builder.MakePrismVecH( face, zDir, 1.0, theName="box" )

spc = "0.1" # spacing

# default axes
mesh = smesh_builder.Mesh( box, "custom axes")
algo = mesh.BodyFitted()
algo.SetGrid( spc, spc, spc, 10000 )
if not mesh.Compute(): raise Exception("Error when computing Mesh")
print("Default axes")
print("   nb hex:",mesh.NbHexas())

# set axes using edges of the box
algo.SetAxesDirs( xDir, [-0.1,1,0], zDir )
if not mesh.Compute(): raise Exception("Error when computing Mesh")
print("Manual axes")
print("   nb hex:",mesh.NbHexas())

# set optimal orthogonal axes
algo.SetOptimalAxesDirs( isOrthogonal=True )
if not mesh.Compute(): raise Exception("Error when computing Mesh")
print("Optimal orthogonal axes")
print("   nb hex:",mesh.NbHexas())

# set optimal non-orthogonal axes
algo.SetOptimalAxesDirs( isOrthogonal=False )
if not mesh.Compute(): raise Exception("Error when computing Mesh")
print("Optimal non-orthogonal axes")
print("   nb hex:",mesh.NbHexas())

import salome
salome.salome_init()

from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

# This script demonstrates generation of 3D mesh basing on a modified 2D mesh
#
# Purpose is to get a tetrahedral mesh in a sphere cut by a cube.
# The requirement is to have a surface mesh on the cube comprised of
# triangles of exactly the same size arranged in a grid pattern.
#
# To fulfill this requirement we mesh the box using Quadrangle: Mapping
# meshing algorithm, split quadrangles into triangles and then generate
# tetrahedrons.


# Make the geometry

Box_1    = geompy.MakeBox(-100,-100,-100, 100, 100, 100)
Sphere_1 = geompy.MakeSphereR( 300 )
Cut_1    = geompy.MakeCut(Sphere_1, Box_1, theName="Cut_1")
# get a spherical face
Sph_Face = geompy.ExtractShapes( Sphere_1, geompy.ShapeType["FACE"] )[0]

# get the shape Sph_Face turned into during MakeCut()
Sph_Face = geompy.GetInPlace(Cut_1, Sph_Face, isNewImplementation=True, theName="Sphere_1")


# 1) Define a mesh with 1D and 2D meshers

import  SMESH
from salome.smesh import smeshBuilder
smesh = smeshBuilder.New(salome.myStudy)

Mesh_1 = smesh.Mesh(Cut_1)

# "global" meshers (assigned to Cut_1) that will be used for the box
Regular_1D = Mesh_1.Segment()
Local_Length_1 = Regular_1D.LocalLength(20)
Quadrangle_2D = Mesh_1.Quadrangle()

# a "local" mesher (assigned to a sub-mesh on Sphere_1) to mesh the sphere
algo_2D = Mesh_1.Triangle( smeshBuilder.NETGEN_1D2D, Sph_Face )
algo_2D.SetMaxSize( 70. )
algo_2D.SetFineness( smeshBuilder.Moderate )
algo_2D.SetMinSize( 7. )

# 2) Compute 2D mesh
isDone = Mesh_1.Compute()

# 3) Split quadrangles into triangles
isDone = Mesh_1.SplitQuadObject( Mesh_1, Diag13=True )

# 4) Define a 3D mesher
Mesh_1.Tetrahedron()

# 5) Compute 3D mesh
Mesh_1.Compute()

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser(True)

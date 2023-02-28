# Deflection 2D

import salome
salome.salome_init_without_session()

import SMESH
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# fuse a box and a sphere
Sphere_1 = geom_builder.MakeSphereR(100)
Box_1 = geom_builder.MakeBoxDXDYDZ(200, 200, 200)
Fuse = geom_builder.MakeFuse( Sphere_1, Box_1, theName="box + sphere" )

# create a mesh
mesh = smesh_builder.Mesh( Fuse, "Deflection_2D")
algo = mesh.Segment()
algo.LocalLength(35)
algo = mesh.Triangle()
if not mesh.Compute(): raise Exception("Error when computing Mesh")

# get min and max deflection
minMax = mesh.GetMinMax( SMESH.FT_Deflection2D )
print("min and max deflection: ", minMax)

# get deflection of a certain face
faceID = mesh.NbEdges() + mesh.NbFaces()
defl = mesh.FunctorValue( SMESH.FT_Deflection2D, faceID )
print("deflection of face %s = %s" % ( faceID, defl ))

margin = minMax[1] / 2

# get all faces with deflection LESS than the margin
aFilter = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_Deflection2D, '<', margin, mesh=mesh)
anIds = aFilter.GetIDs()
print("%s faces have deflection less than %s" %( len(anIds), margin ))

# create a group of faces with deflection MORE than the margin
aGroup = mesh.MakeGroup("Deflection > " + repr(margin), SMESH.FACE, SMESH.FT_Deflection2D,'>',margin)
print("%s faces have deflection more than %s: %s ..." %( aGroup.Size(), margin, aGroup.GetIDs()[:10] ))

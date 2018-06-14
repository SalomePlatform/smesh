# Deflection 2D


import salome
salome.salome_init()
from salome.geom import geomBuilder
geompy = geomBuilder.New()

import SMESH
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New()

# fuse a box and a sphere
Sphere_1 = geompy.MakeSphereR(100)
Box_1 = geompy.MakeBoxDXDYDZ(200, 200, 200)
Fuse = geompy.MakeFuse( Sphere_1, Box_1, theName="box + sphere" )

# create a mesh
mesh = smesh.Mesh( Fuse, "Deflection_2D")
algo = mesh.Segment()
algo.LocalLength(35)
algo = mesh.Triangle()
mesh.Compute()

# get min and max deflection
minMax = mesh.GetMinMax( SMESH.FT_Deflection2D )
print("min and max deflection: ", minMax)

# get deflection of a certain face
faceID = mesh.NbEdges() + mesh.NbFaces()
defl = mesh.FunctorValue( SMESH.FT_Deflection2D, faceID )
print("deflection of face %s = %s" % ( faceID, defl ))

margin = minMax[1] / 2

# get all faces with deflection LESS than the margin
aFilter = smesh.GetFilter(SMESH.FACE, SMESH.FT_Deflection2D, '<', margin, mesh=mesh)
anIds = aFilter.GetIDs()
print("%s faces have deflection less than %s" %( len(anIds), margin ))

# create a group of faces with deflection MORE than the margin
aGroup = mesh.MakeGroup("Deflection > " + repr(margin), SMESH.FACE, SMESH.FT_Deflection2D,'>',margin)
print("%s faces have deflection more than %s: %s ..." %( aGroup.Size(), margin, aGroup.GetIDs()[:10] ))

salome.sg.updateObjBrowser()

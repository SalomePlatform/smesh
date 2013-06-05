# Export of a Mesh

import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# create a box
box = geompy.MakeBox(0., 0., 0., 100., 200., 300.)
idbox = geompy.addToStudy(box, "box")

# create a mesh
tetra = smesh.Mesh(box, "MeshBox")
tetra.Segment().NumberOfSegments(7)
tetra.Triangle()
tetra.Tetrahedron()

# compute the mesh
tetra.Compute()

# export the mesh in a MED file
import tempfile
medFile = tempfile.NamedTemporaryFile(suffix=".med").name
tetra.ExportMED( medFile, 0 )

# export a group in a MED file
face = geompy.SubShapeAll( box, geompy.ShapeType["FACE"])[0] # a box side
group = tetra.GroupOnGeom( face, "face group" ) # group of 2D elements on the <face>
tetra.ExportMED( medFile, meshPart=group )

# ========================
# autoDimension parameter
# ========================

face = geompy.MakeFaceHW( 10, 10, 1, "rectangle" )
mesh2D = smesh.Mesh( face, "mesh2D" )
mesh2D.AutomaticHexahedralization(0)

import MEDLoader, os

# exported mesh is in 2D space because it is a planar mesh lying
# on XOY plane, and autoDimension=True by default
mesh2D.ExportMED( medFile )
medMesh = MEDLoader.MEDLoader.ReadUMeshFromFile(medFile,mesh2D.GetName(),0)
print "autoDimension==True, exported mesh is in %sD"%medMesh.getSpaceDimension()

# exported mesh is in 3D space, same as in Mesh module,
# thanks to autoDimension=False
mesh2D.ExportMED( medFile, autoDimension=False )
medMesh = MEDLoader.MEDLoader.ReadUMeshFromFile(medFile,mesh2D.GetName(),0)
print "autoDimension==False, exported mesh is in %sD"%medMesh.getSpaceDimension()

os.remove( medFile )

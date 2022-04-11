# Export of a Mesh

import os
import tempfile
import MEDLoader

import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# create a box
box = geom_builder.MakeBox(0., 0., 0., 100., 200., 300.)
idbox = geom_builder.addToStudy(box, "box")

# create a mesh
tetra = smesh_builder.Mesh(box, "MeshBox")
tetra.Segment().NumberOfSegments(7)
tetra.Triangle()
tetra.Tetrahedron()

# compute the mesh
tetra.Compute()

# export the mesh in a MED file
medFile = tempfile.NamedTemporaryFile(suffix=".med").name
tetra.ExportMED( medFile, 0 )

# export a group in a MED file
face = geom_builder.SubShapeAll( box, geom_builder.ShapeType["FACE"])[0] # a box side
group = tetra.GroupOnGeom( face, "face group" ) # group of 2D elements on the <face>
tetra.ExportMED( medFile, meshPart=group )

# ========================
# autoDimension parameter
# ========================

face = geom_builder.MakeFaceHW( 10, 10, 1, "rectangle" )
mesh2D = smesh_builder.Mesh( face, "mesh2D" )
mesh2D.AutomaticHexahedralization(0)

# exported mesh is in 2D space because it is a planar mesh lying
# on XOY plane, and autoDimension=True by default
mesh2D.ExportMED( medFile )
medMesh = MEDLoader.ReadUMeshFromFile(medFile,mesh2D.GetName(),0)
print("autoDimension==True, exported mesh is in %sD"%medMesh.getSpaceDimension())

# exported mesh is in 3D space, same as in Mesh module,
# thanks to autoDimension=False
mesh2D.ExportMED( medFile, autoDimension=False )
medMesh = MEDLoader.ReadUMeshFromFile(medFile,mesh2D.GetName(),0)
print("autoDimension==False, exported mesh is in %sD"%medMesh.getSpaceDimension())

os.remove( medFile )

# Export of a Mesh

import geompy
import smesh

# create a box
box = geompy.MakeBox(0., 0., 0., 100., 200., 300.)
idbox = geompy.addToStudy(box, "box")

# create a mesh
tetra = smesh.Mesh(box, "MeshBox")

algo1D = tetra.Segment()
algo1D.NumberOfSegments(7)

algo2D = tetra.Triangle()
algo2D.MaxElementArea(800.)

algo3D = tetra.Tetrahedron()
algo3D.MaxElementVolume(900.)

# compute the mesh
tetra.Compute()

# export the mesh in a MED file
tetra.ExportMED("/tmp/meshMED.med", 0)

# export a group in a MED file
face = geompy.SubShapeAll( box, geompy.ShapeType["FACE"])[0] # a box side
group = tetra.GroupOnGeom( face, "face group" ) # group of 2D elements on the <face>
tetra.ExportMED("/tmp/groupMED.med", meshPart=group)

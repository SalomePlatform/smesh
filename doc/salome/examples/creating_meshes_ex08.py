# Mesh Copying

from smesh import *
SetCurrentStudy(salome.myStudy)

# make geometry of a box
box = geompy.MakeBoxDXDYDZ(100,100,100)
face = geompy.SubShapeAllSorted(box, geompy.ShapeType["FACE"])[0]

# generate 3D mesh
mesh = Mesh(box)
localAlgo = mesh.Triangle(face)
mesh.AutomaticHexahedralization()

# objects to copy
fGroup = mesh.GroupOnGeom( face, "2D on face")
nGroup = mesh.GroupOnGeom( face, "nodes on face", NODE)
subMesh = localAlgo.GetSubMesh()

# make a new mesh by copying different parts of the mesh

# 1. copy the whole mesh
newMesh = CopyMesh( mesh, "whole mesh copy")

# 2. copy a group of 2D elements along with groups
newMesh = CopyMesh( fGroup,  "face group copy with groups",toCopyGroups=True)

# 3. copy a group of nodes with preseving their ids
newMesh = CopyMesh( nGroup, "node group copy", toKeepIDs=True)

# 4. copy some faces
faceIds = fGroup.GetIDs()[-10:]
newMesh = CopyMesh( mesh.GetIDSource( faceIds, FACE ), "some faces copy")

# 5. copy some nodes
nodeIds = nGroup.GetIDs()[-10:]
newMesh = CopyMesh( mesh.GetIDSource( nodeIds, NODE), "some nodes copy")

# 6. copy a sub-mesh
newMesh = CopyMesh( subMesh, "submesh copy" )

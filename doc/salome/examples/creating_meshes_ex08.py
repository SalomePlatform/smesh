# Mesh Copying

import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# make geometry of a box
box = geompy.MakeBoxDXDYDZ(100,100,100)
face = geompy.SubShapeAllSorted(box, geompy.ShapeType["FACE"])[0]

# generate a prismatic 3D mesh
mesh = smesh.Mesh(box)
localAlgo = mesh.Triangle(face)
mesh.AutomaticHexahedralization()

# objects to copy
fGroup = mesh.GroupOnGeom( face, "2D on face")
nGroup = mesh.GroupOnGeom( face, "nodes on face", SMESH.NODE)
subMesh = localAlgo.GetSubMesh()

# make a new mesh by copying different parts of the mesh

# 1. copy the whole mesh
newMesh = smesh.CopyMesh( mesh, "whole mesh copy")

# 2. copy a group of 2D elements along with groups
newMesh = smesh.CopyMesh( fGroup,  "face group copy with groups",toCopyGroups=True)

# 3. copy a group of nodes
newMesh = smesh.CopyMesh( nGroup, "node group copy")

# 4. copy some faces
faceIds = fGroup.GetIDs()[-10:]
newMesh = smesh.CopyMesh( mesh.GetIDSource( faceIds, SMESH.FACE ), "some faces copy")

# 5. copy some nodes
nodeIds = nGroup.GetIDs()[-10:]
newMesh = smesh.CopyMesh( mesh.GetIDSource( nodeIds, SMESH.NODE), "some nodes copy")

# 6. copy a sub-mesh
newMesh = smesh.CopyMesh( subMesh, "sub-mesh copy" )

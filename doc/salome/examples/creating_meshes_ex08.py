# Mesh Copying

import salome
salome.salome_init_without_session()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New()

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New()

# make geometry of a box
box = geompy.MakeBoxDXDYDZ(100,100,100)
face = geompy.SubShapeAllSorted(box, geompy.ShapeType["FACE"])[0]

# generate a prismatic 3D mesh
mesh = smesh.Mesh(box, "box")
localAlgo = mesh.Triangle(face)
mesh.Segment().NumberOfSegments( 3 )
mesh.Quadrangle()
mesh.Prism()
mesh.Compute()

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


# make a new mesh with same hypotheses on a modified geometry

smallBox = geompy.MakeScaleAlongAxes( box, None, 1, 0.5, 0.5 )
cutBox = geompy.MakeCut( box, smallBox, theName="box - smallBox" )

ok, newMesh, groups, submehses, hyps, invIDs = smesh.CopyMeshWithGeom( mesh, cutBox, "cutBox" )
newMesh.Compute()

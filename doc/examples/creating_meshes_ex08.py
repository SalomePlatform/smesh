# Mesh Copying

import salome
salome.salome_init_without_session()

import SMESH
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# make geometry of a box
box = geom_builder.MakeBoxDXDYDZ(100,100,100)
face = geom_builder.SubShapeAllSorted(box, geom_builder.ShapeType["FACE"])[0]

# generate a prismatic 3D mesh
mesh = smesh_builder.Mesh(box, "box")
localAlgo = mesh.Triangle(face)
mesh.Segment().NumberOfSegments( 3 )
mesh.Quadrangle()
mesh.Prism()
if not mesh.Compute(): raise Exception("Error when computing Mesh")

# objects to copy
fGroup = mesh.GroupOnGeom( face, "2D on face")
nGroup = mesh.GroupOnGeom( face, "nodes on face", SMESH.NODE)
subMesh = localAlgo.GetSubMesh()

# make a new mesh by copying different parts of the mesh

# 1. copy the whole mesh
newMesh = smesh_builder.CopyMesh( mesh, "whole mesh copy")

# 2. copy a group of 2D elements along with groups
newMesh = smesh_builder.CopyMesh( fGroup,  "face group copy with groups",toCopyGroups=True)

# 3. copy a group of nodes
newMesh = smesh_builder.CopyMesh( nGroup, "node group copy")

# 4. copy some faces
faceIds = fGroup.GetIDs()[-10:]
newMesh = smesh_builder.CopyMesh( mesh.GetIDSource( faceIds, SMESH.FACE ), "some faces copy")

# 5. copy some nodes
nodeIds = nGroup.GetIDs()[-10:]
newMesh = smesh_builder.CopyMesh( mesh.GetIDSource( nodeIds, SMESH.NODE), "some nodes copy")

# 6. copy a sub-mesh
newMesh = smesh_builder.CopyMesh( subMesh, "sub-mesh copy" )


# make a new mesh with same hypotheses on a modified geometry

smallBox = geom_builder.MakeScaleAlongAxes( box, None, 1, 0.5, 0.5 )
cutBox = geom_builder.MakeCut( box, smallBox, theName="box - smallBox" )

ok, newMesh, groups, submehses, hyps, invIDs = smesh_builder.CopyMeshWithGeom( mesh, cutBox, "cutBox" )
if not newMesh.Compute(): raise Exception("Error when computing Mesh")
